#include "Credentials.h"
#include "Tipos.h"
#include "ListaEncadeada.h"
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <LittleFS.h>

const char LITTLEFS_ERROR[] PROGMEM = "Erro ocorreu ao tentar montar LittleFS";

#define DEBUG
#define SERIAL_PORT                115200
//Rest API
#define HTTP_REST_PORT             80

#define DEFAULT_VOLUME             70
/* ports */
#define D0                         16
#define D1                         5
#define D2                         4
#define D3                         0
#define D4                         2
#define D5                         14
#define D6                         12
#define D7                         13
#define D8                         15
#define D9                         16

#define RelayEyes                  D0
#define RelayHat                   D1
#define RelayBlink                 D2
#define RelayShake                 D3
#define TemperatureHumidity        D4

#define MAX_STRING_LENGTH          2000
#define MAX_PATH                   256
#define MAX_FLOAT                  5

/* 200 OK */
#define HTTP_OK                    200
/* 204 No Content */
#define HTTP_NO_CONTENT            204
/* 400 Bad Request */
#define HTTP_BAD_REQUEST           400
#define HTTP_UNAUTHORIZED          401
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_FOUND             404
#define HTTP_CONFLICT              409

Preferences preferences;
//---------------------------------//
int timeSinceLastRead = 0;
int iCelsius = 0;
int iFahrenheit = 0;
int iHumidity = 0;
int iHeatIndexFahrenheit = 0;
int iHeatIndexCelsius = 0;

/* versão do firmware */
const char version[] PROGMEM = API_VERSION;

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();

// Lista de media no sdcard
ListaEncadeada<Media*> mediaListaEncadeada = ListaEncadeada<Media*>();

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Inicia sensor DHT
DHT dht(TemperatureHumidity, DHT11);

AsyncWebServer server(HTTP_REST_PORT);               // initialise webserver

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
bool WIFI_CONFIG = false;

String getContent(const char* filename) {
  String payload="";  
  bool exists = LittleFS.exists(filename);
  if(exists){
    File file = LittleFS.open(filename, "r"); 
    String mensagem = "Falhou para abrir para leitura";
    if(!file){    
      #ifdef DEBUG
        Serial.println(mensagem);
      #endif
      return mensagem;
    }
    while (file.available()) {
      payload += file.readString();
    }
    file.close();
  } else {
    Serial.println(LITTLEFS_ERROR);
  }  
  return payload;
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpg";
  else if (filename.endsWith(".json")) return "application/json";
  return "text/plain";
}

bool writeContent(String filename, String content){
  File file = LittleFS.open(filename, "w");
  return writeContent(&file, content);
}

bool writeContent(File * file, String content){
  if (!file) {    
    #ifdef DEBUG
      Serial.println(F("Falhou para abrir para escrita"));
    #endif
    return false;
  }
  if (file->print(content)) {    
    #ifdef DEBUG
      Serial.println(F("Arquivo foi escrito"));
    #endif
  } else {    
    #ifdef DEBUG
      Serial.println(F("Falha ao escrever arquivo"));
    #endif
  }
  file->close();
  return true; 
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String getDataHora() {
    // Busca tempo no NTP. Padrao de data: ISO-8601
    time_t nowSecs = time(nullptr);
    struct tm timeinfo;
    char buffer[80];
    while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      nowSecs = time(nullptr);
    }
    gmtime_r(&nowSecs, &timeinfo);
    // ISO 8601: 2021-10-04T14:12:26+00:00
    strftime (buffer,80,"%FT%T%z",&timeinfo);
    return String(buffer);
}

int searchList(String name, String language) {
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    if (name == app->name && language==app->language) {
      return i;
    }
  }
  return -1;
}

String getData(uint8_t *data, size_t len) {
  char raw[len];
  for (size_t i = 0; i < len; i++) {
    //Serial.write(data[i]);
    raw[i] = data[i];
  }
  return String(raw);
}

bool readBodySensorData(byte status, byte gpio) {
  #ifdef DEBUG
    Serial.println(status);
  #endif
  ArduinoSensorPort * arduinoSensorPort = searchListSensor(gpio);
  if(arduinoSensorPort!=NULL) {    
    arduinoSensorPort->status = status;
    return true;
  }
  return false;
}

ArduinoSensorPort * searchListSensor(byte gpio) {
  ArduinoSensorPort *arduinoSensorPort;
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    arduinoSensorPort = sensorListaEncadeada.get(i);
    if (gpio == arduinoSensorPort->gpio) {
      return arduinoSensorPort;
    }
  }
  return NULL;
}

String readSensor(byte gpio){
  String data="";
  ArduinoSensorPort *arduinoSensorPort = searchListSensor(gpio);  
  if(arduinoSensorPort != NULL) {
    arduinoSensorPort->status=digitalRead(gpio);
    data="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
  }
  return data;
}

String readSensorStatus(byte gpio){
  return String(digitalRead(gpio));
}

void addApplication(String name, String language, String description) {
  Application *app = new Application();
  app->name = name;
  app->language = language;
  app->description = description;

  // Adiciona a aplicação na lista
  applicationListaEncadeada.add(app);
}

void addMedia(String name, int size, String lastModified) {
  Media *media = new Media();
  media->name = name;
  media->size = size;
  media->lastModified=lastModified;

  // Adiciona a aplicação na lista
  mediaListaEncadeada.add(media);
}

void saveApplicationList() {
  Application *app;
  String JSONmessage;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}"+',';
  }
  JSONmessage = '['+JSONmessage.substring(0, JSONmessage.length()-1)+']';
  // Grava no storage
  writeContent("/lista.json",JSONmessage); 
  // Grava no adafruit
  mqttClient.publish((String(MQTT_USERNAME)+String("/feeds/list")).c_str(), JSONmessage.c_str());
}

int loadApplicationList() {
  // Carrega do storage
  String JSONmessage = getContent("/lista.json");
  if(JSONmessage == "") {    
    #ifdef DEBUG
      Serial.println(F("Lista local de aplicações vazia"));
    #endif
    return -1;
  } else {
    DynamicJsonDocument doc(MAX_STRING_LENGTH);
    DeserializationError error = deserializeJson(doc, JSONmessage);
    if (error) {
      return 1;
    }
    for(int i = 0; i < doc.size(); i++){
      addApplication(doc[i]["name"], doc[i]["language"], doc[i]["description"]);
    }    
  }
  return 0;
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("SSID ou endereço IP indefinido.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Falhou para configurar");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Conectando ao WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    Serial.println("currentMillis: "+currentMillis);
    Serial.println("previousMillis: "+previousMillis);
    Serial.println("interval: "+interval);    
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Falhou para conectar.");
      return false;
    }
  }
  Serial.println(WiFi.localIP());
  return true;
}

void setup() {
  Serial.begin(SERIAL_PORT);

  // métricas para prometheus
  setupStorage();
  incrementBootCounter();
  //
  
  #ifdef DEBUG
    Serial.println(F("modo debug"));
  #else
    Serial.println(F("modo produção"));
  #endif
  
  // DH11 inicia temperatura
  dht.begin();

  if(!LittleFS.begin()){
    #ifdef DEBUG
      Serial.println(LITTLEFS_ERROR);
    #endif      
  }

  ssid = preferences.getString(PARAM_INPUT_1);
  pass = preferences.getString(PARAM_INPUT_2);
  ip = preferences.getString(PARAM_INPUT_3);
  gateway = preferences.getString(PARAM_INPUT_4);
  
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if(initWiFi()) {    
    #ifdef DEBUG
      Serial.println("\n\nNetwork Configuration:");
      Serial.println("----------------------");
      Serial.print("         SSID: "); Serial.println(WiFi.SSID());
      Serial.print("  Wifi Status: "); Serial.println(WiFi.status());
      Serial.print("Wifi Strength: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
      Serial.print("          MAC: "); Serial.println(WiFi.macAddress());
      Serial.print("           IP: "); Serial.println(WiFi.localIP());
      Serial.print("       Subnet: "); Serial.println(WiFi.subnetMask());
      Serial.print("      Gateway: "); Serial.println(WiFi.gatewayIP());
      Serial.print("        DNS 1: "); Serial.println(WiFi.dnsIP(0));
      Serial.print("        DNS 2: "); Serial.println(WiFi.dnsIP(1));
      Serial.print("        DNS 3: "); Serial.println(WiFi.dnsIP(2));   
    #endif
    startWebServer();
    // exibindo rota /update para atualização de firmware e filesystem
    AsyncElegantOTA.begin(&server, USER_FIRMWARE, PASS_FIRMWARE);
    //setClock();
    /* Usa MDNS para resolver o DNS */
    Serial.println("mDNS configurado e inicializado;");    
    if (!MDNS.begin(HOST)) 
    { 
        //http://minion.local (linux) e http://minion (windows)
        #ifdef DEBUG
          Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
        #endif
        delay(1000);
        ESP.restart();        
    }
    // carrega dados
    loadApplicationList();
    Serial.println("Minion esta funcionando!");
    WIFI_CONFIG = true;      
  }
  else {
    // Conecta a rede Wi-Fi com SSID e senha
    Serial.println("Atribuindo Ponto de Acesso");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Endereço IP do ponto de acesso: ");
    Serial.println(IP);
    startWifiManagerServer();    
  }
}

void loop(void) {
  if(WIFI_CONFIG){
    MDNS.update();
  }
  // Report every 1 minute.
  if(timeSinceLastRead > 60000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    getTemperatureHumidity();
    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}
