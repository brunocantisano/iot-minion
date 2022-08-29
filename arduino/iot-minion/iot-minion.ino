#include "Credentials.h"
#include "ListaEncadeada.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecureBearSSL.h>

#define MQTT_DELAY                 5000
#define MQTT_CONN_KEEPALIVE        180
#define SERIAL_PORT                115200
#define TEMPERATURE_DELAY          60000
//Rest API
#define HTTP_REST_PORT             80
#define WIFI_RETRY_DELAY           500
#define MAX_WIFI_INIT_RETRY        50
#define MAX_LENGTH                 10
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

#define RelayEyes                  D5
#define RelayHat                   D7
#define RelayBlink                 D8
#define TemperatureHumidity        D6
#define MAX_STRING_LENGTH          200
#define MAX_PATH                   256
#define MAX_LIST_STRING_LENGTH     1024
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

const char WRONG_CLIMATE[] PROGMEM = "Erro desconhecido ao buscar temperatura e umidade";
const char WRONG_AUTHORIZATION[] PROGMEM = "Authorization token errado";
const char WRONG_STATUS[] PROGMEM = "Erro ao atualizar o status";
const char PLAYED[] PROGMEM = "Arquivo foi colocado para tocar.";
const char EXISTING_ITEM[] PROGMEM = "Item já existente na lista";
const char REMOVED_ITEM[] PROGMEM = "Item removido da lista";
const char NOT_FOUND_ITEM[] PROGMEM = "Item não encontrado na lista";
const char NOT_FOUND_ROUTE[] PROGMEM = "Rota nao encontrada";
const char PARSER_ERROR[] PROGMEM = "{\"message\": \"Erro ao fazer parser do json\"}";
const char WEB_SERVER_CONFIG[] PROGMEM = "\nConfiguring Webserver ...";
const char WEB_SERVER_STARTED[] PROGMEM = "Webserver started";
const char MIME_TYPE_JPG[] PROGMEM = "image/jpg";
const char MIME_TYPE_PNG[] PROGMEM = "image/png";
const char MIME_TYPE_ICO[] PROGMEM = "image/ico";
const char FILE_TYPE_CSS[] PROGMEM = "text/css";
const char FILE_TYPE_HTML[] PROGMEM = "text/html";
const char FILE_TYPE_JSON[] PROGMEM = "application/json";
const char FILE_TYPE_TEXT[] PROGMEM = "text/plain";

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
typedef enum {
  celsius,
  fahrenheit,
  humidity
} temperature_dht;
/* versão do firmware */
const char version[] PROGMEM = API_VERSION;
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
AsyncWebServer *server;               // initialise webserver

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Setup feeds called 'eye', 'hat', 'blink' and 'temperature' for subscribing to changes.
Adafruit_MQTT_Subscribe eyeReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/eye"); // set adafruit FeedName
Adafruit_MQTT_Subscribe hatReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/hat"); // set adafruit FeedName
Adafruit_MQTT_Subscribe blinkReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/blink"); // set adafruit FeedName
Adafruit_MQTT_Subscribe temperatureReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/temperature"); // set adafruit FeedName
Adafruit_MQTT_Subscribe listReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/list"); // set adafruit FeedName
Adafruit_MQTT_Subscribe talkReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/talk"); // set adafruit FeedName
Adafruit_MQTT_Subscribe playReadFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME"/feeds/play"); // set adafruit FeedName

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish eyeWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/eye");
Adafruit_MQTT_Publish hatWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/hat");
Adafruit_MQTT_Publish blinkWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/blink");
Adafruit_MQTT_Publish temperatureWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish listWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/list");
Adafruit_MQTT_Publish talkWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/talk");
Adafruit_MQTT_Publish playWriteFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/play");

bool inicio = true;

// Inicia sensor DHT
DHT dht(D6, DHT11);

// Lista de sensores
ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();

// Lista de aplicacoes do jenkins
ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();

std::unique_ptr<BearSSL::WiFiClientSecure> clientSecureBearSSL (new BearSSL::WiFiClientSecure);

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
  sensorListaEncadeada.add(arduinoSensorPort);
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

int init_wifi() {
  int retries = 0;
  Serial.println("Conectando com o ponto de acesso WiFi..........");
  WiFi.hostname("minion");
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
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

void handle_OnError(){
  server->onNotFound([](AsyncWebServerRequest *request) {
    if(request->method() == HTTP_OPTIONS) {
      request->send(HTTP_NO_CONTENT);
    }
    request->send(HTTP_NOT_FOUND, FILE_TYPE_TEXT, NOT_FOUND_ROUTE);
  });
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

void handle_Sensors() {
  server->on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {
    //"/sensors?type=eye"
    //"/sensors?type=hat"
    //"/sensors?type=blink"
    if(check_authorization_header(request)) {
      int relayPin = RelayEyes;
      int paramsNr = request->params();
      for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        if (strcmp("hat", p->value().c_str())==0){
          relayPin = RelayHat;
        }
        else if (strcmp("blink", p->value().c_str())==0){
          relayPin = RelayBlink;
        }      
      }
      request->send(HTTP_OK, FILE_TYPE_JSON, readSensor(relayPin));
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
}

void handle_UpdateSensors(){
  //"/sensor?type=eye"
  //"/sensor?type=hat"
  //"/sensor?type=blink"
  server->on("/sensor", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      int sensor = RelayEyes;
      String feedName = "eye";
      int paramsNr = request->params();
      for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        feedName = p->value();
        if(strcmp("hat", p->value().c_str())==0){
          sensor = RelayHat;
        } else if (strcmp("blink", p->value().c_str())==0){
          sensor = RelayBlink;
        }
      }
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, FILE_TYPE_JSON, PARSER_ERROR);
      } else {
        String JSONmessage;
        if(readBodySensorData(doc["status"], sensor)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(sensor);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          if(strcmp("hat", arduinoSensorPort->name)==0){
            eyeWriteFeed.publish(arduinoSensorPort->status==0?"OFF":"ON");  
          } else if(strcmp("hat", arduinoSensorPort->name)==0){
            hatWriteFeed.publish(arduinoSensorPort->status==0?"OFF":"ON");  
          } else if (strcmp("blink", arduinoSensorPort->name)==0){
            blinkWriteFeed.publish(arduinoSensorPort->status==0?"OFF":"ON");  
          }
          doc.clear();
          request->send(HTTP_OK, FILE_TYPE_JSON, JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, FILE_TYPE_TEXT, WRONG_STATUS);
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
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

void handle_Swagger(){
  server->on("/swagger.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = swaggerJSON;
    html.replace("0.0.0",version);
    html.replace("HOST_MINION",String(HOST)+".local");
    request->send(HTTP_OK, FILE_TYPE_JSON, html);
  });
}

void handle_SwaggerUI(){
  server->on("/swaggerUI", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = swaggerUI;
    html.replace("HOST_MINION",String(HOST)+".local");
    request->send(HTTP_OK, FILE_TYPE_HTML, html);
  });  
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

String IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

void handle_Health(){
  server->on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
    String mqttConnected = client.connected()?"true":"false";
    String JSONmessage = "{\"greeting\": \"Bem vindo ao Minion ESP32 REST Web Server\",\"date\": \""+getDataHora()+"\",\"url\": \"/health\",\"mqtt\": \""+mqttConnected+"\",\"version\": \""+version+"\",\"ip\": \""+IpAddress2String(WiFi.localIP())+"\"}";
    request->send(HTTP_OK, FILE_TYPE_JSON, JSONmessage);
  });
}

void handle_TemperatureAndHumidity(){
  //http://minion.local/climate?type=celsius
  server->on("/climate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      int paramsNr = request->params();
      for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(strcmp("celsius", p->value().c_str())==0){
          request->send(HTTP_OK, FILE_TYPE_JSON, treatTemperatureAndHumidity("celsius", getTemperatureHumidity(celsius)));
        } else if (strcmp("fahrenheit", p->value().c_str())==0){
          request->send(HTTP_OK, FILE_TYPE_JSON, treatTemperatureAndHumidity("fahrenheit", getTemperatureHumidity(fahrenheit)));
        }
        else if (strcmp("humidity", p->value().c_str())==0){
          request->send(HTTP_OK, FILE_TYPE_JSON, treatTemperatureAndHumidity("humidity", getTemperatureHumidity(humidity)));
        }
      }
      request->send(HTTP_BAD_REQUEST, FILE_TYPE_TEXT, WRONG_CLIMATE);
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
}

String treatTemperatureAndHumidity(String field, String value)
{
  String JSONmessage="{\""+field+"\": \""+value+"\"}";
  #ifdef DEBUG
    Serial.println(field+": "+JSONmessage);
  #endif
  return JSONmessage;
}

/* ByPass mensagem para o google text to speech e depois enviar para o google home via bluetooth */
void handle_InsertTalk(){
  server->on("/talk", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {      
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      char urlText2Speech[MAX_STRING_LENGTH]="";
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, FILE_TYPE_JSON, PARSER_ERROR);
      } else {
        #ifdef DEBUG
          Serial.printf("Mensagem: %s\n",doc["mensagem"]);
        #endif        
        String feedName="talk";

        sprintf(urlText2Speech,"https://%s/v1beta1/text:synthesize?key=%s", text2SpeechHost, API_TEXT2SPEECH_KEY);
        String mensagem = doc["mensagem"];
        char json[MAX_LIST_STRING_LENGTH];
        sprintf(json, "{\"input\": {\"text\": \"%s\"},\"voice\": {\"languageCode\": \"%s\",\"name\": \"%s\",\"ssmlGender\": \"%s\"},\"audioConfig\": {\"audioEncoding\": \"%s\"}}", mensagem.c_str(), languageCode, voiceName, ssmlGender, audioEncoding);
        char * payload = postUtil(urlText2Speech, json);
        // publish
        talkWriteFeed.publish(mensagem.c_str());
        doc.clear();
        request->send(HTTP_OK, FILE_TYPE_TEXT, PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
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

void playMidia(const char * midia)
{
  char filenameMidia[strlen(midia)+1];
  filenameMidia[0]='/';
  filenameMidia[1]='\0';
  strcat(filenameMidia, midia);
  #ifdef DEBUG
    Serial.printf("Arquivo a tocar: %s\n",filenameMidia);
  #endif  
}

/* ByPass mensagem para o minion */
void handle_InsertPlay(){
  server->on("/play", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, FILE_TYPE_JSON, PARSER_ERROR);
      } else {
        const char * midia = doc["midia"];
        #ifdef DEBUG
          Serial.printf("Arquivo: %s\n",midia);
        #endif        
        String feedName="play";
        // publish
        playWriteFeed.publish(midia);        
        // toca o audio
        playMidia(midia);
        doc.clear();
        request->send(HTTP_OK, FILE_TYPE_TEXT, PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
}

void handle_Ports(){
  server->on("/ports", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage;
      ArduinoSensorPort *arduinoSensorPort;    
      for(int i = 0; i < sensorListaEncadeada.size(); i++){
        // Obtem a aplicação da lista
        arduinoSensorPort = sensorListaEncadeada.get(i);
        arduinoSensorPort->status = digitalRead(arduinoSensorPort->gpio);
        JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"status\": \""+String(arduinoSensorPort->status)+"\",\"name\": \""+String(arduinoSensorPort->name)+"\"},";
      }
      request->send(HTTP_OK, FILE_TYPE_JSON, '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });  
}

void handle_Lists(){
  server->on("/lists", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {      
      String JSONmessage;
      Application *app;
      for(int i = 0; i < applicationListaEncadeada.size(); i++){
        // Obtem a aplicação da lista
        app = applicationListaEncadeada.get(i);
        JSONmessage += "{\"id\": "+String(i+1)+",\"name\": \""+app->name+"\",\"language\": \""+app->language+"\",\"description\": \""+app->description+"\"}"+',';
      }
      request->send(HTTP_OK, FILE_TYPE_JSON, '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
    }
  });
}

void addApplication(const char * name, const char * language, const char * description) {
  Application *app = new Application();
  strcpy(app->name,name);
  strcpy(app->language,language);
  strcpy(app->description,description);

  // Adiciona a aplicação na lista
  applicationListaEncadeada.add(app);
}

void loadListFromAdafruit() {
  DynamicJsonDocument doc(MAX_STRING_LENGTH);
  char json[MAX_STRING_LENGTH];

  // ler do feed list no adafruit
  strcat(json,readFeedFromAdafruitList());
  DeserializationError error = deserializeJson(doc, json);
  // Test if parsing succeeds.
  if (error) {
    Serial.println("falha ao fazer o deserializeJson()");
    return;
  }  
  // para cada objeto no array
  Application *item;
  for(int i = 0; i < applicationListaEncadeada.size(); i++) {
    addApplication(item->name, item->language, item->description);  
  }
  doc.clear();
}

int searchList(String name, String language) {
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    if (name == app->name && language == app->language) {
      return i;
    }
  }
  return -1;
}
void handle_InsertItemList(){
  server->on("/list", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, FILE_TYPE_JSON, PARSER_ERROR);
      } else {
        //busco para checar se aplicacao já existe
        int index = searchList(doc["name"],doc["language"]);
        if(index == -1) {
          String JSONmessage;
          // não existe, então posso inserir
          // adiciona item na lista de aplicações jenkins 
          addApplication(doc["name"], doc["language"], doc["description"]);  
       
          Application *app;
          for(int i = 0; i < applicationListaEncadeada.size(); i++){
            // Obtem a aplicação da lista
            app = applicationListaEncadeada.get(i);
            JSONmessage="{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}";
            if((i < applicationListaEncadeada.size()) && (i < applicationListaEncadeada.size()-1)) {
              JSONmessage+=',';
            }
          }
          JSONmessage='[' + JSONmessage +']';
          #ifdef DEBUG
            Serial.println("handle_InsertItemList:"+JSONmessage);
          #endif
          // publish         
          listWriteFeed.publish(JSONmessage.c_str());
          doc.clear();
          request->send(HTTP_OK, FILE_TYPE_JSON, JSONmessage);
        } else {
          request->send(HTTP_CONFLICT, FILE_TYPE_TEXT, EXISTING_ITEM);
        }
      }
   } else {
    request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
   }
  });
}

void handle_DeleteItemList(){
  server->on("/list/del", HTTP_DELETE, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, FILE_TYPE_JSON, PARSER_ERROR);
      } else {
      //busco pela aplicacao a ser removida
      int index = searchList(doc["name"],doc["language"]);
      if(index != -1) {
        String JSONmessage;
        //removo
        applicationListaEncadeada.remove(index);
        
        Application *app;
        for(int i = 0; i < applicationListaEncadeada.size(); i++){
          // Obtem a aplicação da lista
          app = applicationListaEncadeada.get(i);
          JSONmessage="{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}";
          if((i < applicationListaEncadeada.size()) && (i < applicationListaEncadeada.size()-1)) {
            JSONmessage+=',';
          }
        }
        JSONmessage='[' + JSONmessage +']';          
        #ifdef DEBUG
          Serial.println("handle_DeleteItemList:"+JSONmessage);
        #endif
        // Grava no adafruit
        // publish
        listWriteFeed.publish(JSONmessage.c_str());
        doc.clear();
        request->send(HTTP_OK, FILE_TYPE_TEXT, REMOVED_ITEM);
      } else {
        request->send(HTTP_NOT_FOUND, FILE_TYPE_TEXT, NOT_FOUND_ITEM);
      }
    }
   } else {
    request->send(HTTP_UNAUTHORIZED, FILE_TYPE_TEXT, WRONG_AUTHORIZATION);
   }
  });
}

String getTemperatureHumidity(temperature_dht tipo) {
  float valor = 0.0;
  String feedName = "temperature";
  if(dht.read()== 0){
    // Leituras do sensor podem demorar até 2 segundos (o sensor DHT11 é muito lento)
    switch(tipo) {
      case celsius:
        // Le a temperatura como Celsius (padrao)
        valor = dht.readTemperature();
        delay(2000);
        break;
      case fahrenheit:
        valor = dht.readTemperature(true);
        delay(2000);
        break;
      case humidity:
        valor = dht.readHumidity();
        feedName = "humidity";
        delay(2000);
        break;            
    }  
    // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
    if (isnan(valor)) {    
      #ifdef DEBUG
        Serial.println("Falha na leitura do tipo: "+String(tipo));
      #endif
      valor = 0.0;
    }      
  } else {
    valor = 0.0;
  }
  char buffer [MAX_PATH];
  snprintf ( buffer, MAX_PATH, "%.1f", valor );   
  // publish
  temperatureWriteFeed.publish(buffer);
  return String(buffer);
}

void readFeedFromAdafruitEyes() {
  Serial.print(F("Lido: "));
  Serial.println((char *)eyeReadFeed.lastread);
  
  if (EEPROM.read(0) == HIGH){
    digitalWrite(RelayEyes, LOW);
    EEPROM.write(0, LOW);
  }
  else {
    digitalWrite(RelayEyes, HIGH);
    EEPROM.write(0, HIGH);
  }
}

void readFeedFromAdafruitHat() {
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
  Serial.print(F("Lido: "));
  Serial.println((char *)listReadFeed.lastread);
  return (char *)listReadFeed.lastread;
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
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
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(MQTT_DELAY))) {
    if (subscription == &eyeReadFeed) {
      readFeedFromAdafruitEyes();
    }
    if (subscription == &hatReadFeed) {
      readFeedFromAdafruitHat();
    }
    if (subscription == &blinkReadFeed) {
      readFeedFromAdafruitBlink();
    }
    if (subscription == &temperatureReadFeed) {
       getTemperatureHumidity(celsius);
    }
    if (subscription == &listReadFeed) {
      readFeedFromAdafruitList();
    }
    if (subscription == &talkReadFeed) {
      Serial.println("Minion falou!");
    }
    if (subscription == &playReadFeed) {
      Serial.println("Minion tocou áudio!");
    }
  }
}

String getData(uint8_t *data, size_t len) {
  char raw[len];
  for (size_t i = 0; i < len; i++) {
    //Serial.write(data[i]);
    raw[i] = data[i];
  }
  return String(raw);
}

void startWebServer() {
  /* Webserver para se comunicar via browser com ESP32  */
  Serial.println(WEB_SERVER_CONFIG);
  server = new AsyncWebServer(HTTP_REST_PORT);

  /* 
   *  Rotas sem bloqueios de token na API
   *  Configura as páginas de login e upload 
   *  de firmware OTA 
   */
  // Rotas das imagens a serem usadas na página home e o Health (não estão com basic auth)
  handle_Health();
  handle_Swagger();
  handle_SwaggerUI();
  
  /*
   * Rotas bloqueadas pelo token authorization
   */
  handle_Ports();
  handle_Sensors();
  handle_Lists();
  handle_TemperatureAndHumidity();
  handle_InsertTalk();
  handle_InsertPlay();
  handle_UpdateSensors();
  handle_InsertItemList();
  handle_DeleteItemList();
  // ------------------------------------ //
  // se não se enquadrar em nenhuma das rotas
  handle_OnError();

  // permitindo todas as origens. O ideal é trocar o '*' pela url do frontend poder utilizar a api com maior segurança
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin, Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  // startup web server
  server->begin();
  
  #ifdef DEBUG
    Serial.println(WEB_SERVER_STARTED);
  #endif
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
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  if (init_wifi() == WL_CONNECTED) {
    Serial.print("Conectado com ");
    Serial.print(WIFI_SSID);
    Serial.print("--- IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.print("Erro ao conectar com: ");
    Serial.println(WIFI_SSID);
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&eyeReadFeed);
  mqtt.subscribe(&hatReadFeed);
  mqtt.subscribe(&blinkReadFeed);
  mqtt.subscribe(&temperatureReadFeed);
  mqtt.subscribe(&listReadFeed);
  mqtt.subscribe(&talkReadFeed);
  mqtt.subscribe(&playReadFeed);

  //carrega a lista do feed list no adafruit  
  loadListFromAdafruit();

  startWebServer();
  Serial.println("Servidor HTTP REST Iniciado");
}

void loop(void) {
  MQTT_connect();
  if(inicio) {
    inicio=false;
    listWriteFeed.publish("[]");
  }
  incomingSubscriptionPackets();
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}
