#include "Credentials.h"
#include "Tipos.h"
#include "ListaEncadeada.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.h>
#include <HTTPClient.h>

#define FILESYSTEM "LittleFS"
#define CONFIG_LITTLEFS_FOR_IDF_3_2
#define CONFIG_LITTLEFS_SPIFFS_COMPAT 1
#include <LittleFS.h>

const char LITTLEFS_ERROR[] PROGMEM = "Erro ocorreu ao tentar montar LittleFS";

//#define DEBUG
#define SERIAL_PORT                  115200
//Rest API
#define HTTP_REST_PORT               80
//Volume
#define DEFAULT_VOLUME               20
#define RelayHat                     13
#define RelayEyes                    14
#define RelayBlink                   15
#define RelayShake                   22
#define TemperatureHumidity          33

//Pinos de conexão do ESP32 e o módulo de cartão SD
#define SD_CS                        5
#define SCK                          18
#define MISO                         19
#define MOSI                         23
//
//Pinos de conexão do ESP32-I2S e o módulo I2S/DAC CJMCU 1334
#define I2S_DOUT                     25
#define I2S_LRC                      26
#define I2S_BCLK                     27

#define MAX_STRING_LENGTH            2000
#define MAX_PATH                     256

/* 200 OK */
#define HTTP_OK                      200
/* 204 No Content */
#define HTTP_NO_CONTENT              204
/* 400 Bad Request */
#define HTTP_BAD_REQUEST             400
#define HTTP_UNAUTHORIZED            401
#define HTTP_INTERNAL_SERVER_ERROR   500
#define HTTP_NOT_FOUND               404
#define HTTP_CONFLICT                409

Preferences preferences;
//Cria o objeto que representará o áudio
Audio audio;
//---------------------------------//
int timeSinceLastRead = 0;
String strCelsius;
String strFahrenheit;
String strHumidity;
String strHeatIndexFahrenheit;
String strHeatIndexCelsius;

/* versão do firmware */
const char version[] PROGMEM = API_VERSION;

enum AudioType {
  GOOGLE, // 0
  MARYTTS, // 1
  OPENAI,
};

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();

// Lista de media no sdcard
ListaEncadeada<Media*> mediaListaEncadeada = ListaEncadeada<Media*>();

WiFiClient espClient;
PubSubClient client(espClient);
// variavel para checar se já conectou na rede
bool rede = false;
// Inicia sensor DHT
DHT dht(TemperatureHumidity, DHT11);

AsyncWebServer server(HTTP_REST_PORT);               // initialise webserver

IPAddress localIP;
// Set your Gateway IP address
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
const char *chatGPTUrl = "https://api.openai.com/v1/chat/completions";

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

bool getAllowedStorageFiles(String filename) {  
  if (filename.endsWith(".crt")) return true;
  return false;
}

bool getAllowedSdCardFiles(String filename) {  
  if (filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".gif")) return true;
  else if (filename.endsWith(".wav") || filename.endsWith(".mp3")) return true;
  return false;
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

bool writeContentFile(String filename, String content){
  File file = LittleFS.open(filename, "w");
  return writeContent(&file, content);
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
    return (String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]));
}

void setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
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

// matchStar - search for c*regexp at beginning of text
int matchStar(int c, char* regexp, char* text) {
  do {
    if(matchHere(regexp, text)) {return 1;}
  }while(*text != '\0' &&(*text++ == c || c=='.'));
  return 0;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
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
  raw[len]=0x00;
  return String(raw);
}

bool addSensor(int id, int gpio, int status, String name) {
  ArduinoSensorPort *arduinoSensorPort = new ArduinoSensorPort(); 
  pinMode(gpio, OUTPUT);
  
  arduinoSensorPort->id = id;
  arduinoSensorPort->gpio = gpio;
  arduinoSensorPort->status = status;
  arduinoSensorPort->name = name;  
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

bool readBodySensorData(uint8_t status, uint8_t gpio) {
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

ArduinoSensorPort * searchListSensor(uint8_t gpio) {
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

String readSensor(uint8_t gpio){
  String data="";
  ArduinoSensorPort *arduinoSensorPort = searchListSensor(gpio);  
  if(arduinoSensorPort != NULL) {
    arduinoSensorPort->status=digitalRead(gpio);
    data="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
  }
  return data;
}

String readSensorStatus(uint8_t gpio){
  return String(digitalRead(gpio));
}

void addApplication(String name, String language, String description) {
  if(searchList(name, language)== -1) {
    Application *app = new Application();
    app->name = name;
    app->language = language;
    app->description = description;
    // Adiciona a aplicação na lista
    applicationListaEncadeada.add(app);
  }
}

void addMedia(String name, int size, String lastModified) {
  Media *media = new Media();
  media->name = name;
  media->size = size;
  media->lastModified=lastModified;

  // Adiciona a aplicação na lista
  mediaListaEncadeada.add(media);
}

String saveApplicationList() {
  Application *app;
  String JSONmessage;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}"+',';
  }
  JSONmessage = '['+JSONmessage.substring(0, JSONmessage.length()-1)+']';
  // Grava no storage
  writeContentFile("/lista.json",JSONmessage); 
  return JSONmessage;
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
}

bool playSpeech(const char * mensagem, AudioType tipo=AudioType::GOOGLE)
{
  switch(tipo) {
    case AudioType::GOOGLE:
      //Para executar uma síntese de voz
      audio.connecttospeech(mensagem, "pt");
      break;
    case AudioType::MARYTTS:
      // voice speed: 74%
      // pitch: 52%
      //audio.connecttomarytts(mensagem, "it", "istc-lucia-hsmm");
      Serial.println("Foi removido o uso da Marytts no código recente");
      break;   
    case AudioType::OPENAI:
      // Open IA Speech
      audio.openai_speech(OPEN_IA_KEY, "tts-1", mensagem, "fable", "mp3", "3"); //speed vai de 1.0 a 4.0
      break;
    default:
      Serial.println("Não encontrei o tipo de audio a ser executado!");
      return false;
  }
  return true;
}

bool playMidia(const char * midia)
{
  char filenameMidia[strlen(midia)+1];
  filenameMidia[0]='/';
  filenameMidia[1]='\0';
  strcat(filenameMidia, midia);
  #ifdef DEBUG
    Serial.printf("Arquivo a tocar: %s\n",filenameMidia);
  #endif  
  // exemplo: "/1.mp3"
  bool exists = SD.exists(filenameMidia);
  if(exists){
    audio.connecttoFS(SD, filenameMidia);
    return true;
  }
  return false;
}

void playRemoteMidia(const char * url)
{ 
  audio.connecttohost(url); //  128k mp3
}

// Initialize WiFi
bool initWiFi() {
  // Search for parameter in HTTP POST request
  //Variables to save values from HTML form
  String ssid = preferences.getString("ssid");
  String pass = preferences.getString("pass");
  String ip = preferences.getString("ip");
  String gateway = preferences.getString("gateway");

  Serial.println(ssid);
  /*Serial.println(pass);*/
  Serial.println(ip);
  Serial.println(gateway);
  
  if(ssid=="" || ip==""){
    Serial.println("SSID ou endereço IP indefinido.");
    return false;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando ao WiFi...");
  }
  ip = String(IpAddress2String(WiFi.localIP()));
  Serial.println("Conectado ao WiFi");  
  Serial.println(ip);

  preferences.putString("ssid", ssid.c_str());
  preferences.putString("pass", pass.c_str());
  preferences.putString("ip", ip.c_str());
  preferences.putString("gateway", gateway.c_str());
 
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Falhou para conectar.");
      return false;
    }
  }  
  return true;
}

void setup()
{
  Serial.begin(SERIAL_PORT);
  
  preferences.begin("store",false);
   
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

  pinMode(RelayEyes, OUTPUT);
  pinMode(RelayHat, OUTPUT);
  pinMode(RelayBlink, OUTPUT);
  pinMode(RelayShake, OUTPUT);
  pinMode(TemperatureHumidity, OUTPUT);

  #ifdef DEBUG
    Serial.println("Versão: "+String(version));
  #endif

  if(!LittleFS.begin(true)){
    #ifdef DEBUG
      Serial.println(LITTLEFS_ERROR);
    #endif      
  }

  // carrega sensores  
  bool load = loadSensorList();
  if(!load) {
    #ifdef DEBUG
      Serial.println(F("Nao foi possivel carregar a lista de sensores!"));
    #endif
  }

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
    setClock();
    /* Usa MDNS para resolver o DNS */
    Serial.println("mDNS configurado e inicializado;");    
    if (!MDNS.begin(HOST)) 
    { 
        //http://minion.local
        #ifdef DEBUG
          Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
        #endif
        ESP.restart();        
    }
    // carrega lista de arquivos de media no SDCARD
    if(loadSdCardMedias()) loadI2S(); //Configura e inicia o SPI para conexão com o cartão SD

    //Ajusta os pinos de conexão I2S para saída de áudio
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

    //Ajusta o volume de saída.
    audio.setVolume(DEFAULT_VOLUME); // 0...21
  
    rede=true;
    //connecting to a mqtt broker
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);  
    Serial.println(F("Minion funcionando!"));
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
  preferences.end();
}

void callback(char *topic, uint8_t *payload, unsigned int length) {
  uint8_t gpio;
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
  /*
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/play")).c_str()) {
    playMidia(message.c_str()); 
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/talk")).c_str()) {
    playSpeech(message.c_str());
  }
  */
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/volume")).c_str()) {
    setVolumeAudio(atoi(message.c_str()));
  }
  if(String(topic) == (String(MQTT_USERNAME)+String("/feeds/temperature")).c_str() ||
    String(topic) == (String(MQTT_USERNAME)+String("/feeds/humidity")).c_str()) {
    #ifdef DEBUG
      // busca temperatura e umidade
    Serial.println("busca temperatura e umidade");
    #endif
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
    }
  }
}

void loop() 
{  
  unsigned long currentMillis = millis();
  if (!client.connected()) {
    // tento conectar no MQTT somente se já tiver rede
    if(rede) reconnect();
  }
  client.loop();
  //Executa o loop interno da biblioteca audio
  audio.loop(); 

  // Report every 1 minuto.
  if (currentMillis - previousMillis >= 60000) {
    previousMillis = currentMillis;
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    if(rede) getTemperatureHumidity();
  }
}

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
