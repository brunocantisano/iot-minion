// Adafruit.io docs: https://learn.adafruit.com/mqtt-adafruit-io-and-you/intro-to-adafruit-mqtt
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>

#define MQTT                      0

#ifdef MQTT
  #include <Adafruit_MQTT.h>
  #include <Adafruit_MQTT_Client.h>

  #define MQTT_DELAY              5000

  #define AIO_SERVER              "io.adafruit.com"
  #define AIO_SERVERPORT          1883
  #define AIO_USERNAME            "<AIO_USERNAME>"
  #define AIO_KEY                 "<AIO_KEY>"
  #define MQTT_CONN_KEEPALIVE     180
#endif

#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecureBearSSL.h>

#include "ListaEncadeada.h"

#define SERIAL_PORT               115200
#define TEMPERATURE_DELAY         60000

//Rest API
#define HTTP_REST_PORT            80
#define WIFI_RETRY_DELAY          500
#define MAX_WIFI_INIT_RETRY       50
#define MAX_LENGTH                10  
/* ports */
#define D0                        16
#define D1                        5
#define D2                        4
#define D3                        0
#define D4                        2
#define D5                        14
#define D6                        12
#define D7                        13
#define D8                        15

#define RelayEyes                 D5
#define RelayHat                  D7
#define RelayBlink                D8
#define TemperatureHumidity       D6
#define MAX_STRING_LENGTH         200
#define MAX_LIST_STRING_LENGTH    1024
/* 200 OK */
#define HTTP_OK                   200
/* 204 No Content */
#define HTTP_NO_CONTENT           204
/* 400 Bad Request */
#define HTTP_BAD_REQUEST          400
#define HTTP_NOT_FOUND            404
#define HTTP_CONFLICT             409
#define API_TEXT2SPEECH_KEY       "<API_TEXT2SPEECH_KEY>"
/************* lista de aplicacoes jenkins *************/
class Application {
  public:
    char name[MAX_STRING_LENGTH];
    char language[MAX_STRING_LENGTH];
    char description[MAX_STRING_LENGTH];
};
/******************************************************/
/******************* lista de sensores ****************/
struct ArduinoSensorPort {
    byte id;
    byte gpio;
    byte status;
    char name[MAX_STRING_LENGTH];
};
/******************************************************/
const char* wifi_ssid = "<wifi_ssid>";
const char* wifi_passwd = "<wifi_passwd>";

/******************** speech to text *****************/
const char* languageCode="pt-BR";
const char* voiceName="pt-BR-Wavenet-A";
const char* ssmlGender="FEMALE";
const char* audioEncoding="MP3";
const char *text2SpeechHost = "texttospeech.googleapis.com";
const int text2SpeechHttpsPort = 443;  //HTTPS= 443 and HTTP = 80

//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "7F 4A A6 9D A6 A8 B5 A6 48 AE C5 5A 03 4C B8 B0 25 32 B8 7F";
/******************************************************/

char* swaggerJSON = "{\"swagger\": \"2.0\", \"info\":{\"description\": \"Minion Soap Esp8266 using REST api server. You can find out more about Minion Server at [minion-server](https://github.com/brunocantisano/iot-minion)\", \"version\": \"1.0.0\", \"title\": \"Swagger Minion\", \"termsOfService\": \"http://swagger.io/terms/\", \"contact\":{\"email\": \"bruno.cantisano@gmail.com\"}, \"license\":{\"name\": \"Apache 2.0\", \"url\": \"http://www.apache.org/licenses/LICENSE-2.0.html\"}}, \"host\": \"192.168.0.10\", \"basePath\": \"/\", \"tags\": [{\"name\": \"comando\", \"description\": \"Comandos\", \"externalDocs\":{\"description\": \"Descubra mais sobre a garagem digital\", \"url\": \"https://www.garagemdigital.io\"}}], \"schemes\": [ \"https\", \"http\"], \"paths\":{\"/health\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Checagem Health\", \"description\": \"Faz checagem da saúde da aplicação\", \"operationId\": \"health\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"default\":{\"description\": \"successful operation\"}}}}, \"/ports\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca as portas utilizadas\", \"description\": \"\", \"operationId\": \"getPorta\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/talk\":{\"post\":{\"tags\": [ \"comando\"], \"summary\": \"Fala através do Google Home\", \"description\": \"\", \"operationId\": \"postTalk\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Mensagem a ser passada para o Google Home\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Mensagem\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/eye\":{\"put\":{\"tags\": [ \"comando\"], \"summary\": \"Acende ou apaga os olhos\", \"description\": \"\", \"operationId\": \"updateEye\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Alterado o comando\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Comando\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/eyes\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca o estado dos olhos\", \"description\": \"\", \"operationId\": \"getEye\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/laugh\":{\"post\":{\"tags\": [ \"comando\"], \"summary\": \"Toca um audio pelo nome\", \"description\": \"\", \"operationId\": \"postLaugh\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Nome do audio\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Midia\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/hat\":{\"put\":{\"tags\": [ \"comando\"], \"summary\": \"Gira o chapéu ou para\", \"description\": \"\", \"operationId\": \"updateHat\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Alterado o comando\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Comando\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/hats\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca o estado do chapéu\", \"description\": \"\", \"operationId\": \"getHat\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/blink\":{\"put\":{\"tags\": [ \"comando\"], \"summary\": \"Pisca ou para de piscar o corpo\", \"description\": \"\", \"operationId\": \"updateBlink\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Alterado o comando\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Comando\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/blinks\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca o estado do corpo\", \"description\": \"\", \"operationId\": \"getBlink\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/list\":{\"post\":{\"tags\": [ \"comando\"], \"summary\": \"Insere item na lista\", \"description\": \"\", \"operationId\": \"insertList\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Inserido na lista\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Lista\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/lists\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca o estado da lista\", \"description\": \"\", \"operationId\": \"getLista\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/list/del\":{\"delete\":{\"tags\": [ \"comando\"], \"summary\": \"Apaga item da lista\", \"description\": \"\", \"operationId\": \"deleteItemFromList\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [{\"in\": \"body\", \"name\": \"body\", \"description\": \"Apagado item da lista\", \"required\": true, \"schema\":{\"$ref\": \"#/definitions/Lista\"}}], \"responses\":{\"400\":{\"description\": \"Comando inválido fornecido\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}, \"/temperature\":{\"get\":{\"tags\": [ \"comando\"], \"summary\": \"Busca a temperatura e umidade\", \"description\": \"\", \"operationId\": \"temperature\", \"produces\": [ \"application/xml\", \"application/json\"], \"parameters\": [], \"responses\":{\"204\":{\"description\": \"Não foi possível encontrar as informações\"}, \"200\":{\"description\": \"Comando executado com sucesso\"}}}}}, \"definitions\":{\"Comando\":{\"type\": \"object\", \"properties\":{\"status\":{\"type\": \"integer\", \"format\": \"int64\", \"description\": \"1-inicia, 0-para\"}}, \"xml\":{\"name\": \"Comando\"}}, \"Lista\":{\"type\": \"object\", \"properties\":{\"name\":{\"type\": \"string\", \"description\": \"nome da aplicação\"}, \"language\":{\"type\": \"string\", \"description\": \"nome da linguagem de programação\"}, \"description\":{\"type\": \"string\", \"description\": \"descrição de execução\"}}, \"xml\":{\"name\": \"Lista\"}}, \"Mensagem\":{\"type\": \"object\", \"properties\":{\"mensagem\":{\"type\": \"string\", \"description\": \"mensagem a ser passada para o Google Home\"}}, \"xml\":{\"name\": \"Mensagem\"}}, \"Midia\":{\"type\": \"object\", \"properties\":{\"midia\":{\"type\": \"integer\", \"format\": \"int64\", \"description\": \"Nome do audio a ser tocado\", \"enum\": [ 1, 2]}}, \"xml\":{\"name\": \"Midia\"}}}, \"externalDocs\":{\"description\": \"Descubra mais sobre a garagem digital\", \"url\": \"https://www.garagemdigital.io\"}}";
char* swaggerUI = "<!DOCTYPE html><html lang=\"en\" xml:lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"x-ua-compatible\" content=\"IE=edge\"> <title>Swagger UI</title> <link href='https://cdnjs.cloudflare.com/ajax/libs/meyer-reset/2.0/reset.min.css' media='screen' rel='stylesheet' type='text/css'/> <link href='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/css/screen.css' media='screen' rel='stylesheet' type='text/css'/> <script>if (typeof Object.assign !='function'){(function (){Object.assign=function (target){'use strict'; if (target===undefined || target===null){throw new TypeError('Cannot convert undefined or null to object');}var output=Object(target); for (var index=1; index < arguments.length; index++){var source=arguments[index]; if (source !==undefined && source !==null){for (var nextKey in source){if (Object.prototype.hasOwnProperty.call(source, nextKey)){output[nextKey]=source[nextKey];}}}}return output;};})();}</script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery/1.8.0/jquery-1.8.0.min.js' type='text/javascript'></script> <script>(function(b){b.fn.slideto=function(a){a=b.extend({slide_duration:\"slow\",highlight_duration:3E3,highlight:true,highlight_color:\"#FFFF99\"},a);return this.each(function(){obj=b(this);b(\"body\").animate({scrollTop:obj.offset().top},a.slide_duration,function(){a.highlight&&b.ui.version&&obj.effect(\"highlight\",{color:a.highlight_color},a.highlight_duration)})})}})(jQuery); </script> <script>jQuery.fn.wiggle=function(o){var d={speed:50,wiggles:3,travel:5,callback:null};var o=jQuery.extend(d,o);return this.each(function(){var cache=this;var wrap=jQuery(this).wrap('<div class=\"wiggle-wrap\"></div>').css(\"position\",\"relative\");var calls=0;for(i=1;i<=o.wiggles;i++){jQuery(this).animate({left:\"-=\"+o.travel},o.speed).animate({left:\"+=\"+o.travel*2},o.speed*2).animate({left:\"-=\"+o.travel},o.speed,function(){calls++;if(jQuery(cache).parent().hasClass('wiggle-wrap')){jQuery(cache).parent().replaceWith(cache);}if(calls==o.wiggles&&jQuery.isFunction(o.callback)){o.callback();}});}});}; </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery.ba-bbq/1.2.1/jquery.ba-bbq.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/handlebars.js/4.0.5/handlebars.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/lodash-compat/3.10.1/lodash.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/backbone.js/1.1.2/backbone-min.js' type='text/javascript'></script> <script>Backbone.View=(function(View){return View.extend({constructor: function(options){this.options=options ||{}; View.apply(this, arguments);}});})(Backbone.View); </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/swagger-ui.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/highlight.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/languages/json.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/json-editor/0.7.28/jsoneditor.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/marked/0.3.6/marked.min.js' type='text/javascript'></script> <script type=\"text/javascript\">$(function (){url=\"http://192.168.0.10/swagger.json\"; hljs.configure({highlightSizeThreshold: 5000}); window.swaggerUi=new SwaggerUi({url: url, dom_id: \"swagger-ui-container\", supportedSubmitMethods: ['get', 'post', 'put', 'delete', 'patch'],validatorUrl: null, onComplete: function(swaggerApi, swaggerUi){}, onFailure: function(data){log(\"Unable to Load SwaggerUI\");}, docExpansion: \"none\", jsonEditor: false, defaultModelRendering: 'schema', showRequestHeaders: false, showOperationIds: false}); window.swaggerUi.load(); function log(){if ('console' in window){console.log.apply(console, arguments);}}}); </script></head><body class=\"swagger-section\"><div id='header'> <div class=\"swagger-ui-wrap\"> <a id=\"logo\" href=\"http://swagger.io\"><img class=\"logo__img\" alt=\"swagger\" height=\"30\" width=\"30\" src=\"https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/images/logo_small.png\"/><span class=\"logo__title\">swagger</span></a> <form id='api_selector'> </form> </div></div><div id=\"message-bar\" class=\"swagger-ui-wrap\" data-sw-translate>&nbsp;</div><div id=\"swagger-ui-container\" class=\"swagger-ui-wrap\"></div></body></html>";
ESP8266WebServer http_rest_server(HTTP_REST_PORT);

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

#ifdef MQTT
  // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
  Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
  
  // Setup feeds called 'eye', 'hat', 'blink' and 'temperature' for subscribing to changes.
  Adafruit_MQTT_Subscribe eyesReadFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/eye"); // set adafruit FeedName
  Adafruit_MQTT_Subscribe hatReadFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/hat"); // set adafruit FeedName
  Adafruit_MQTT_Subscribe listReadFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/list"); // set adafruit FeedName
  Adafruit_MQTT_Subscribe blinkReadFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/blink"); // set adafruit FeedName
  // Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
  Adafruit_MQTT_Publish temperatureHumidityWriteFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
  Adafruit_MQTT_Publish listWriteFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/list");
#endif

bool inicio = true;

// Inicia sensor DHT
DHT dht(D6, DHT11);

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorLinkedList = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationLinkedList = ListaEncadeada<Application*>();

std::unique_ptr<BearSSL::WiFiClientSecure> clientSecureBearSSL (new BearSSL::WiFiClientSecure);

void sendResponse(int statusCode, char route[], const char type[], char data[]) {
  Serial.println(data);
  http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
  http_rest_server.sendHeader("Location", route);
  http_rest_server.send(statusCode, type, data);
}

void sendResponseJson(int statusCode, char route[], char data[]) {
  sendResponse(statusCode, route, "application/json", data);
}

void sendResponseHtml(int statusCode, char route[], char data[]) {
  sendResponse(statusCode, route, "text/html", data);
}

void loadSensorList()
{
  addSensor(1, D5, LOW, "eyes");
  addSensor(2, D7, LOW, "hat");
  addSensor(3, D6, LOW, "temperature");
  addSensor(4, D8, LOW, "blink");
  addSensor(5, D2, LOW, "Bluetooth RX");
  addSensor(6, D3, LOW, "Bluetooth TX");
}

void addSensor(byte id, byte gpio, byte status, const char * name) {
  ArduinoSensorPort *arduinoSensorPort = new ArduinoSensorPort(); 
  arduinoSensorPort->id = id;
  arduinoSensorPort->gpio = gpio;
  arduinoSensorPort->status = status;
  strcpy(arduinoSensorPort->name,name);
  pinMode(gpio, OUTPUT);

  // Adiciona sensor na lista
  sensorLinkedList.add(arduinoSensorPort);
}

ArduinoSensorPort * searchListSensor(byte gpio) {  
  ArduinoSensorPort *arduinoSensorPort;
  for(int i = 0; i < sensorLinkedList.size(); i++){
    // Obtem a aplicação da lista
    arduinoSensorPort = sensorLinkedList.get(i);
    if (gpio == arduinoSensorPort->gpio) {
      return arduinoSensorPort;
    }
  }
  return NULL;
}

int init_wifi() {
  int retries = 0;
  Serial.println("Conectando com o ponto de acesso WiFi..........");
  WiFi.hostname("minion");
  WiFi.begin(wifi_ssid, wifi_passwd);
  // checa o estado da conexao WiFi para ser: WL_CONNECTED
  while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
      retries++;
      delay(WIFI_RETRY_DELAY);
      Serial.print(".");
  }  
  clientSecureBearSSL->setFingerprint(fingerprint);
  Serial.println(WiFi.hostname());
  return WiFi.status(); // retorna o estado da conexao WiFi
}

void handleNotFound() {
  String message = "Arquivo nao encontrado\n\n";
  message += "URI: ";
  message += http_rest_server.uri();
  message += "\nMetodo: ";
  message += (http_rest_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArgumentos: ";
  message += http_rest_server.args();
  message += "\n";
  for (uint8_t i = 0; i < http_rest_server.args(); i++) {
    message += " " + http_rest_server.argName(i) + ": " + http_rest_server.arg(i) + "\n";
  }  
  sendResponseJson(HTTP_NOT_FOUND, "", "{\"message\":\" + message + \"}");
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

/* D5 */
void putEye() {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);    
  char JSONmessageBuffer[MAX_STRING_LENGTH];
  
  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
  if (error) {
    char mensagem[]="Erro ao fazer parser do json";
    sendResponseJson(HTTP_BAD_REQUEST, "/eye", mensagem);
  }
  else {
    readBodySensorData(doc, D5);    
    sendResponseJson(HTTP_OK, "/eye", JSONmessageBuffer);
 }
}

void readSensor(char * name, byte port){
  char data[MAX_STRING_LENGTH]="";
  char id [MAX_LENGTH];
  char gpio [MAX_LENGTH];
  char status [MAX_LENGTH];
  ArduinoSensorPort *arduinoSensorPort = searchListSensor(port);  
  itoa (arduinoSensorPort->id,id,10);
  itoa (arduinoSensorPort->gpio,gpio,10);
  itoa (arduinoSensorPort->status,status,10);
  sprintf(data, "{\"id\":\"%s\",\"gpio\":\"%s\",\"status\":\"%s\"}", id, gpio, status);
  sendResponseJson(HTTP_OK, name, data);
}

void getEyes() {
  readSensor("eyes", D5);
}

/* ByPass mensagem para o google text to speech e depois enviar para o google home via bluetooth */
void postTalk() {  
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  char urlText2Speech[MAX_STRING_LENGTH]="";
  char data[MAX_STRING_LENGTH]="";

  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
  if (error) {
    char mensagem[]="Erro ao fazer parser do json";
    sprintf(data, "{\"message\": \"%s\"}", mensagem);
    sendResponseJson(HTTP_BAD_REQUEST, "/talk", data);
  }
  else {    
    sprintf(urlText2Speech,"https://%s/v1beta1/text:synthesize?key=%s", text2SpeechHost, API_TEXT2SPEECH_KEY);
    const char* mensagem = doc["mensagem"];
    sprintf(data, "{\"input\": {\"text\": \"%s\"},\"voice\": {\"languageCode\": \"%s\",\"name\": \"%s\",\"ssmlGender\": \"%s\"},\"audioConfig\": {\"audioEncoding\": \"%s\"}}", mensagem, languageCode, voiceName, ssmlGender, audioEncoding);
    char * payload = postUtil(urlText2Speech, data);
    sendResponseJson(HTTP_OK, "/talk", payload);
  }
}

char * postUtil(char * url, char * httpRequestData) { 
  char * payload = NULL;
  int i = 0;
  HTTPClient http;  
  http.begin(*clientSecureBearSSL, String(url));
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(httpRequestData);
  if (httpCode > 0) {
    WiFiClient * cliente = http.getStreamPtr();
    while (cliente->available()) {
      char c = cliente->read();
      /*
      payload = (char*)malloc( strlen(payload) * sizeof(char) );
      payload[i] = c;
      i++;
      */
      Serial.print(c);
    }
    Serial.flush();
  } else {
    payload = "{\"message\": \"Erro na requisição HTTP\"}";
  }
  http.end();
  Serial.println("payload: " + String(payload));
  return payload;
}

/* ByPass mensagem para o google home */
void postLaugh() {
  char data[MAX_STRING_LENGTH]="";
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  String post_body = http_rest_server.arg("plain");
  Serial.println(post_body);

  DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
  char JSONmessageBuffer[MAX_STRING_LENGTH];
  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  if (error) {
    char mensagem[]="Erro ao fazer parser do json";    
    sprintf(data, "{\"message\": \"%s\"}", mensagem);
    sendResponseJson(HTTP_BAD_REQUEST, "/laugh", data);
  }
  else {
    sendResponseJson(HTTP_OK, "/laugh", JSONmessageBuffer);
  }
}

/* D7 */
void putHat() {
  char data[MAX_STRING_LENGTH]="";
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  String put_body = http_rest_server.arg("plain");
  Serial.println(put_body);
  DeserializationError error = deserializeJson(doc, http_rest_server.arg("plain"));
  char JSONmessageBuffer[MAX_STRING_LENGTH];
  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  if (error) {
    char mensagem[]="Erro ao fazer parser do json";    
    sprintf(data, "{\"message\": \"%s\"}", mensagem);
    sendResponseJson(HTTP_BAD_REQUEST, "/hat", data);    
  }
  else {
    readBodySensorData(doc, D7);
    sendResponseJson(HTTP_OK, "/hat", JSONmessageBuffer);
  }
}

void getHats() {
  readSensor("hats",D7);
}

void getPorts() {
  char JSONmessage[MAX_LIST_STRING_LENGTH]="[";
  char JSONmessageTemp[MAX_STRING_LENGTH];
  ArduinoSensorPort *arduinoSensorPort;
  for(int i = 0; i < sensorLinkedList.size(); i++){
    // Obtem a aplicação da lista
    arduinoSensorPort = sensorLinkedList.get(i);
    sprintf(JSONmessageTemp, "{\"id\": \"%d\",\"gpio\": \"%d\",\"status\": \"%d\",\"name\": \"%s\"}", arduinoSensorPort->id, arduinoSensorPort->gpio, arduinoSensorPort->status, arduinoSensorPort->name);
    strcat(JSONmessage, JSONmessageTemp);
    if((i < sensorLinkedList.size()) && (i < sensorLinkedList.size()-1)) {
      strcat(JSONmessage, ",");
    }
  }
  strcat(JSONmessage, "]");
  sendResponseJson(HTTP_OK, "/ports", JSONmessage);
}

void getLists() {
  char JSONmessage[MAX_LIST_STRING_LENGTH]="[";
  char JSONmessageTemp[MAX_STRING_LENGTH];
  Application *app;
  for(int i = 0; i < applicationLinkedList.size(); i++){
    // Obtem a aplicação da lista
    app = applicationLinkedList.get(i);
    sprintf(JSONmessageTemp, "{\"name\": \"%s\",\"language\": \"%s\",\"description\": \"%s\"}", app->name, app->language, app->description);
    strcat(JSONmessage, JSONmessageTemp);
    if((i < applicationLinkedList.size()) && (i < applicationLinkedList.size()-1)) {
      strcat(JSONmessage, ",");
    }
  }
  strcat(JSONmessage, "]");
  sendResponseJson(HTTP_OK, "/lists", JSONmessage);
}

void addApplication(const char * name, const char * language, const char * description) {
  Application *app = new Application();
  strcpy(app->name,name);
  strcpy(app->language,language);
  strcpy(app->description,description);

  // Adiciona a aplicação na lista
  applicationLinkedList.add(app);
}

void loadListFromAdafruit() {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  char json[MAX_STRING_LENGTH];

  #ifdef MQTT
  // ler do feed list no adafruit
  strcat(json,readFeedFromAdafruitList());
  #endif
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.println("falha ao fazer o deserializeJson()");
    return;
  }  
  // para cada objeto no array
  Application *item;
  for(int i = 0; i < applicationLinkedList.size(); i++) {
    addApplication(item->name, item->language, item->description);  
  }
}

int searchListAdafruit(DynamicJsonDocument doc) {  
  Application *app;
  for(int i = 0; i < applicationLinkedList.size(); i++){
    // Obtem a aplicação da lista
    app = applicationLinkedList.get(i);
    if (doc["name"] == app->name && doc["language"]==app->language) {
      return i;
    }
  }
  return -1;
}

void postList() {
  char data[MAX_STRING_LENGTH]="";
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  String post_body = http_rest_server.arg("plain");
  Serial.println(post_body);
  int ret = HTTP_CONFLICT;
  char JSONmessageBuffer[MAX_STRING_LENGTH] = "[]";
  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  DeserializationError error = deserializeJson(doc, post_body);
  if (error) {
    char mensagem[]="Erro ao fazer parser do json";    
    sprintf(data, "{\"message\": \"%s\"}", mensagem);
    sendResponseJson(HTTP_BAD_REQUEST, "/list", data);
  }
  else {
    //busco para checar se aplicacao já existe
    int index = searchListAdafruit(doc);
    if(index == -1) {
      // não existe, então posso inserir
      // adiciona item na lista de aplicações jenkins
      const char* name = doc["name"];
      const char* language = doc["language"];
      const char* description = doc["description"];
      addApplication(name, language, description);
      ret = HTTP_OK;
      sprintf(JSONmessageBuffer, "{\"name\": \"%s\",\"language\": \"%s\",\"description\": \"%s\"}", name, language, description);
      
      #ifdef MQTT
      // Grava no Adafruit
      writeFeedToAdafruitList(JSONmessageBuffer);
      #endif

      sendResponseJson(ret, "/list", JSONmessageBuffer);
    }
    sendResponseJson(ret, "/list", JSONmessageBuffer);
  }

}

void delList() {
  String delete_body = http_rest_server.arg("plain");
  Serial.println(delete_body);
  int ret = HTTP_NOT_FOUND;
  DynamicJsonDocument doc = treatDataString(delete_body);
  if(doc.isNull()) http_rest_server.send(HTTP_BAD_REQUEST);
  else {
    char JSONmessage[MAX_STRING_LENGTH]="[";
    char JSONmessageTemp[MAX_STRING_LENGTH];
    Serial.print("Metodo HTTP: ");
    Serial.println(http_rest_server.method());
    //busco pela aplicacao a ser removida
    int index = searchListAdafruit(doc);
    if(index != -1) {
      //removo
      applicationLinkedList.remove(index);      
      ret = HTTP_OK;

      Application *app;
      sprintf(JSONmessage, "[");
      for(int i = 0; i < applicationLinkedList.size(); i++){
        // Obtem a aplicação da lista
        app = applicationLinkedList.get(i);
        sprintf(JSONmessageTemp, "{\"name\": \"%s\",\"language\": \"%s\",\"description\": \"%s\"}", app->name, app->language, app->description);
        strcat(JSONmessage, JSONmessageTemp);
        if((i < applicationLinkedList.size()) && (i < applicationLinkedList.size()-1)) {
          strcat(JSONmessage, ",");
        }
      }
      strcat(JSONmessage, "]");

      #ifdef MQTT
      // Grava no Adafruit
      writeFeedToAdafruitList(JSONmessage);
      #endif
    }
    sendResponseJson(HTTP_BAD_REQUEST, "/list/del", JSONmessage);
  }
}

char * getTemperatureAndHumidity() {
    char JSONmessageBuffer[MAX_STRING_LENGTH];
    // Leituras do sensor podem demorar até 2 segundos (o sensor e muito lento)
    float humidity = dht.readHumidity();
    // Le a temperatura como Celsius (padrao)
    float celsius = dht.readTemperature();
    // Le a temperatura como Fahrenheit (isFahrenheit = true)
    float fahrenheit = dht.readTemperature(true);
    // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
    if (isnan(humidity) || isnan(celsius) || isnan(fahrenheit)) {
      Serial.println("Falha para ler do sensor DHT!");
      celsius = 0.0;
      fahrenheit = 0.0;
      humidity = 0.0;
    }
    sprintf(JSONmessageBuffer, "{\"celsius\": \"%0.1f\",\"fahrenheit\": \"%0.1f\",\"umidade\": \"%0.1f\"}", celsius, fahrenheit, humidity);
    Serial.println("temperatura:"+String(JSONmessageBuffer));
    return JSONmessageBuffer;
}

/* D6 */
void getTemperature() {
  char JSONmessageBuffer[MAX_STRING_LENGTH]="";
  strcat(JSONmessageBuffer,getTemperatureAndHumidity());
  Serial.println("Mensagem:"+String(JSONmessageBuffer));
  sendResponseJson(HTTP_OK, "/temperature", JSONmessageBuffer);
}
  
/* D8 */
void putBlink() {
  Serial.print("Metodo HTTP: ");
  Serial.println(http_rest_server.method());
  String dado = http_rest_server.arg("plain");
  DynamicJsonDocument doc = treatDataString(dado);
  if(doc.isNull()) http_rest_server.send(HTTP_BAD_REQUEST);
  else {
    readBodySensorData(doc, D8);
    http_rest_server.sendHeader("Location", "/blink");
    http_rest_server.send(HTTP_OK, "application/json", dado);    
  }
}

void getBlinks() {
  readSensor("blinks", D8);
}

void getSwaggerJson() { 
  sendResponseJson(HTTP_OK, "/swagger.json", swaggerJSON);
}
 
void getSwaggerUI() { 
  sendResponseHtml(HTTP_OK, "/swaggerUI", swaggerUI);
}

void config_rest_server_routing() {
  http_rest_server.on("/health", HTTP_GET, []() {
    sendResponseHtml(HTTP_OK, "/health", "<p>Bem vindo ao Minion ESP8266 REST Web Server</p>");
  });
  http_rest_server.on("/ports", HTTP_GET, getPorts);
  http_rest_server.on("/talk", HTTP_POST, postTalk);
  http_rest_server.on("/eye", HTTP_PUT, putEye);
  http_rest_server.on("/eyes", HTTP_GET, getEyes);
  http_rest_server.on("/laugh", HTTP_POST, postLaugh);
  http_rest_server.on("/hat", HTTP_PUT, putHat);    
  http_rest_server.on("/hats", HTTP_GET, getHats);
  http_rest_server.on("/blink", HTTP_PUT, putBlink);    
  http_rest_server.on("/blinks", HTTP_GET, getBlinks);  
  http_rest_server.on("/list", HTTP_POST, postList);
  http_rest_server.on("/lists", HTTP_GET, getLists);
  http_rest_server.on("/list/del", HTTP_DELETE, delList);
  http_rest_server.on("/temperature", HTTP_GET, getTemperature);
  http_rest_server.on("/swagger.json", HTTP_GET, getSwaggerJson);
  http_rest_server.on("/swaggerUI", HTTP_GET, getSwaggerUI);
  http_rest_server.onNotFound(handleNotFound);
}

void writeFeedToAdafruitTemperatureAndHumidity(const char * json) {
  #ifdef MQTT
    // publicando dados de temperatura em celsius, em fahrenheit e a umidade
    if (!temperatureHumidityWriteFeed.publish(json)) {
      Serial.println(F("Falhou ao gravar a temperatura e a ummidade no adafruit"));
    } else {
      Serial.println(F("Gravacao da temperatura e da umidade efetuada no adafruit com sucesso"));
    }
  #endif
}

void writeFeedToAdafruitList(const char * json) {
  #ifdef MQTT  
  // publicando dados de temperatura em celsius, em fahrenheit e a umidade
  if (!listWriteFeed.publish(json)) {
    Serial.println(F("Falhou ao gravar a lista de aplicações jenkins no adafruit"));
  } else {
    Serial.println(F("Gravacao da lista de aplicações jenkins no adafruit com sucesso"));
  }
  #endif
}

void readFeedFromAdafruitEyes() {
  #ifdef MQTT
    Serial.print(F("Lido: "));
    Serial.println((char *)eyesReadFeed.lastread);
    
    if (EEPROM.read(0) == HIGH){
      digitalWrite(RelayEyes, LOW);
      EEPROM.write(0, LOW);
    }
    else {
      digitalWrite(RelayEyes, HIGH);
      EEPROM.write(0, HIGH);
    }
  #endif
}

void readFeedFromAdafruitHat() {
  #ifdef MQTT
    Serial.print(F("Lido: "));
    Serial.println((char *)hatReadFeed.lastread);
    
    if (EEPROM.read(1) == HIGH){
      digitalWrite(RelayHat, LOW);
      EEPROM.write(1, LOW);
    }
    else {
      digitalWrite(RelayHat, HIGH);
      EEPROM.write(1, HIGH);
    }
  #endif
}

void readFeedFromAdafruitBlink() {
  Serial.print(F("Lido: "));
  Serial.println((char *)blinkReadFeed.lastread);
  
  if (EEPROM.read(2) == HIGH){
    digitalWrite(RelayBlink, LOW);
    EEPROM.write(2, LOW);
  }
  else {
    digitalWrite(RelayBlink, HIGH);
    EEPROM.write(2, HIGH);
  }
}

char * readFeedFromAdafruitList() {
  #ifdef MQTT
    Serial.print(F("Lido: "));
    Serial.println((char *)listReadFeed.lastread);
    return (char *)listReadFeed.lastread;
  #endif
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  #ifdef MQTT  
    int8_t ret;
  
    // Stop if already connected.
    if (mqtt.connected()) {
      return;
    }
  
    Serial.print("Connecting to MQTT... ");
  
    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
         Serial.println(mqtt.connectErrorString(ret));
         Serial.println("Retrying MQTT connection in seconds...");
         mqtt.disconnect();
         delay(MQTT_DELAY);  // wait seconds
    }
    Serial.println("MQTT Connected!");
  #endif
}

void lastState() {
  // eyes
  if (EEPROM.read(0) == LOW){
    EEPROM.write(0, HIGH);
  }
  else {
    EEPROM.write(0, LOW);
  }
  // hat
  if (EEPROM.read(1) == LOW){
    EEPROM.write(1, HIGH);
  }
  else {
    EEPROM.write(1, LOW);
  }
  // blink
  if (EEPROM.read(2) == LOW){
    EEPROM.write(2, HIGH);
  }
  else {
    EEPROM.write(2, LOW);
  }  
}

void incomingSubscriptionPackets(){
  #ifdef MQTT
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(MQTT_DELAY))) {
      if (subscription == &eyesReadFeed) {
        readFeedFromAdafruitEyes();
      }
      if (subscription == &hatReadFeed) {
        readFeedFromAdafruitHat();
      }
      if (subscription == &blinkReadFeed) {
        readFeedFromAdafruitBlink();
      }
      if (subscription == &listReadFeed) {
        readFeedFromAdafruitList();
      }
    }
  #endif
}

void writeFeedTemperature(){
  delay(TEMPERATURE_DELAY);
  char temperatureData[MAX_STRING_LENGTH]="";
  strcat(temperatureData,getTemperatureAndHumidity());
  writeFeedToAdafruitTemperatureAndHumidity(temperatureData);
}

String getData(uint8_t *data, size_t len) {
  char raw[len];
  for (size_t i = 0; i < len; i++) {
    //Serial.write(data[i]);
    raw[i] = data[i];
  }
  return String(raw);
}

DynamicJsonDocument treatData(uint8_t *data, size_t len) {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  String JSONmessageBody = getData(data, len);
  DeserializationError error = deserializeJson(doc, JSONmessageBody);
  return doc;
}

DynamicJsonDocument treatDataString(String JSONmessageBody) {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  DeserializationError error = deserializeJson(doc, JSONmessageBody);
  return doc;
}

void setup(void) {
  loadSensorList();
  
  // DH11 inicia temperatura
  dht.begin();
  
  pinMode(RelayEyes, OUTPUT);
  pinMode(RelayHat, OUTPUT);
  pinMode(TemperatureHumidity, OUTPUT);
  EEPROM.begin(512);
  lastState(); //recover last state of relays
  
  Serial.begin(SERIAL_PORT);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_passwd);
  if (init_wifi() == WL_CONNECTED) {
    Serial.print("Conectado com ");
    Serial.print(wifi_ssid);
    Serial.print("--- IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.print("Erro ao conectar com: ");
    Serial.println(wifi_ssid);
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  #ifdef MQTT
    // Setup MQTT subscription for onoff feed.
    mqtt.subscribe(&eyesReadFeed);
    mqtt.subscribe(&hatReadFeed);
    mqtt.subscribe(&blinkReadFeed);
    mqtt.subscribe(&listReadFeed);

    //carrega a lista do feed list no adafruit  
    loadListFromAdafruit();
  #endif

  config_rest_server_routing();
  http_rest_server.begin();
  Serial.println("Servidor HTTP REST Iniciado");
}

void loop(void) {
  #ifdef MQTT
    MQTT_connect();
    if(inicio) {
      inicio=false;
      writeFeedToAdafruitList("[]");
    }
    incomingSubscriptionPackets();
    //writeFeedTemperature();
    // ping the server to keep the mqtt connection alive
    if(! mqtt.ping()) {
      mqtt.disconnect();
    }
  #endif

  http_rest_server.handleClient();
}
