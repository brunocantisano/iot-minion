#include "Credentials.h"
#include "Tipos.h"
#include "ListaEncadeada.h"
#include <google-tts.h>
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

#include <SPI.h>
#include <SD.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioOutputSPDIF.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2SNoDAC *out;
AudioFileSourceID3 *id3;
AudioFileSourceICYStream *fileRemote;
AudioFileSourceBuffer *buff;

// Output device is SPDIF
AudioOutputSPDIF *outSPDIF;

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

//Pinos de conexão do ESP8266 e o módulo de cartão SD
#define SD_CS                      11
#define SCK                        6
#define MISO                       7
#define MOSI                       8
//
//Pinos de conexão do ESP8266-I2S e o módulo I2S/DAC CJMCU 1334
#define I2S_DOUT                   D5
#define I2S_LRC                    D6
#define I2S_BCLK                   D7

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
//---------------------------------//
int timeSinceLastRead = 0;

String strCelsius = "0.0";
String strFahrenheit = "0.0";
String strHumidity = "0.0";
String strHeatIndexFahrenheit = "0.0";
String strHeatIndexCelsius = "0.0";

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
PubSubClient client(espClient);

// Inicia sensor DHT
DHT dht(TemperatureHumidity, DHT11);

// Create a webserver object that listens for HTTP request on port 80
AsyncWebServer *server;

/*
// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  (void)cbData;
  Serial.printf("ID3 callback for: %s = '", type);

  if (isUnicode) {
    string += 2;
  }
  
  while (*string) {
    char a = *(string++);
    if (isUnicode) {
      string++;
    }
    Serial.printf("%c", a);
  }
  Serial.printf("'\n");
  Serial.flush();
}
*/
/*
String postUtil(String url, String httpRequestData, String certificate, String key="") { 
  String payload;
  WiFiClientSecure *secureClient = new WiFiClientSecure;
  if(secureClient) {
    char certificadoArray[certificate.length()];
    certificate.toCharArray(certificadoArray, certificate.length());
    secureClient -> setCACert(certificadoArray);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;      
      #ifdef DEBUG
        Serial.print("[HTTPS] begin...\n");
      #endif
      if (https.begin(*client, url)) {  // HTTPS
        if(key.length()!=0){
          https.addHeader("Authorization", "Bearer "+key);
        }
        https.addHeader("Content-Type", "application/json");
        #ifdef DEBUG
          Serial.print("[HTTPS] POST...\n");
        #endif
        // start connection and send HTTP header
        int httpCode = https.POST(httpRequestData);
        // httpCode will be negative on error
        if (httpCode == HTTP_OK) {
          // HTTP header has been send and Server response header has been handled          
          #ifdef DEBUG
            Serial.printf("[HTTPS] POST... code: %d\n", httpCode);
          #endif
          payload = https.getString();
        } else {          
          #ifdef DEBUG
            Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
          #endif
          payload = "{\"message\": \"Método não implementado\"}";
        }
        https.end();
      } else {        
        #ifdef DEBUG
          Serial.printf("[HTTPS] Unable to connect\n");
        #endif
      }
      // End extra scoping block
    }
    delete secureClient;
  }
  return payload;
}

String getUtil(String url, String certificate, String filename="") {
  String payload;
  WiFiClientSecure *secureClient = new WiFiClientSecure;
  if(secureClient) {
    char certificadoArray[certificate.length()];
    certificate.toCharArray(certificadoArray, certificate.length());
    secureClient -> setCACert(certificadoArray);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;      
      #ifdef DEBUG
        Serial.print("[HTTPS] begin...\n");
      #endif      
      if (https.begin(*secureClient, url)) {  // HTTPS        
        #ifdef DEBUG
          Serial.print("[HTTPS] GET...\n");
        #endif
        // start connection and send HTTP header
        int httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled          
          #ifdef DEBUG
            Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
          #endif
          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            payload = https.getString();
            if(filename!=""){
              writeContent(filename, payload);
            }
            #ifdef DEBUG
              Serial.println(payload);
            #endif
          }
        } else {          
          #ifdef DEBUG
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          #endif          
        }
        https.end();
      } else {        
        #ifdef DEBUG
          Serial.printf("[HTTPS] Unable to connect\n");
        #endif
      }
      // End extra scoping block
    }
    delete secureClient;
  } else {
    Serial.println("Unable to create client");
  }
  return payload;
}
*/
// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
  const char *ptr = reinterpret_cast<const char *>(cbData);
  (void) isUnicode; // Punt this ball for now
  // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
  char s1[32], s2[64];
  strncpy_P(s1, type, sizeof(s1));
  s1[sizeof(s1)-1]=0;
  strncpy_P(s2, string, sizeof(s2));
  s2[sizeof(s2)-1]=0;
  Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
  Serial.flush();
}

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
      return String(mensagem);
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

// TODO
void loadI2S() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  //SPI.begin(SCK, MISO, MOSI);
  //SPI.setFrequency(1000000);
  //SD.begin(SD_CS);

  //Ajusta os pinos de conexão I2S
  //audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  //Ajusta o volume de saída.
  //audio.setVolume(DEFAULT_VOLUME); // 0...21
}

// TODO
bool playSpeech(const char * mensagem)
{
  //Para executar uma síntese de voz
  TTS tts;
  String url = tts.getSpeechUrl(mensagem, "pt");
  /*
  if(getUtil(url, getContent("/text2speech.crt"), "/audio.mp3").length() == 0) {
    Serial.println("Não foi possível tocar o arquivo: /audio.mp3");
    return false;
  }
  */
  // toca o áudio
  playMidia("/audio.mp3");
  return true;
}

// TODO
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
  audioLogger = &Serial;
  file = new AudioFileSourceSPIFFS(filenameMidia);
  id3 = new AudioFileSourceID3(file);
  id3->RegisterMetadataCB(MDCallback, (void*)"ID3TAG");
  out = new AudioOutputI2SNoDAC();
  mp3 = new AudioGeneratorMP3();
  mp3->begin(id3, out);  
}

// TODO
void playRemoteMidia(const char * url)
{ 
  audioLogger = &Serial;
  fileRemote = new AudioFileSourceICYStream(url);

  // Commented out for performance issues with high rate MP3 stream
  //file->RegisterMetadataCB(MDCallback, (void*)"ICY");

  buff = new AudioFileSourceBuffer(fileRemote, 4096);  // Doubled form default 2048

  // Commented out for performance issues with high rate MP3 stream
  //buff->RegisterStatusCB(StatusCallback, (void*)"buffer");

  // Set SPDIF output
  outSPDIF = new AudioOutputSPDIF();
  mp3 = new AudioGeneratorMP3();

  // Commented out for performance issues with high rate MP3 stream
  //mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");

  mp3->begin(buff, outSPDIF);
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

    WiFi.mode(WIFI_STA);   
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    Serial.println("Connecting ...");
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);        
        #ifdef DEBUG
          Serial.print(F("."));
        #endif
    }  
    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());              // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer  

    if (MDNS.begin(HOST)) {              // Start the mDNS responder for minion.local      
      Serial.println("mDNS responder started");
    } else {
      Serial.println("Error setting up MDNS responder!");
    }

    if(!LittleFS.begin()){
      Serial.println(LITTLEFS_ERROR);
    }
    
    startWebServer();  

    // exibindo rota /update para atualização de firmware e filesystem
    AsyncElegantOTA.begin(server, USER_FIRMWARE, PASS_FIRMWARE);
    
    Serial.println("HTTP server started");
}

void loop(void) {
  MDNS.update();
  /*
  if (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }
  */
  // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    getTemperatureHumidity();
    timeSinceLastRead = 0;
  }
  delay(100);
  timeSinceLastRead += 100;
}
