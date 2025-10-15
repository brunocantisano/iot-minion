// WebServerHandler.cpp
#include "WebServerHandler.h"

static const char* MSG_ARQUIVO_NAO_ENCONTRADO = "Provavelmente voce nao carregou os arquivos da pasta \"data\" (LittleFS) para o servidor!";
WebServerHandler::WebServerHandler(
    const String& token, 
    const String& version, 
    const String& hostServer, 
    const String& caller,
    ArduinoUtilsCds * cds):
                      apiToken(token),
                      apiVersion(version),
                      host(hostServer),
                      chatGPTUrl("https://api.openai.com/v1/chat/completions"),
                      callerOrigin(caller),
                      utilscds(cds)
{
  server = new AsyncWebServer(HTTP_REST_PORT);
  ws = new AsyncWebSocket("/ws");  
}

WebServerHandler::~WebServerHandler() {
    delete server;  // Free the allocated memory
    delete ws;
    delete utilscds;
}

String WebServerHandler::obtemMetricas() {
  String p = "";
  int sketch_size = ESP.getSketchSize();
  int flash_size =  ESP.getFreeSketchSpace();
  int available_size = flash_size - sketch_size;
  int heap_size = ESP.getHeapSize();
  int free_heap = ESP.getFreeHeap();
  int psram_size = ESP.getPsramSize();
  int free_psram_size = ESP.getFreePsram();
  const String boardName = "esp32";
  
  float temperature = ((temprature_sens_read() - 32) / 1.8);
   
  atribuiMetrica(&p, boardName+"_uptime", String(millis()));
  atribuiMetrica(&p, boardName+"_wifi_rssi", String(WiFi.RSSI()));
  atribuiMetrica(&p, boardName+"_sketch_size", String(sketch_size));
  atribuiMetrica(&p, boardName+"_flash_size", String(flash_size));
  atribuiMetrica(&p, boardName+"_available_size", String(available_size));
  atribuiMetrica(&p, boardName+"_heap_size", String(heap_size));
  atribuiMetrica(&p, boardName+"_free_heap", String(free_heap));
  atribuiMetrica(&p, boardName+"_psram_size", String(psram_size));
  atribuiMetrica(&p, boardName+"_free_psram_size", String(free_psram_size));
  atribuiMetrica(&p, boardName+"_temperature", String(temperature));
  atribuiMetrica(&p, boardName+"_boot_counter", String(obtemContagemBoots()));
  atribuiMetrica(&p, boardName+"_celsius", String(utilscds->obtemCelsius()));
  atribuiMetrica(&p, boardName+"_fahrenheit", String(utilscds->obtemFahrenheit()));
  atribuiMetrica(&p, boardName+"_humidity", String(utilscds->obtemUmidade()));
  atribuiMetrica(&p, boardName+"_heat_celsius", String(utilscds->obtemIndiceAquecimentoCelsius()));
  atribuiMetrica(&p, boardName+"_heat_fahrenheit", String(utilscds->obtemIndiceAquecimentoFahrenheit()));  
  atribuiMetrica(&p, boardName+"_eyes", obtemEstadoSensor(RelayEyes));
  atribuiMetrica(&p, boardName+"_hat", obtemEstadoSensor(RelayHat));
  atribuiMetrica(&p, boardName+"_blink", obtemEstadoSensor(RelayBlink));
  atribuiMetrica(&p, boardName+"_shake", obtemEstadoSensor(RelayShake));
  atribuiMetrica(&p, boardName+"_volume", String(utilscds->obtemVolumeAudio()));
  atribuiMetrica(&p, boardName+"_sdcard_total", utilscds->converteUint64ParaTexto(sdcard_total));
  atribuiMetrica(&p, boardName+"_sdcard_used", utilscds->converteUint64ParaTexto(sdcard_used));
  return p;
}

String WebServerHandler::obtemEstadoSensor(int pin) {
  return readSensorStable(pin) ? "1" : "0";
}

/**
   Layout

   # heltec_lora32_uptime
   # TYPE heltec_lora32_uptime gauge
   heltec_lora32_uptime 23899

*/
void WebServerHandler::atribuiMetrica(String *p, String metric, String value) {
  *p += "# " + metric + "\n";
  *p += "# TYPE " + metric + " gauge\n";
  *p += "" + metric + " ";
  *p += value;
  *p += "\n";
}

int WebServerHandler::obtemContagemBoots() {
  String boot = utilscds->salvaDado("storage", "boot", "0");
  return boot.toInt();
}

void WebServerHandler::incrementaContagemBoots() {
  int boot = obtemContagemBoots()+1;
  char buffer[10];
  sprintf(buffer, "%d", boot);
  const char* texto = buffer;
  utilscds->salvaDado("storage", "boot", texto);
}

bool WebServerHandler::check_authorization_header(AsyncWebServerRequest * request){
  int headers = request->headers();
  int i;
  for(i=0;i<headers;i++){
    const AsyncWebHeader* h = request->getHeader(i);
    //Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    if(h->name()=="Authorization" && h->value()=="Basic "+String(apiToken)){
      return true;
    }
  }
  return false;
}

void WebServerHandler::handleFileServing(void){
  server->on("/get-file", HTTP_GET, [this](AsyncWebServerRequest *request) {
    // "/get-file?name=delete.png"
    char filename[MAX_PATH];
    memset(filename, 0x00, MAX_PATH);

    if (request->hasParam("name")) {
      String file = request->getParam("name")->value();
      String safeFile = "/" + utilscds->sanitizaNomeArquivo(file);
      strlcpy(filename, safeFile.c_str(), MAX_PATH);
      request->send(LittleFS, filename, utilscds->obtemTipoMime(filename));
    } else {
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Parametro 'name' ausente");
    }
  });
}

void WebServerHandler::handleHome(){
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {    
    String html = "";
    if(!utilscds->lerArquivo(LittleFS, "/home.html", html)) {
      html=String(MSG_ARQUIVO_NAO_ENCONTRADO);
    } else {
      // versao do firmware: https://semver.org/
      html.replace("0.0.0",apiVersion);
      html.replace("MQTT_USERNAME",utilscds->obtemMqttUser());
      html.replace("HOST_MINION",host);
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), html);
  });
}

void WebServerHandler::handleCiCd() {
  server->on("/cicd", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html;
    if (utilscds->lerArquivo(LittleFS, "/cicd.html", html)) {
      html.replace("MQTT_BROKER", utilscds->obtemMqttBroker());
      html.replace("MQTT_USERNAME", utilscds->obtemMqttUser());
      html.replace("MQTT_PASSWORD", utilscds->obtemMqttPass());
    } else {
      html = HTML_MISSING_DATA_UPLOAD;
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), html);
  });
}

void WebServerHandler::handleSwagger(){
  server->on("/swagger.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = "";
    if(!utilscds->lerArquivo(LittleFS, "/swagger.json", html)) {      
      html=String(MSG_ARQUIVO_NAO_ENCONTRADO);  
    } else {
      html.replace("0.0.0",apiVersion);
      html.replace("HOST_MINION",host);
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), html);
  });
}

void WebServerHandler::handleSwaggerUI(){
  server->on("/swaggerUI", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = "";
    if(!utilscds->lerArquivo(LittleFS, "/swaggerUI.html", html)) {
      html=String(MSG_ARQUIVO_NAO_ENCONTRADO);
    } else {
      html.replace("HOST_MINION",host);  
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), html);
  });  
}

void WebServerHandler::handleHealth(){
  server->on("/health", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String JSONmessage = "{\"greeting\": \"Bem vindo ao Minion ESP32 REST Web Server\",\"date\": \""+utilscds->obtemDataHora()+"\",\"url\": \"/health\",\"version\": \""+apiVersion+"\",\"ip\": \""+utilscds->enderecoIpTexto(WiFi.localIP())+"\"}";
    request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), JSONmessage);
  });
}

void WebServerHandler::handleMetrics(){
  server->on("/metrics", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), obtemMetricas());
  });
}

void WebServerHandler::handlePorts(){
  server->on("/ports", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listSensorJson();
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void WebServerHandler::handleAudios(){
  server->on("/audios", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listMediaJson();
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void WebServerHandler::handleSensors() {
  server->on("/sensors", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) { request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing sensor"); return; }

    int sensor = pSensor->value().toInt();  // 1..4
    int pin = (sensor==1)?RelayEyes : (sensor==2)?RelayHat : (sensor==3)?RelayBlink : (sensor==4)?RelayShake : -1;
    bool on = readSensorStable(pin);
    if (auto s = searchListSensor(pin)) s->status = on;
    String resp = on ? "ativado" : "desativado";
    request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), resp);
  });  
}

void WebServerHandler::handleUpdateSensors() {
  server->on("/sensors", HTTP_PUT, [this](AsyncWebServerRequest *request) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    // Parâmetro sensor obrigatório na query
    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) { 
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing sensor"); 
      return; 
    }
    int sensor = pSensor->value().toInt();
    int pin = (sensor==1)?RelayEyes : (sensor==2)?RelayHat : (sensor==3)?RelayBlink : (sensor==4)?RelayShake : -1;
    if (pin < 0) {
      request->send(HTTP_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "sensor not found");
      return;
    }

    // Ler corpo JSON (value = 0/1)
    String body = request->arg("plain");
    if (body.length() == 0) {
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing body");
      return;
    }

    int newValue = -1;
    StaticJsonDocument<128> doc;   // payload bem pequeno: {"value":0|1}
    DeserializationError err = deserializeJson(doc, body);
    if (!err && doc["value"].is<int>()) {
      newValue = doc["value"].as<int>();
    }
    if (newValue != 0 && newValue != 1) {
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "invalid value");
      return;
    }

    // Atualiza o pino
    pinMode(pin, OUTPUT);
    digitalWrite(pin, newValue ? HIGH : LOW);
    if (auto s = searchListSensor(pin)) s->status = (newValue == 1);

    String resp = (newValue == 1) ? "ativado" : "desativado";
    request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), resp);
  });
}

void WebServerHandler::handleLists(){
  server->on("/lists", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listApplicationJson();
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleTemperatureAndHumidity(){
  server->on("/climate", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    if (!request->hasParam("type")) {
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), WRONG_CLIMATE);
      return;
    }

    String type = request->getParam("type")->value();
    if (type == "celsius") {
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), String(utilscds->obtemCelsius()));
    } else if (type == "fahrenheit") {
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), String(utilscds->obtemFahrenheit()));
    } else if (type == "humidity") {
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), String(utilscds->obtemUmidade()));
    } else {
      request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), WRONG_CLIMATE);
    }
  });
}

void WebServerHandler::handleInsertTalk(){
  server->on("/talk", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      int headers = request->headers();
      String host = "Host não encontrado";
      for(int i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);
        if(h->name() == "Host") host = h->value();
          #ifdef DEBUG
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
          #endif
      }
      String JSONmessageBody; 
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
          const char * mensagem = doc["mensagem"];
          #ifdef DEBUG
            Serial.printf("Mensagem: %s\n",mensagem);
          #endif
          String feedName="talk";
          host +="->"+String(mensagem);
          
          // publish
          //client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), host.c_str());
          doc.clear();          
          // toca o audio
          if(utilscds->tocaFala(mensagem)) {
            request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), PLAYED);
          } else {
            request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
          }          
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertAsk() {
  server->on("/ask", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!check_authorization_header(request)) {
        request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
        return;
      }

      JsonDocument doc;  // v7: alocação dinâmica
      String JSONmessageBody;
      if (deserializeJson(doc, JSONmessageBody)) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
        return;
      }

      const char * mensagem = doc["mensagem"];
      String retorno = enviarMensagemParaChatGPT(mensagem);

      if (retorno.length() == 0) {
        request->send(HTTP_BAD_REQUEST, "text/plain", "Erro ao conversar com o ChatGPT.");
        return;
      }

      // toca o áudio em background, sem quebrar a resposta
      utilscds->tocaFala(retorno.c_str());

      request->send(HTTP_OK, "text/plain", retorno);
  });
}

void WebServerHandler::handleInsertPlay(){
  server->on("/play", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      int headers = request->headers();
      String host = "Host não encontrado";
      for(int i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);
        if(h->name() == "Host") host = h->value();
          #ifdef DEBUG
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
          #endif
      }      
      JsonDocument doc;  // v7: alocação dinâmica
      String JSONmessageBody;
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        const char * midia = doc["midia"];
        #ifdef DEBUG
          Serial.printf("Arquivo: %s\n",midia);
        #endif        
        String feedName="play";
        host +="->"+String(midia);

        // publish
        //client.publish((mqttUser+String("/feeds/")+feedName).c_str(), host.c_str());
        doc.clear();
        // toca o audio
        if(utilscds->tocaMidia(midia)) {
          request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), PLAYED);
        } else {
          request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertPlayRemote(){
  server->on("/playRemote", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      JsonDocument doc;  // v7: alocação dinâmica
      String JSONmessageBody;
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        const char * url = doc["url"];        
        #ifdef DEBUG
          Serial.printf("URL: %s\n",url);
        #endif
        // toca o audio
        // exemplos:
        // 1- http://mp3.ffh.de/radioffh/hqlivestream.mp3
        // 2- http://stream.friskyradio.com:9000/frisky_mp3_h
        utilscds->tocaMidiaRemota(url);
        doc.clear();
        request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleVolume(){
  //"/volume"
  server->on("/volume", HTTP_PUT, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        String feedName="volume";
        int intensidade = doc["intensidade"];
        utilscds->atribuiVolumeAudio(intensidade);
        char buffer [MAX_PATH];              
        // publish
        //client.publish((mqttUser+String("/feeds/")+feedName).c_str(), buffer);
        snprintf ( buffer, MAX_PATH, "Intensidade do volume foi alterada para: %d", utilscds->obtemVolumeAudio());  
        doc.clear();      
        request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), buffer);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertItemList(){
  server->on("/list", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      JsonDocument doc;  // v7: alocação dinâmica
      String JSONmessageBody;
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        //busco para checar se aplicacao já existe
        int index = searchList(doc["name"],doc["language"]);
        if(index == -1) {
          String JSONmessage;
          // não existe, então posso inserir
          // adiciona item na lista de aplicações jenkins 
          addApplication(doc["name"], doc["language"], doc["description"]);  

          // Grava no Storage
          JSONmessage = saveApplicationList();
          // Grava no storage
          utilscds->escreveArquivo(LittleFS,"/lista.json",JSONmessage.c_str()); 
          #ifdef DEBUG
            Serial.println("handleInsertItemList:"+JSONmessage);
          #endif
          // Grava no adafruit
          // publish
          //client.publish((mqttUser+String("/feeds/list")).c_str(), JSONmessage.c_str());
          
          doc.clear();
          request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), JSONmessage);
        } else {
          request->send(HTTP_CONFLICT, utilscds->obtemTipoMime(".txt"), EXISTING_ITEM);
        }
      }
   } else {
    request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
   }
  });
}

void WebServerHandler::handleDeleteItemList(){
  server->on("/list/del", HTTP_DELETE, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
      //busco pela aplicacao a ser removida
      int index = searchList(doc["name"],doc["language"]);
      if(index != -1) {
        String JSONmessage;
        //removo
        removeApplication(index);
        
        // Grava no Storage
        JSONmessage = saveApplicationList();
        // Grava no storage
        utilscds->escreveArquivo(LittleFS,"/lista.json",JSONmessage.c_str()); 
        #ifdef DEBUG
          Serial.println("handleDeleteItemList:"+JSONmessage);
        #endif
        // Grava no adafruit
        // publish
        //client.publish((String(mqttUser)+String("/feeds/list")).c_str(), JSONmessage.c_str());
        doc.clear();
        request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), REMOVED_ITEM);
      } else {
        request->send(HTTP_NOT_FOUND, utilscds->obtemTipoMime(".txt"), NOT_FOUND_ITEM);
      }
    }
   } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleDeleteFile(){
  server->on("/deleteFile", HTTP_DELETE, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    //"/deleteFile?type=storage"
    //"/deleteFile?type=sdcard"
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        int paramsNr = request->params();
        const AsyncWebParameter* p = request->getParam(static_cast<size_t>(paramsNr-1));
        const char * midia = doc["midia"];
        String filename = String(midia);
        if(p->value() == "storage") {          
          if(LittleFS.remove("/"+filename))
            Serial.println("arquivo removido do storage do ESP32: "+filename);
          else
            Serial.println("Não foi possível remover o arquivo: "+filename+" do storage do ESP32!");
        } else if(p->value() == "sdcard"){
          if(SD.remove(filename))
            Serial.println("arquivo removido do sdcard: "+filename);
          else
            Serial.println("Não foi possível remover o arquivo: "+filename+" do sdcard!");
        }
        doc.clear();
        request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), REMOVED_FILE);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleListStorage() {
  server->on("/storage", HTTP_GET, [this](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    char filename[] = "/storageAndSdcard.html";
    String html;
    if (!utilscds->lerArquivo(LittleFS, filename, html)) {
      html = HTML_MISSING_DATA_UPLOAD;
    } else {
      html.replace("API_MINION_TOKEN",apiToken);
      html.replace("FILELIST",utilscds->listaArquivos());
      html.replace("MESSAGE", "Upload de arquivos para o storage interno do ESP32");
      html.replace("FREE",utilscds->obtemTamanhoLegivel((LittleFS.totalBytes() - LittleFS.usedBytes())));
      html.replace("USED",utilscds->obtemTamanhoLegivel(LittleFS.usedBytes()));
      html.replace("TOTAL",utilscds->obtemTamanhoLegivel(LittleFS.totalBytes()));
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(filename), html);
  });
}

void WebServerHandler::handleUploadStorage() {
  // run handleUpload function when any file is uploaded
  server->on("/uploadStorage", HTTP_POST,
    [this](AsyncWebServerRequest *request) {
      request->send(HTTP_OK);
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      this->handleUploadStorage(request, filename, index, data, len, final);
    });
}
 
void WebServerHandler::handleListSdcard() {
  server->on("/sdcard", HTTP_GET, [this](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    char filename[] = "/storageAndSdcard.html";
    String html;
    if(!utilscds->lerArquivo(LittleFS, filename, html)){
      html=HTML_MISSING_DATA_UPLOAD;
    } else {
      File entry =  SD.open("/", FILE_WRITE);
      html.replace("API_MINION_TOKEN",apiToken);
      html.replace("FILELIST",utilscds->listaArquivosSD(entry, 0, apiToken));
      html.replace("MESSAGE", "Mídias no cartão SD");
      html.replace("FREE",utilscds->obtemTamanhoLegivel((SD.totalBytes() - SD.usedBytes())));
      html.replace("USED",utilscds->obtemTamanhoLegivel(SD.usedBytes()));
      html.replace("TOTAL",utilscds->obtemTamanhoLegivel(SD.totalBytes()));
      entry.close();
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(filename), html);
  });
}

// handles uploads to sd card
void WebServerHandler::handleUploadSdCard() {
  // run handleUpload function when any file is uploaded
  server->on("/uploadSdcard", HTTP_POST,
    [this](AsyncWebServerRequest *request) {
      request->send(HTTP_OK);
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      this->handleUploadSdcard(request, filename, index, data, len, final);
    });
}
  
void WebServerHandler::handleInsertJigSaw(){
  server->on("/jigsaw", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      int headers = request->headers();
      String host = "Host não encontrado";
      for(int i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);
        if(h->name() == "Host") host = h->value();
          #ifdef DEBUG
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
          #endif
      }
      String JSONmessageBody;
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        const char * mensagem = doc["mensagem"];          
        request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), mensagem);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleWiFiManager(void){
  // Página principal
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request){
    Serial.println("[HTTP] GET /");
    String html = "";
    if(!utilscds->lerArquivo(LittleFS, "/wifimanager.html", html)) {
      Serial.println("handleWiFiManager");
      request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), MSG_ARQUIVO_NAO_ENCONTRADO);
    }
    else {
      request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), html);
    }
  });
}

void WebServerHandler::handleSaveCredentials(void){
  // Salvar credenciais (usa PreferencesHandler do projeto)
  server->on("/save", HTTP_POST, [this](AsyncWebServerRequest* request){
    Serial.println("[HTTP] POST /");
    String ssid = request->arg("ssid");
    String pass = request->arg("pass");
    if (ssid.isEmpty()) { request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "SSID vazio"); return; }

    utilscds->salvaCredenciaisWiFi(ssid.c_str(), pass.c_str());
  
    request->send(HTTP_OK, utilscds->obtemTipoMime(".txt"), "Credenciais salvas. Reiniciando...");
    delay(300);
    ESP.restart();
  });
}

void WebServerHandler::handleOptions(){
  server->on("/", HTTP_OPTIONS, [this](AsyncWebServerRequest *request){
    request->send(HTTP_NO_CONTENT); // No Content
  });
}

void WebServerHandler::handleOnError(){
  server->onNotFound([this](AsyncWebServerRequest *request) {
    request->send(HTTP_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "Rota não encontrada");
  });
}

AsyncWebServer * WebServerHandler::getWebServer() {
  return server;  
}
/*
void WebServerHandler::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  (void)server;  // evita -Wunused-parameter
  (void)arg;

  auto pushState = [this](){
    bool b25  = utilshdl->readSensorStable(RelayEyes);
    bool b50  = utilshdl->readSensorStable(RelayHat);
    bool b75  = utilshdl->readSensorStable(RelayBlink);
    bool b100 = utilshdl->readSensorStable(RelayShake);
  
    if (auto s = utilshdl->searchListSensor(RelayEyes))  s->status  = b25;
    if (auto s = utilshdl->searchListSensor(RelayHat))  s->status  = b50;
    if (auto s = utilshdl->searchListSensor(RelayBlink))  s->status  = b75;
    if (auto s = utilshdl->searchListSensor(RelayShake)) s->status  = b100;
  
    notifySensors("cx", b25, b50, b75, b100);
  };

  if (type == WS_EVT_CONNECT){
    pushState();                 // envia estado inicial ao conectar
  } else if (type == WS_EVT_DATA){
    Serial.println("WS_EVT_DATA -> atualizando estado");
    pushState();                 // reenvia quando chegar 'ping' do front
  }
}

void WebServerHandler::notifySensors(const String& id, bool s25, bool s50, bool s75, bool s100){
  // Normalização (garante monotonicidade)
  if (s100) { s75 = s50 = s25 = true; }
  else if (s75) { s50 = s25 = true; }
  else if (s50) { s25 = true; }

  String json;
  json.reserve(140);
  json  = "{\"id\":\"";  json += id; json += "\",\"state\":{";
  json += "\"s25\":";  json += (s25 ? "true" : "false");
  json += ",\"s50\":"; json += (s50 ? "true" : "false");
  json += ",\"s75\":"; json += (s75 ? "true" : "false");
  json += ",\"s100\":";json += (s100 ? "true" : "false");
  json += "}}";
  ws->textAll(json);
}
*/

void WebServerHandler::startWebServer() {
  // carrega sensores  
  bool load = loadSensorList();
  if(!load) {
    #ifdef DEBUG
      Serial.println(F("Nao foi possivel carregar a lista de sensores!"));
    #endif
  }
  
  // carrega lista de arquivos de media no SDCARD
  if(utilscds->carregaMidiasSdCard(apiToken)) utilscds->iniciaSdCard(); //Configura e inicia o SPI para conexão com o cartão SD
  /* 
   *  Rotas sem bloqueios de token na API
   *  Configura as páginas de login e upload 
   *  de firmware OTA 
   */
  // Rotas das imagens a serem usadas na página (não tem basic auth)
  //não tem basic auth
  handleFileServing();  
  handleHealth();
  handleHome();
  handleCiCd();
  handleSwagger();
  handleSwaggerUI();
  handleMetrics();
  //não tem basic auth
  
  /*
  * Rotas bloqueadas pelo token authorization
  */
  handlePorts();
  handleAudios();
  handleSensors();
  handleUpdateSensors();
  handleLists();
  handleTemperatureAndHumidity();
  handleInsertTalk();
  handleInsertAsk();
  handleInsertPlay();
  handleInsertPlayRemote();
  handleVolume();
  handleInsertItemList();
  handleDeleteItemList();
  handleDeleteFile();
  handleListStorage();
  handleUploadStorage();
  handleListSdcard();
  handleUploadSdCard();
  handleInsertJigSaw();
  handleOptions();
  // ------------------------------------ //
    
  // se não se enquadrar em nenhuma das rotas
  handleOnError();
  
  // permitindo todas as origens. O ideal é trocar o '*' pela url do frontend poder utilizar a api com maior segurança
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin, Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT,DELETE");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", callerOrigin);
/*
  // startup web server
  ws->onEvent([this](AsyncWebSocket* server,
                   AsyncWebSocketClient* client,
                   AwsEventType type,
                   void* arg,
                   uint8_t* data,
                   size_t len){
    this->onWsEvent(server, client, type, arg, data, len);
  });
  server->addHandler(ws);
  */
  server->begin();
}

/**********************************************
 *  Rotas do portal (AP)
 **********************************************/
void WebServerHandler::registerPortalRoutes() {
  handleFileServing();
  handleWiFiManager();
  handleSaveCredentials();
  server->begin();
}

String WebServerHandler::treatTemperatureAndHumidity(String field, String value)
{
  String JSONmessage="{\""+field+"\": \""+value+"\"}";
  Serial.println(field+": "+JSONmessage);
  return JSONmessage;
}

String WebServerHandler::enviarMensagemParaChatGPT(String mensagem) {
  String resposta = "";
  HTTPClient http;
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + utilscds->obtemTokenChatGpt());
  http.begin(chatGPTUrl);

  String payload = "{\"model\": \"gpt-3.5-turbo\",\"messages\": [{\"role\": \"user\", \"content\": \""+mensagem+"\"}],\"temperature\": 0.7,\"max_tokens\": 100, \"top_p\": 0.9,\"frequency_penalty\": 0.5,\"presence_penalty\": 0.9}";
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.printf("[HTTP] POST para o ChatGPT retornou código: %d\n", httpCode);

  // Usar HTTP_CODE_OK se existir, senão usar 200
  #ifdef ESP8266
    if (httpCode == HTTP_CODE_OK) {
  #else
    if (httpCode == 200) {  // ESP32 não define HTTP_CODE_OK
  #endif
      resposta = http.getString();
      //Serial.println("Resposta do ChatGPT: " + resposta);
      // Parse da resposta JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, resposta);
      if (error) {
        Serial.println("Erro ao analisar a resposta JSON do ChatGPT");
      } else {
        const char* mensagem = doc["choices"][0]["message"]["content"] | "";
        resposta = String(mensagem);
        Serial.println("Resposta final: " + resposta);
      }
    }
  } else {
    Serial.printf("[HTTP] Falha na conexão ao ChatGPT\n");
  }
  http.end();
  return resposta;
}

// handles uploads to storage
void WebServerHandler::handleUploadStorage(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if(this->check_authorization_header(request)) {
    if(!this->utilscds->obtemArquivosStoragePermitidos(filename)) {
      request->send(HTTP_BAD_REQUEST, "text/plain", NOT_AUTHORIZED_EXTENTIONS);
      return;
    } else {
      String logmessage = "Cliente:" + request->client()->remoteIP().toString() + "-" + request->url() + "-" + filename;      
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);
        Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
      }
      #ifdef DEBUG
        Serial.println(logmessage);
      #endif
      if (!index) {
        logmessage = "Upload Iniciado: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = LittleFS.open("/" + filename, "w");
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
        request->send(HTTP_OK, "text/plain", UPLOADED_FILE);
      }
    }      
  } else {
    request->send(HTTP_UNAUTHORIZED, "text/plain", WRONG_AUTHORIZATION);
  }
}

void WebServerHandler::handleUploadSdcard(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (this->check_authorization_header(request)) {
    if (!this->utilscds->obtemArquivosStoragePermitidos(filename)) {
      request->send(HTTP_BAD_REQUEST, "text/plain", NOT_AUTHORIZED_EXTENTIONS);
      return;
    } else {
      String logmessage = "Cliente:" + request->client()->remoteIP().toString() + "-" + request->url() + "-" + filename;      
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);        
        Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
      }
      Serial.println(logmessage);
      if (!index) {
        logmessage = "Upload Iniciado: " + String(filename);
        if (filename.endsWith(".wav") || filename.endsWith(".mp3")) {
          // open the file on first call and store the file handle in the request object
          request->_tempFile = SD.open("/" + filename, FILE_WRITE);        
        } else {
          // check if folder photos exists, if not create the folder
          if(!SD.exists("/photos")) SD.mkdir("/photos");
          // open the file on first call and store the file handle in the request object
          request->_tempFile = SD.open("/photos/" + filename, FILE_WRITE);          
        }
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
        request->send(HTTP_OK, "text/plain", SDCARD_PHOTO_WRITTEN);
      }    
    }
  } else {
    request->send(HTTP_UNAUTHORIZED, "text/plain", WRONG_AUTHORIZATION);
  }
}

/**********************************************
 *  AP + DNS cativo + portal
 **********************************************/
void WebServerHandler::startWebServerWifiManager(const String& apName) {
  
  Serial.println("==> Iniciando AP + DNS cativo + Portal");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str());         // coloque senha se quiser: softAP(ssid, pass)
  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("AP '%s' em %s\n", apName.c_str(), apIP.toString().c_str());

  dns.start(53, "*", apIP);            // captive DNS

  registerPortalRoutes();
}

/**********************************************
 *  Conexão STA (Wi-Fi do roteador)
 **********************************************/
bool WebServerHandler::connectSTA(const String& hostForMDNS) {
  (void)hostForMDNS;
 
  utilscds->carregaCredenciaisWiFi(savedSsid, savedPass);
  if (savedSsid.isEmpty()) {
    Serial.println(F("Sem credenciais salvas."));
    return false;
  }

  Serial.printf("Tentando STA: ssid='%s'\n", savedSsid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(savedSsid.c_str(), savedPass.c_str());

  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Falha na conexão STA."));
    return false;
  }

  Serial.print(F("Conectado! IP: "));
  Serial.println(WiFi.localIP());
  return true;
}

bool WebServerHandler::loadSensorList(){
  // 1 → RelayEyes | 2 → RelayHat | 3 → RelayBlink | 4 → RelayShake | 5 → TemperatureHumidity
  if(!addSensor(1, RelayEyes, "Eyes")) return false;
  if(!addSensor(2, RelayHat, "Hat")) return false;
  if(!addSensor(3, RelayBlink, "Blink")) return false;
  if(!addSensor(4, RelayShake, "Shake")) return false; 
  if(!addSensor(5, TemperatureHumidity, "TemperatureHumidity")) return false;
  return true;  
}

void WebServerHandler::addApplication(String name, String language, String description) {
  if(searchList(name, language)== -1) {
    Application *app = new Application();
    app->name = name;
    app->language = language;
    app->description = description;
    // Adiciona a aplicação na lista
    applicationListaEncadeada.add(app);
  }
}

void WebServerHandler::removeApplication(int index) {
  applicationListaEncadeada.remove(index);
}

void WebServerHandler::addMedia(String name, int size, String lastModified) {
  Media *media = new Media();
  media->name = name;
  media->size = size;
  media->lastModified=lastModified;

  // Adiciona a aplicação na lista
  mediaListaEncadeada.add(media);
}

String WebServerHandler::listApplicationJson() {
  String JSONmessage;
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"id\": "+String(i+1)+",\"name\": \""+app->name+"\",\"language\": \""+app->language+"\",\"description\": \""+app->description+"\"}"+',';
  }
  return JSONmessage;
}

String WebServerHandler::listMediaJson() {
  String JSONmessage;
  Media *media;    
  for(int i = 0; i < mediaListaEncadeada.size(); i++){
    // Obtem a midia da lista de midias
    media = mediaListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(media->name)+"\",\"size\": \""+String(media->size)+"\",\"lastModified\": \""+String(media->lastModified)+"\"},";
  }
  return JSONmessage;
}

String WebServerHandler::listSensorJson(){
  String JSONmessage;
  ArduinoSensorPort *arduinoSensorPort;    
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    arduinoSensorPort = sensorListaEncadeada.get(i);
    JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"name\": \""+arduinoSensorPort->name+"\",\"status\": \""+String(arduinoSensorPort->status)+"\"},";
  }
  return JSONmessage;
}

String WebServerHandler::saveApplicationList() {
  Application *app;
  String JSONmessage;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}"+',';
  }
  JSONmessage = '['+JSONmessage.substring(0, JSONmessage.length()-1)+']';

  return JSONmessage;
}

bool WebServerHandler::addSensor(int id, int gpio, String name) {
  ArduinoSensorPort *p = new ArduinoSensorPort();
  pinMode(gpio, INPUT_PULLUP);
  delay(10); // estabiliza após configurar o pino

  p->id = id;
  p->gpio = gpio;
  p->name = name;
  p->status = readSensorStable(gpio); // estado inicial sem “fantasma”
  sensorListaEncadeada.add(p);
  return true;
}

ArduinoSensorPort * WebServerHandler::searchListSensor(int gpio) {
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    ArduinoSensorPort *p = sensorListaEncadeada.get(i);
    if (gpio == p->gpio) return p;
  }
  return nullptr;
}

int WebServerHandler::searchList(String name, String language) {
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

// Lê várias vezes e decide por maioria: robusto contra ruído/boot
bool WebServerHandler::readSensorStable(int pin, uint8_t samples, uint16_t gap_ms) {
  uint8_t trues = 0;
  for (uint8_t i = 0; i < samples; i++) {
    int v = digitalRead(pin);
    bool on = (v == LOW);
    if (on) trues++;
    delay(gap_ms);
  }
  return (trues > samples/2);
}
