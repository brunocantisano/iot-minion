#include "Credentials.h"
#include "ListaEncadeada.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.h>

#define FILESYSTEM "LittleFS"
#define CONFIG_LITTLEFS_FOR_IDF_3_2
#define CONFIG_LITTLEFS_SPIFFS_COMPAT 1
#include <LITTLEFS.h> 

#define DEBUG
#define SERIAL_PORT                115200
//Rest API
#define HTTP_REST_PORT             80
//Volume
#define DEFAULT_VOLUME             70
#define RelayHat                   13
#define RelayEyes                  14
#define RelayBlink                 15
#define RelayShake                 22
#define TemperatureHumidity        33

//Pinos de conexão do ESP32 e o módulo de cartão SD
#define SD_CS                      5
#define SCK                        18
#define MISO                       19
#define MOSI                       23
//
//Pinos de conexão do ESP32-I2S e o módulo I2S/DAC CJMCU 1334
#define I2S_DOUT                   25
#define I2S_LRC                    26
#define I2S_BCLK                   27

#define MAX_STRING_LENGTH          2000
#define MAX_PATH                   256

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
//Cria o objeto que representará o áudio
Audio audio;
//---------------------------------//

/************* lista de aplicacoes jenkins *************/
class Application {
  public:
    String name;
    String language;
    String description;
};
/******************************************************/
/************** lista de mídias no sdcard *************/
class Media {
  public:
    String name;
    int size;
    String lastModified;
};
/******************************************************/
/******************* lista de sensores ****************/
class ArduinoSensorPort {
  public:
    char* name;
    byte id;
    byte gpio;
    byte status; // 1-TRUE / 0-FALSE
};
/******************************************************/

typedef enum {
  celsius,
  fahrenheit,
  humidity
} temperature_dht;

/* versão do firmware */
const char version[] PROGMEM = API_VERSION;

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();

// Lista de media no sdcard
ListaEncadeada<Media*> mediaListaEncadeada = ListaEncadeada<Media*>();

WiFiClient espClient;
PubSubClient client(espClient);

// Inicia sensor DHT
DHT dht(TemperatureHumidity, DHT11);

AsyncWebServer *server;               // initialise webserver

String getContent(const char* filename) {
  String payload="";
  File file = LITTLEFS.open(filename, "r"); 
  if(!file){    
    #ifdef DEBUG
      Serial.println(F("Falhou para abrir para leitura"));
    #endif
    return F("<p>Falhou ao abrir para leitura</p>");
  }
  while (file.available()) {
    payload += file.readString();
  }
  file.close();
  return payload;
}

bool writeContent(String filename, String content){
   File file = LITTLEFS.open(filename, FILE_WRITE);
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

char* substr(char* arr, int begin, int len)
{
    char* res = new char[len + 1];
    for (int i = 0; i < len; i++)
        res[i] = *(arr + begin + i);
    res[len] = 0;
    return res;
}

String IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

// match - search for regular expression anywhere in text
int match(char* regexp, char* text) {
  if(regexp[0] == '^') {
    return matchHere(++regexp, text);
  }
  do{
    if(matchHere(regexp, text)) {return 1;}
  }while(*text++ != '\0');
    return 0;
}

// matchHere - search for regex at beginning of text
int matchHere(char* regexp, char* text) {
  if(regexp[0] == '\0') {return 1;}
  if(regexp[1] == '*') {
    return matchStar(regexp[0], regexp+2, text);
  }
  if(regexp[0] == '$' && regexp[1] == '\0') {
    return *text == '\0';
  }
  if(*text != '\0' && (regexp[0] == '.' || regexp[0] == *text)) {
    return matchHere(++regexp, ++text);
  }
  return 0;
}

// matchStar - search for c*regexp at beginning of text
int matchStar(int c, char* regexp, char* text) {
  do {
    if(matchHere(regexp, text)) {return 1;}
  }while(*text != '\0' &&(*text++ == c || c=='.'));
  return 0;
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println(F("Listando arquivos armazenados no storage"));
  File root = LITTLEFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      int tam = strlen(foundfile.name());
      char temp[tam];
      strncpy(temp,foundfile.name(),tam);
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
    } else {
      returnText += "Arquivo: " + String(foundfile.name()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

// handles uploads to storage
void handleUploadStorage(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Cliente:" + request->client()->remoteIP().toString() + "-" + request->url() + "-" + filename;  
  #ifdef DEBUG
    Serial.println(logmessage);
  #endif
  if (!index) {
    logmessage = "Upload Iniciado: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LITTLEFS.open("/" + filename, "w");
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Escrevendo arquivo: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
  }

  if (final) {
    logmessage = "Upload Completo: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    request->redirect("/");
  }
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

unsigned char* getStreamData(const char* filename) {
  unsigned char* payload;
  unsigned char ch;
  int i = 0;
  File file = LITTLEFS.open(filename);
  if(!file){
    #ifdef DEBUG
      Serial.println(F("Falhou para abrir para leitura"));
    #endif
    return NULL;
  }
  while (file.available()) {
    ch = file.read();
    payload = (unsigned char*)malloc(sizeof(payload)*sizeof(unsigned char));
    payload[i] = ch;
    i++;
  }
  file.close();
  return payload;
}

bool check_authorization_header(AsyncWebServerRequest * request){
  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    AsyncWebHeader* h = request->getHeader(i);
    //Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    if(h->name()=="Authorization" && h->value()=="Basic "+String(API_MINION_TOKEN)){
      return true;
    }
  }
  return false;
}

bool addSensor(byte id, byte gpio, byte status, char* name) {
  ArduinoSensorPort *arduinoSensorPort = new ArduinoSensorPort(); 
  arduinoSensorPort->id = id;
  arduinoSensorPort->gpio = gpio;
  arduinoSensorPort->status = status;
  arduinoSensorPort->name = name;
  pinMode(gpio, OUTPUT);

  // Adiciona sensor na lista
  sensorListaEncadeada.add(arduinoSensorPort);
  return true;
}

bool loadSensorList()
{
  bool ret = false;
  ret=addSensor(1, RelayEyes, LOW, "eyes");
  if(!ret) return false;
  ret=addSensor(2, RelayHat, LOW, "hat");
  if(!ret) return false;
  ret=addSensor(3, RelayBlink, LOW, "blink");
  if(!ret) return false;
  ret=addSensor(4, RelayShake, LOW, "shake");
  if(!ret) return false;
  ret=addSensor(5, TemperatureHumidity, LOW, "temperature");
  if(!ret) return false;
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
  media->lastModified = lastModified;

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
  client.publish((String(MQTT_USERNAME)+String("/feeds/list")).c_str(), JSONmessage.c_str());
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

void loadI2S() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SCK, MISO, MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  //Ajusta os pinos de conexão I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  //Ajusta o volume de saída.
  audio.setVolume(DEFAULT_VOLUME); // 0...21
}

void playSpeech(const char * mensagem)
{
  //Para executar uma síntese de voz
  // audio.connecttospeech(mensagem, "pt");
  // voice speed: 74%
  // pitch: 52%
  audio.connecttomarytts(mensagem, "it", "istc-lucia-hsmm");
}

void playMidia(const char * midia)
{
  char filenameMidia[strlen(midia)+1];
  filenameMidia[0]='/';
  filenameMidia[1]='\0';
  strcat(filenameMidia, midia);
  #ifdef DEBUG
    Serial.printf("Arquivo a tocar: %s\n",filenameMidia);
  #endif  
  // exemplo: "/1.mp3"
  audio.connecttoSD(filenameMidia);
}

void playRemoteMidia(const char * url)
{ 
  audio.connecttohost(url); //  128k mp3
}

void setup(void)
{
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

    // carrega sensores
    bool load = loadSensorList();
    if(!load) {
      #ifdef DEBUG
        Serial.println(F("Nao foi possivel carregar a lista de sensores!"));
      #endif
    }
          
    // DH11 inicia temperatura
    dht.begin();

    pinMode(RelayEyes, OUTPUT);
    pinMode(RelayHat, OUTPUT);
    pinMode(RelayBlink, OUTPUT);
    pinMode(RelayShake, OUTPUT);
    pinMode(TemperatureHumidity, OUTPUT);

    #ifdef DEBUG
      Serial.println("Versão: "+String(version));
    #endif

    // conteudo da pasta data
    if(!LITTLEFS.begin(true)){      
      #ifdef DEBUG
        Serial.println(F("Erro aconteceu enquanto montava LittleFS"));
      #endif
    }
    /* Conecta-se a rede wi-fi */
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);        
        #ifdef DEBUG
          Serial.print(F("."));
        #endif
    }    
    
    #ifdef DEBUG
      Serial.println(F("\n\nNetwork Configuration:"));
      Serial.println(F("----------------------"));
      Serial.print(F("         SSID: ")); Serial.println(WiFi.SSID());
      Serial.print(F("  Wifi Status: ")); Serial.println(WiFi.status());
      Serial.print(F("Wifi Strength: ")); Serial.print(WiFi.RSSI()); Serial.println(F(" dBm"));
      Serial.print(F("          MAC: ")); Serial.println(WiFi.macAddress());
      Serial.print(F("           IP: ")); Serial.println(WiFi.localIP());
      Serial.print(F("       Subnet: ")); Serial.println(WiFi.subnetMask());
      Serial.print(F("      Gateway: ")); Serial.println(WiFi.gatewayIP());
      Serial.print(F("        DNS 1: ")); Serial.println(WiFi.dnsIP(0));
      Serial.print(F("        DNS 2: ")); Serial.println(WiFi.dnsIP(1));
      Serial.print(F("        DNS 3: ")); Serial.println(WiFi.dnsIP(2));   
    #endif

    startWebServer();
        
    // exibindo rota /update para atualização de firmware e filesystem
    AsyncElegantOTA.begin(server, USER_FIRMWARE, PASS_FIRMWARE);

    setClock();

    /* Usa MDNS para resolver o DNS */
    //Serial.println(F("mDNS configurado e inicializado;"));    
    if (!MDNS.begin(HOST)) 
    { 
        //http://HOST.local        
        #ifdef DEBUG
          Serial.println(F("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s..."));
        #endif
        delay(1000);
        ESP.restart();        
    }
    // carrega lista de arquivos de media no SDCARD
    if(loadSdCardMedias()) loadI2S(); //Configura e inicia o SPI para conexão com o cartão SD
        
    //connecting to a mqtt broker
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
    Serial.println(F("Minion funcionando!"));
}

void callback(char *topic, byte *payload, unsigned int length) {
  byte gpio;
  String message;
  
  #ifdef DEBUG
    Serial.print(F("Mensagem que chegou no tópico: "));
    Serial.println(topic);
    Serial.print(F("Mensagem:"));
  #endif
          
  for (int i = 0; i < length; i++) {
     message+=(char) payload[i];
  }
  #ifdef DEBUG
    Serial.println(message);
  #endif
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/eye")).c_str()) {
    digitalWrite(RelayEyes, message=="ON"?1:0);
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/hat")).c_str()) {
    digitalWrite(RelayHat, message=="ON"?1:0);
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/blink")).c_str()) {
    digitalWrite(RelayBlink, message=="ON"?1:0);
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/shake")).c_str()) {
    digitalWrite(RelayShake, message=="ON"?1:0);
  }    
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/list")).c_str()) {
    DynamicJsonDocument doc(MAX_STRING_LENGTH);
    // ler do feed list no adafruit
    if(message == "") {    
      #ifdef DEBUG
        Serial.println(F("Lista de aplicações vazia"));
      #endif
      // carrega lista a partir do storage      
      if(loadApplicationList()>=0) saveApplicationList();
    } else {
      DeserializationError error = deserializeJson(doc, message);
      if (error) {
        #ifdef DEBUG
          Serial.println(F("Erro ao fazer o parser da lista vindo do Adafruit"));
        #endif
      }
      for(int i = 0; i < doc.size(); i++){
        addApplication(doc[i]["name"], doc[i]["language"], doc[i]["description"]);
      }
    }
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/play")).c_str()) {
    playMidia(message.c_str());
    
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/talk")).c_str()) {
    playSpeech(message.c_str());
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/volume")).c_str()) {
    setVolumeAudio(atoi(message.c_str()));
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/temperature")).c_str()) {
    getTemperatureHumidity(celsius);
  }  
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/humidity")).c_str()) {
    getTemperatureHumidity(humidity);
  }  
  #ifdef DEBUG
    Serial.println(F("-----------------------"));
  #endif
}

void reconnect() {
  // Loop até que esteja reconectado
  while (!client.connected()) {
    Serial.println("Tentando conexão com o servidor MQTT...");
    String client_id = String(HOST)+".local-"+String(WiFi.macAddress());
    #ifdef DEBUG
      Serial.printf("O cliente %s conecta ao mqtt broker publico\n", client_id.c_str());
    #endif      
    if (client.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println(F("Adafruit mqtt broker conectado"));
      // Subscribe
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/eye")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/hat")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/blink")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/shake")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/list")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/play")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/talk")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/volume")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/temperature")).c_str());
      client.subscribe((String(MQTT_USERNAME)+String("/feeds/humidity")).c_str());
      //
    } else {
      #ifdef DEBUG
        Serial.printf("Falhou com o estado %d\nNao foi possivel conectar com o broker mqtt.\nPor favor, verifique as credenciais e instale uma nova versão de firmware.\nTentando novamente em 5 segundos.", client.state());
      #endif
      delay(5000);
    }
  }
}

void loop()
{ 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //Executa o loop interno da biblioteca audio
  audio.loop();
}

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
