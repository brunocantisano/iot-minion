#include "Credentials.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "SPIFFS.h"
#include "ListaEncadeada.h"
#include <Preferences.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>

extern "C" {
#include "crypto/base64.h"
}

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define DEBUG
#define SERIAL_PORT                115200
#define TEMPERATURE_DELAY          60000
//Rest API
#define HTTP_REST_PORT             80

#define RelayHat                   13
#define RelayEyes                  14
#define RelayBlink                 15
#define RelayAudio                 25
#define RelayShake                 26
#define TemperatureHumidity        27

#define MAX_STRING_LENGTH          10000

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

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
//---------------------------------//

/************* lista de aplicacoes jenkins *************/
class Application {
  public:
    String name;
    String language;
    String description;
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

/* versão do firmware */
const char version[] = API_VERSION;

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();

WiFiClient espClient;
PubSubClient client(espClient);

// Inicia sensor DHT
DHT dht(TemperatureHumidity, DHT11);

AsyncWebServer *server;               // initialise webserver

bool fileWriteDecodingBase64(String name, String content){
  //abre o arquivo para escrita.
  //Modos:
  //"r" Abre um arquivo para leitura. O arquivo deve existir.
  //"w" Cria um arquivo vazio para escrita. Se já existir um arquivo com o mesmo nome, seu conteúdo é apagado e o arquivo é considerado um novo arquivo vazio.
  //"a" Anexa a um arquivo. Operações de escrita, anexar dados no final do arquivo. O arquivo é criado se não existir.
  //"r+" Abre um arquivo para atualizar tanto a leitura quanto a escrita. O arquivo deve existir.
  //"w+" Cria um arquivo vazio para leitura e escrita.
  //"a+" Abre um arquivo para leitura e anexação.:

  //escolhendo w porque ambos escreveremos no arquivo e depois leremos no final desta função.
  File file = SPIFFS.open(name.c_str(), "w");

  //verifica o arquivo aberto:
  if (!file) {
    //se o arquivo não abrir, exibiremos uma mensagem de erro;
    String errorMessage = "Can't open '" + name + "' !\r\n";
    Serial.println(errorMessage);
    return false;
  } else{
    //Para todos vocês, programadores C++ da velha escola, isso provavelmente faz todo o sentido, mas para o resto de nós, meros mortais, aqui está o que está acontecendo:
    //O método file.write() tem dois argumentos, buffer e comprimento. Como este exemplo está escrevendo uma string de texto, precisamos converter o
    //string de texto (chamada "content") em um ponteiro uint8_t. Se os ponteiros o confundem, você não está sozinho!
    //Não quero entrar em detalhes sobre ponteiros aqui, vou fazer outro exemplo com cast e ponteiros, por enquanto esta é a sintaxe
    //para escrever uma String em um arquivo de texto.

    size_t outputLength;
 
    unsigned char * decoded = base64_decode((const unsigned char *)content.c_str(), content.length(), &outputLength);
    file.write((uint8_t *)decoded, outputLength);
    file.close();
    
    free(decoded);
    
    return true;
  }
}

String fileRead(String name){
  // lê o arquivo do SPIFFS e armazena-o como uma variável String
  String contents;
  File file = SPIFFS.open(name.c_str(), "r");
  if (!file) {
    String errorMessage = "Não é possível abrir '" + name + "' !\r\n";
    Serial.println(errorMessage);
    return "FILE ERROR";
  }
  else {
    
    // isso vai obter o número de bytes no arquivo e nos dar o valor em um inteiro
    int fileSize = file.size();
    int chunkSize=1024;
    //Este é um array de caracteres para armazenar um pedaço do arquivo.
    // Armazenaremos 1024 caracteres por vez
    char buf[chunkSize];
    int numberOfChunks=(fileSize/chunkSize)+1;
    
    int count=0;
    int remainingChunks=fileSize;
    for (int i=1; i <= numberOfChunks; i++){
      if (remainingChunks-chunkSize < 0){
        chunkSize=remainingChunks;
      }
      file.read((uint8_t *)buf, chunkSize-1);
      remainingChunks=remainingChunks-chunkSize;
      contents+=String(buf);
    }
    file.close();
    return contents;
  }
}

boolean fileRemove(String name){
  // lê o arquivo do SPIFFS e armazena-o como uma variável String
  SPIFFS.remove(name.c_str());
  return true;
}

String getContent(const char* filename) {
  String payload="";
  File file = SPIFFS.open(filename);
  if(!file){    
    #ifdef DEBUG
      Serial.println("Falhou para abrir para leitura");
    #endif
    return "<p>Falhou ao abrir para leitura</p>";
  }
  while (file.available()) {
    payload += file.readString();
  }
  file.close();
  return payload;
}

bool writeContent(String filename, String content){
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {    
    #ifdef DEBUG
      Serial.println("Falhou para abrir para escrita");
    #endif
    return false;
  }
  if (file.print(content)) {    
    #ifdef DEBUG
      Serial.println("Arquivo foi escrito");
    #endif
  } else {    
    #ifdef DEBUG
      Serial.println("Falha ao escrever arquivo");
    #endif
  }
  file.close();
  return true; 
}

String postUtil(String url, String httpRequestData, String certificate, String key="") { 
  String payload;
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    char certificadoArray[certificate.length()];
    certificate.toCharArray(certificadoArray, certificate.length());
    client -> setCACert(certificadoArray);
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
    delete client;
  }
  return payload;
}

String getUtil(String url, String certificate, String filename="") {
  String payload;
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    char certificadoArray[certificate.length()];
    certificate.toCharArray(certificadoArray, certificate.length());
    client -> setCACert(certificadoArray);
    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;      
      #ifdef DEBUG
        Serial.print("[HTTPS] begin...\n");
      #endif      
      if (https.begin(*client, url)) {  // HTTPS        
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
    delete client;
  } else {
    Serial.println("Unable to create client");
  }
  return payload;
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

bool isCertificate(char* filename) {  
  if (match(".*.crt", filename) != 0)
  {
    // matching offsets in ms.capture
    return true;
  }
  return false;
}

bool isAudio(char* filename) {  
  if (match(".audio.wav", filename) != 0)
  {
    // matching offsets in ms.capture
    return true;
  }
  return false;
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println("Listando arquivos armazenados no SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      int tam = strlen(foundfile.name());
      char temp[tam];
      strncpy(temp,foundfile.name(),tam);
      if(isCertificate(temp)) {
        // coloco a linha em negrito que tenha a extensão .crt
        returnText += "<tr align='left'><td><b>" + String(foundfile.name()) + "</b></td><td><b>" + humanReadableSize(foundfile.size()) + "</b></td></tr>"; 
      } else {
        if(isAudio(temp)) { 
          returnText += "<tr align='left'><td><a href='audio'>" + String(foundfile.name()) + "</a></td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
        } else {
          returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
        }        
      }      
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

// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();  
  #ifdef DEBUG
    Serial.println(logmessage);
  #endif
  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
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

int searchList(DynamicJsonDocument doc) {
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    if (doc["name"] == app->name && doc["language"]==app->language) {
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
  File file = SPIFFS.open(filename);
  if(!file){
    #ifdef DEBUG
      Serial.println("Falhou para abrir para leitura");
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

String getJsonText2Speech() {
  return "{\"languageCode\": \"pt-BR\",\"voiceName\": \"pt-PT-Wavenet-A\",\"ssmlGender\": \"FEMALE\",\"audioEncoding\": \"LINEAR16\",\"text2SpeechHost\": \"texttospeech.googleapis.com\",\"text2SpeechHttpsPort\": 443,\"fingerprint\": \"7F 4A A6 9D A6 A8 B5 A6 48 AE C5 5A 03 4C B8 B0 25 32 B8 7F\"}";
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
  ret=addSensor(4, RelayAudio, LOW, "audio");
  if(!ret) return false;
  ret=addSensor(5, RelayShake, LOW, "shake");
  if(!ret) return false;
  ret=addSensor(6, TemperatureHumidity, LOW, "temperature");
  if(!ret) return false;
}

bool readBodySensorData(DynamicJsonDocument doc, byte gpio) {
  byte status = doc["status"];  
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

void addApplication(String name, String language, String description) {
  Application *app = new Application();
  app->name = name;
  app->language = language;
  app->description = description;

  // Adiciona a aplicação na lista
  applicationListaEncadeada.add(app);
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
  // Grava no spiffs
  writeContent("/lista.json", JSONmessage);
  
  // Grava no adafruit
  client.publish((String(MQTT_USERNAME)+String("/feeds/list")).c_str(), JSONmessage.c_str()); 
}

int loadApplicationList() {
  // Carrega do SPIFFS
  String JSONmessage = getContent("/lista.json");
  if(JSONmessage == "") {    
    #ifdef DEBUG
      Serial.println("Lista local de aplicações vazia");
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

bool loadListFromAdafruit() {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  // ler do feed list no adafruit
  String JSONmessage = (String)client.subscribe((String(MQTT_USERNAME)+String("/feeds/list")).c_str());
  if(JSONmessage == "") {    
    #ifdef DEBUG
      Serial.println("Lista de aplicações vazia");
    #endif    
    return false;
  } else {
    DeserializationError error = deserializeJson(doc, JSONmessage);
    if (error) {
      return false;
    }
    for(int i = 0; i < doc.size(); i++){
      addApplication(doc[i]["name"], doc[i]["language"], doc[i]["description"]);
    }
    return true;
  }
}

void taskTemperature( void * parameter)
{
  for( ;; ){    
    #ifdef DEBUG
      Serial.println("Task temperature");
    #endif
    setCelsius();
    setFahrenheit();
    setHumidity();
    delay(60000);    
  }
  #ifdef DEBUG
    Serial.println("Ending task temperature");
  #endif  
  vTaskDelete( NULL );
}

void setup(void)
{
    Serial.begin(SERIAL_PORT);

    // métricas para prometheus
    setupStorage();
    incrementBootCounter();
    //

    #ifdef DEBUG
      Serial.println("modo debug");
    #else
      Serial.println("modo produção");
    #endif

    // carrega sensores
    bool load = loadSensorList();
    if(!load) {
      #ifdef DEBUG
        Serial.println("Nao foi possivel carregar a lista de sensores!");
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
      Serial.println("Versão do firmware: "+String(version));
    #endif

    // conteudo da pasta data
    if(!SPIFFS.begin(true)){      
      #ifdef DEBUG
        Serial.println("Erro aconteceu enquanto montava SPIFFS");
      #endif
    }
  
    /* Conecta-se a rede wi-fi */
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);        
        #ifdef DEBUG
          Serial.print(".");
        #endif
    }
    
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
      Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    #endif

    startWebServer();

    // exibindo rota /update para atualização de firmware e filesystem
    AsyncElegantOTA.begin(server, USER_FIRMWARE, PASS_FIRMWARE);

    setClock();

    /* Usa MDNS para resolver o DNS */
    //Serial.println("mDNS configurado e inicializado;");    
    if (!MDNS.begin(HOST)) 
    { 
        //http://minion.local        
        #ifdef DEBUG
          Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
        #endif
        delay(1000);
        ESP.restart();        
    }
    
    //connecting to a mqtt broker
    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
    if (!client.connected()) {
      String client_id = "minion.local-"+String(WiFi.macAddress());
      #ifdef DEBUG
        Serial.printf("O cliente %s conecta ao mqtt broker publico\n", client_id.c_str());
      #endif      
      if (client.connect(client_id.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {        
        #ifdef DEBUG
          Serial.println("Adafruit mqtt broker conectado");
        #endif
        // carrega lista a partir do SPIFFS
        loadApplicationList();
        // já carregou local, salvo local e no adafruit
        saveApplicationList();
      } else {
        #ifdef DEBUG
          Serial.printf("Falhou com o estado %d\n", client.state());
          Serial.println("Nao foi possivel conectar com o broker mqtt. Por favor, verifique as credenciais e instale uma nova versão de firmware.");
        #endif
      }
    }

    /* asigning the firebase api key (required) */
    config.api_key = FIREBASE_API_KEY;

    /* assigning the user sign in credentials */
    auth.user.email = FIREBASE_USER_EMAIL;
    auth.user.password = FIREBASE_USER_PASSWORD;

    /* assigning the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

#if defined(ESP8266)
    //required for large file data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(1024 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    Firebase.begin(&config, &auth);    
    Firebase.reconnectWiFi(true);

    // thread para ler sensor de temperatura e umidade a cada minuto
    xTaskCreate(taskTemperature, "TaskTemperature", 1000, NULL, 1, NULL);
    Serial.println("Minion funcionando!");
}

void callback(char *topic, byte *payload, unsigned int length) {
  byte gpio;
  bool lista = false;
  String message;
  
  #ifdef DEBUG
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
  #endif
          
  for (int i = 0; i < length; i++) {
     message+=(char) payload[i];
  }
  if(strcmp(topic,(String(MQTT_USERNAME)+String("/feeds/eye")).c_str())==0) {
    gpio = RelayEyes;
  }
  if(strcmp(topic,(String(MQTT_USERNAME)+String("/feeds/hat")).c_str())==0) {
    gpio = RelayHat;
  }
  if(strcmp(topic,(String(MQTT_USERNAME)+String("/feeds/blink")).c_str())==0) {
    gpio=RelayBlink;
  }
  if(strcmp(topic,(String(MQTT_USERNAME)+String("/feeds/shake")).c_str())==0) {
    gpio=RelayShake;
  }    
  if(strcmp(topic,(String(MQTT_USERNAME)+String("/feeds/list")).c_str())==0) {
    lista = true;
  }
  if(!lista) {
    byte status = message=="ON"?1:0;
    digitalWrite(gpio, status);    
  } else {
    #ifdef DEBUG
      Serial.println(message);
    #endif
  }  
  
  #ifdef DEBUG
    Serial.println("-----------------------");
  #endif
}

void loop()
{
   if(client.connected()) {
     client.loop();
   }
   // evitando o problema de:
   // "Task watchdog got triggered. The following tasks did not feed the watchdog in time"
   // para o downloads de midias do firebase
   TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
   TIMERG0.wdt_feed=1;
   TIMERG0.wdt_wprotect=0;
}

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
