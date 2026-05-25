// WebServerHandler.cpp
#include "WebServerHandler.h"

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
  utilscds->criaNovoArquivoLog();
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
  int min_free_psram = ESP.getMinFreePsram();
  int max_alloc_psram = ESP.getMaxAllocPsram();
  const String boardName = "esp32";
  float celsius = 0;
  float fahrenheit = 0;
  float humidity = 0;
  float heat_celsius = 0;
  float heat_fahrenheit = 0;
  int volume = 0;
  uint64_t totalSdcard = 0;
  uint64_t usadosSdcard = 0;
  uint64_t chipId = ESP.getEfuseMac();
  String chipRevision = String(ESP.getChipRevision());
  String chipRevisionDesc = String(ESP.getChipModel()) + " Rev " + chipRevision;
  #ifdef USE_TEMPERATURE
    celsius = utilscds->obtemCelsius();
    fahrenheit= utilscds->obtemFahrenheit();
    humidity = utilscds->obtemUmidade();
    heat_celsius = utilscds->obtemIndiceAquecimentoCelsius();
    heat_fahrenheit = utilscds->obtemIndiceAquecimentoFahrenheit();
  #endif
  #ifdef USE_TEMPERATURE
    volume = utilscds->obtemVolumeAudio();
  #endif
  #ifdef USE_SDCARD
    totalSdcard = utilscds->obtemTotalSdcard();
    usadosSdcard = utilscds->obtemUsadosSdcard();
  #endif
  
  atribuiMetrica(&p, boardName+"_chip_id", String(chipId, HEX));
  atribuiMetrica(&p, boardName+"_sketch_md5", ESP.getSketchMD5());
  atribuiMetrica(&p, boardName+"_cycle_count", String(ESP.getCycleCount()));
  atribuiMetrica(&p, boardName+"_chip_model", ESP.getChipModel());
  atribuiMetrica(&p, boardName+"_chip_revision", chipRevision);
  atribuiMetrica(&p, boardName+"_chip_revision_desc", chipRevisionDesc);
  atribuiMetrica(&p, boardName+"_chip_cores", String(ESP.getChipCores()));
  atribuiMetrica(&p, boardName+"_cpu_freq_mhz", String(ESP.getCpuFreqMHz()));
  atribuiMetrica(&p, boardName+"_sdk_version", ESP.getSdkVersion());
  atribuiMetrica(&p, boardName+"_psram_size", String(psram_size));
  atribuiMetrica(&p, boardName+"_free_psram_size", String(free_psram_size));
  atribuiMetrica(&p, boardName+"_min_free_psram", String(min_free_psram));
  atribuiMetrica(&p, boardName+"_max_alloc_psram", String(max_alloc_psram));
  atribuiMetrica(&p, boardName+"_flash_chip_size", String(ESP.getFlashChipSize()));
  atribuiMetrica(&p, boardName+"_flash_chip_speed", String(ESP.getFlashChipSpeed()));
  atribuiMetrica(&p, boardName+"_flash_chip_mode", String(ESP.getFlashChipMode()));
  atribuiMetrica(&p, boardName+"_sketch_size", String(sketch_size));
  atribuiMetrica(&p, boardName+"_flash_size", String(flash_size));
  atribuiMetrica(&p, boardName+"_min_free_heap", String(ESP.getMinFreeHeap()));
  atribuiMetrica(&p, boardName+"_max_alloc_heap", String(ESP.getMaxAllocHeap()));
  atribuiMetrica(&p, boardName+"_uptime", String(millis()));
  atribuiMetrica(&p, boardName+"_wifi_rssi", String(WiFi.RSSI()));
  atribuiMetrica(&p, boardName+"_available_size", String(available_size));
  atribuiMetrica(&p, boardName+"_heap_size", String(heap_size));
  atribuiMetrica(&p, boardName+"_free_heap", String(free_heap));

  atribuiMetrica(&p, boardName+"_boot_counter", String(obtemContagemBoots()));
  atribuiMetrica(&p, boardName+"_celsius", String(celsius));
  atribuiMetrica(&p, boardName+"_fahrenheit", String(fahrenheit));
  atribuiMetrica(&p, boardName+"_humidity", String(humidity));
  atribuiMetrica(&p, boardName+"_heat_celsius", String(heat_celsius));
  atribuiMetrica(&p, boardName+"_heat_fahrenheit", String(heat_fahrenheit));  
  atribuiMetrica(&p, boardName+"_eyes", obtemEstadoSensor(RelayEyes));
  atribuiMetrica(&p, boardName+"_hat", obtemEstadoSensor(RelayHat));
  atribuiMetrica(&p, boardName+"_blink", obtemEstadoSensor(RelayBlink));
  atribuiMetrica(&p, boardName+"_shake", obtemEstadoSensor(RelayShake));
  atribuiMetrica(&p, boardName+"_volume", String(volume));
  atribuiMetrica(&p, boardName+"_sdcard_total", utilscds->converteUint64ParaTexto(totalSdcard));
  atribuiMetrica(&p, boardName+"_sdcard_used", utilscds->converteUint64ParaTexto(usadosSdcard));
  return p;
}

String WebServerHandler::obtemEstadoSensor(int pin) {
  return digitalRead(pin) ? "1" : "0";
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
  char buffer[12];
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
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Parametro 'name' ausente");
    }
  });
}

void WebServerHandler::handleHome(){
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {    
    String html = utilscds->lerArquivo("/home.html");
    if(html.isEmpty()) {
      html=HTML_MISSING_DATA_UPLOAD;
    } else {
      // versao do firmware: https://semver.org/
      html.replace("0.0.0",apiVersion);
      String mqttUser = "";
      #ifdef USE_MQTT
        mqttUser = utilscds->obtemMqttUser();
      #endif
      html.replace("MQTT_USERNAME",mqttUser);
      html.replace("HOST_MINION",host);
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), html);
  });
}

void WebServerHandler::handleCiCd() {
  server->on("/cicd", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = utilscds->lerArquivo("/cicd.html");
    if (html.isEmpty()) {
      html = HTML_MISSING_DATA_UPLOAD;
    } else {
      String mqttBroker = "";
      String mqttUser = "";
      String mqttPass = "";
      #ifdef USE_MQTT
        mqttBroker = utilscds->obtemMqttBroker();
        mqttUser = utilscds->obtemMqttUser();
        mqttPass = utilscds->obtemMqttPass();      
      #endif
      html.replace("MQTT_BROKER", mqttBroker);
      html.replace("MQTT_USERNAME", mqttUser);
      html.replace("MQTT_PASSWORD", mqttPass);
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), html);
  });
}

void WebServerHandler::handleSwagger(){
  server->on("/swagger.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = utilscds->lerArquivo("/swagger.json");
    if(html.isEmpty()) {
      html=HTML_MISSING_DATA_UPLOAD;  
    } else {
      html.replace("0.0.0",apiVersion);
      html.replace("HOST_MINION",host);
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), html);
  });
}

void WebServerHandler::handleSwaggerUI(){
  server->on("/swaggerUI", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = utilscds->lerArquivo("/swaggerUI.html");
    if(html.isEmpty()) {
      html=HTML_MISSING_DATA_UPLOAD;
    } else {
      html.replace("HOST_MINION",host);  
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), html);
  });  
}

void WebServerHandler::handleHealth(){
  server->on("/health", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String JSONmessage = "{\"greeting\": \"Bem vindo ao Minion ESP32 REST Web Server\",\"date\": \""+utilscds->obtemDataHora()+"\",\"url\": \"/health\",\"version\": \""+apiVersion+"\",\"ip\": \""+utilscds->enderecoIpTexto(WiFi.localIP())+"\"}";
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), JSONmessage);
  });
}

void WebServerHandler::handleMetrics(){
  server->on("/metrics", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), obtemMetricas());
  });
}

void WebServerHandler::handlePorts(){
  server->on("/ports", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listSensorJson();
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void WebServerHandler::handleAudios(){
  server->on("/audios", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listMediaJson();
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void WebServerHandler::handleSensors() {
  server->on("/sensors", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) { 
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing sensor"); 
      return; 
    }

    int sensor = pSensor->value().toInt();
    int pin = (sensor==1)?RelayEyes : (sensor==2)?RelayHat : (sensor==3)?RelayBlink : (sensor==4)?RelayShake : (sensor==5)?TemperatureHumidity : -1;
    
    // ✅ Lê da struct, não do pino físico
    int on = 0;
    if (auto s = searchListSensor(pin)) {
      on = s->status;
    }
    
    String resp = on == HIGH ? "ativado":"desativado";
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), resp);
  });  
}

void WebServerHandler::handleUpdateSensors() {
  server->on("/sensors", HTTP_PUT, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

    if (!check_authorization_header(request)) {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    // Parâmetro sensor obrigatório na query
    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) { 
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing sensor"); 
      return; 
    }

    int sensor = pSensor->value().toInt();
    int pin = (sensor==1)?RelayEyes : (sensor==2)?RelayHat : (sensor==3)?RelayBlink : (sensor==4)?RelayShake : (sensor==5)?TemperatureHumidity : -1;
    if (pin < 0) {
      request->send(HTTP_CODE_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "sensor not found");
      return;
    }

    // Monta o corpo JSON (body)
    String body;
    for (size_t i = 0; i < len; i++) {
      body += (char)data[i];
    }

    // Faz parse do JSON
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    int newValue = 0;
    if (!err && doc["value"].is<int>()) {
      newValue = doc["value"].as<int>();
    }
    if (err) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "invalid json");
      return;
    }
    
    if (!(doc["value"].is<int>())) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "missing or invalid 'value'");
      return;
    }

    newValue = doc["value"].as<int>();
    if (newValue != 0 && newValue != 1) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "invalid value");
      return;
    }
    
    // Atualiza o pino
    pinMode(pin, OUTPUT);
    int n = newValue==0?HIGH:LOW;
    digitalWrite(pin, n);
    if (auto s = searchListSensor(pin)) s->status = n;

    String resp = n == HIGH ? "ativado":"desativado";
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), resp);
  });
}

void WebServerHandler::handleLists(){
  server->on("/lists", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage = listApplicationJson();
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleTemperatureAndHumidity(){
  server->on("/climate", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    if (!request->hasParam("type")) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), WRONG_CLIMATE);
      return;
    }

    String type = request->getParam("type")->value();
    String celsius = "";
    String fahrenheit = "";
    String umidade = "";
    #ifdef USE_TEMPERATURE
      celsius = String(utilscds->obtemCelsius());
      fahrenheit = String(utilscds->obtemFahrenheit());
      umidade = String(utilscds->obtemUmidade());
    #endif
    if (type == "celsius") {
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), celsius);
    } else if (type == "fahrenheit") {
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), fahrenheit);
    } else if (type == "humidity") {
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), umidade);
    } else {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), WRONG_CLIMATE);
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
        String message = "_HEADER["+h->name()+"]: "+h->value()+"\n";
        utilscds->mensagemLog(message);
      }
      // monta corpo JSON
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
          const char * mensagem = doc["mensagem"];
          utilscds->mensagemLog("Mensagem: "+String(mensagem));
          String feedName="talk";
          host +="->"+String(mensagem);
          #ifdef USE_MQTT
            // Grava no Adafruit
            utilscds->atribuiFeed(feedName, host);
          #endif
          doc.clear();

          #ifdef USE_AUDIO
            // toca o audio
            if(utilscds->tocaFala(mensagem)) {
              request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), PLAYED);
            } else {
              request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
            }
          #else
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", NOT_LOADED_AUDIO);
          #endif
      }
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertAsk() {
  server->on("/ask", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (!check_authorization_header(request)) {
        request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
        return;
      }

      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      if (deserializeJson(doc, JSONmessageBody)) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
        return;
      }

      const char * mensagem = doc["mensagem"];
      String retorno = enviarMensagemParaChatGPT(mensagem);

      if (retorno.length() == 0) {
        request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Erro ao conversar com o ChatGPT.");
        return;
      }
      #ifdef USE_AUDIO
        // toca o áudio em background, sem quebrar a resposta
        utilscds->tocaFala(retorno.c_str());
        request->send(HTTP_CODE_OK, "text/plain", retorno);
      #else
        request->send(HTTP_CODE_BAD_REQUEST, "text/plain", NOT_LOADED_AUDIO);
      #endif
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
        String message="_HEADER["+h->name()+"]: "+h->value()+"\n";
        utilscds->mensagemLog(message);
      }      
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        const char * midia = doc["midia"];
        utilscds->mensagemLog("Arquivo: "+String(midia));

        String feedName="play";
        host +="->"+String(midia);
        #ifdef USE_MQTT
          // Grava no Adafruit
          utilscds->atribuiFeed(feedName, host);
        #endif
        doc.clear();
        #ifdef USE_AUDIO
          // toca o audio
          if(utilscds->tocaMidia(midia)) {
            request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), PLAYED);
          } else {
            request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
          }
        #else
          request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
        #endif
      }
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertPlayRemote(){
  server->on("/playRemote", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        const char * url = doc["url"];        
        utilscds->mensagemLog("URL: "+String(url));
        #ifdef USE_AUDIO
          // toca o audio
          // exemplos:
          // 1- http://mp3.ffh.de/radioffh/hqlivestream.mp3
          // 2- http://stream.friskyradio.com:9000/frisky_mp3_h
          utilscds->tocaMidiaRemota(url);
        #else
          request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), URL_PLAYED);
        #endif
        doc.clear();
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_PLAYED);
      }
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleVolume(){
  server->on("/volume", HTTP_PUT, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        String feedName="volume";
        int intensidade = doc["intensidade"];
        #ifdef USE_AUDIO
          utilscds->atribuiVolumeAudio(intensidade);
        #else
          request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), NOT_LOADED_AUDIO);
        #endif
        #ifdef USE_MQTT
          // Grava no Adafruit
          utilscds->atribuiFeed(feedName, String(intensidade));
        #endif
        char buffer [MAX_PATH];
        int volume = 0;
        #ifdef USE_AUDIO
          volume = utilscds->obtemVolumeAudio();
        #endif
        snprintf ( buffer, MAX_PATH, "Intensidade do volume foi alterada para: %d", volume);  
        doc.clear();      
        request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), buffer);
      }
    } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleInsertItemList(){
  server->on("/list", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
        //busco para checar se aplicacao já existe
        int index = searchList(doc["name"],doc["language"]);
        if(index == -1) {
          // não existe, então posso inserir
          // adiciona item na lista de aplicações jenkins 
          addApplication(doc["name"], doc["language"], doc["description"]);  
          // Grava no Storage
          String JSONmessage = saveApplicationList();
          // Grava no storage
          utilscds->escreveArquivo("/lista.json",JSONmessage.c_str()); 
          utilscds->mensagemLog("handleInsertItemList:"+JSONmessage);
          String feedName="list";
          #ifdef USE_MQTT
            // Grava no adafruit
            utilscds->atribuiFeed(feedName, JSONmessage);
          #endif
          doc.clear();
          request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".json"), JSONmessage);
        } else {
          request->send(HTTP_CODE_CONFLICT, utilscds->obtemTipoMime(".txt"), EXISTING_ITEM);
        }
      }
   } else {
    request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
   }
  });
}

void WebServerHandler::handleDeleteItemList(){
  server->on("/list", HTTP_DELETE, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      String JSONmessageBody;
      for (size_t i = 0; i < len; i++) {
        JSONmessageBody += (char)data[i];
      }
      JsonDocument doc;  // v7: alocação dinâmica
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".json"), PARSER_ERROR);
      } else {
      //busco pela aplicacao a ser removida
      int index = searchList(doc["name"],doc["language"]);
      if(index != -1) {
        //removo
        removeApplication(index);
        // Grava no Storage
        String JSONmessage = saveApplicationList();
        // Grava no storage
        utilscds->escreveArquivo("/lista.json",JSONmessage.c_str()); 
        utilscds->mensagemLog("handleDeleteItemList:"+JSONmessage);
        String feedName="list";
        #ifdef USE_MQTT
          // Grava no adafruit
          utilscds->atribuiFeed(feedName, JSONmessage);
        #endif
        doc.clear();
        request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), REMOVED_ITEM);
      } else {
        request->send(HTTP_CODE_NOT_FOUND, utilscds->obtemTipoMime(".txt"), NOT_FOUND_ITEM);
      }
    }
   } else {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void WebServerHandler::handleDeleteFile(){
  server->on("/deleteFile", HTTP_DELETE, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    // 1) Autenticação
    if (!check_authorization_header(request)) {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    // 2) Ler e validar query param "type"
    String type;
    if (request->hasParam("type", false)) { // true = search only in GET params (query)
      type = request->getParam("type", false)->value();
    } else {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Faltou o parametro 'type'");
      return;
    }

    if (!(type == "storage" || type == "sdcard")) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Parametro 'type' invalido (use 'storage' ou 'sdcard')");
      return;
    }

    // 3) Acumular o corpo JSON (pode vir em partes)
    if (index == 0) {
      // primeira parte do corpo
      request->_tempObject = new String();
    }
    String *body = reinterpret_cast<String*>(request->_tempObject);
    body->reserve(total);
    body->concat((const char*)data, len);

    // 4) Quando todo o corpo chegar, parsear e executar
    if (index + len == total) {
      // opcional: validar Content-Type
      if (request->hasHeader("Content-Type")) {
        const AsyncWebHeader* h = request->getHeader("Content-Type");
        if (h && h->value().indexOf("application/json") < 0) {
          delete body;
          request->_tempObject = nullptr;
          request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), 
                        "Content-Type deve ser application/json");
          return;
        }
      }

      // 5) Parse JSON
      JsonDocument doc; // ArduinoJson v7
      DeserializationError err = deserializeJson(doc, *body);
      if (err) {
        delete body; request->_tempObject = nullptr;
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "JSON invalido");
        return;
      }

      String filename = doc["filename"] | "";
      filename.trim();
      if (filename.isEmpty()) {
        delete body; request->_tempObject = nullptr;
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Campo 'filename' obrigatorio");
        return;
      }

      // 6) Remoção conforme 'type'
      // use caminho absoluto por consistência
      String path = "/" + filename;

      bool removed = false;
      if (type == "storage") {
        removed = LittleFS.remove(path);
        if (removed) {
          Serial.println("Arquivo removido do storage: " + path);
        } else {
          Serial.println("Falha ao remover do storage: " + path);
        }
      } else { // sdcard
        removed = SD.remove(path);
        if (removed) {
          Serial.println("Arquivo removido do sdcard: " + path);
        } else {
          Serial.println("Falha ao remover do sdcard: " + path);
        }
      }

      delete body; request->_tempObject = nullptr;

      if (removed) {
        request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), REMOVED_FILE); // "REMOVED_FILE"
      } else {
        // Swagger não define 404; use 400 para sinalizar operação inválida/não executada
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Arquivo inexistente ou nao foi possivel remover");
      }
    }
  });
}

void WebServerHandler::handleListStorage() {
  server->on("/storage", HTTP_GET, [this](AsyncWebServerRequest * request) {
    String message = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    utilscds->mensagemLog(message);
    char filename[] = "/storageAndSdcard.html";
    String html = utilscds->lerArquivo(filename);
    if(html.isEmpty()) {
      html = HTML_MISSING_DATA_UPLOAD;
    } else {
      html.replace("API_MINION_TOKEN",apiToken);
      String jsonPayload = utilscds->listaArquivos();
      html.replace("MESSAGE","Storage");
      html.replace("FILELIST",jsonPayload);
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(filename), html);
  });
}

void WebServerHandler::handleUploadStorage() {
  server->on("/uploadStorage", HTTP_POST,
    [this](AsyncWebServerRequest *request) {
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), UPLOADED_FILE);
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      static File uploadFile;

      if (!index) {
        Serial.printf("Upload start: %s\n", filename.c_str());
        uploadFile = LittleFS.open("/" + filename, "w");
      }

      if (uploadFile) {
        uploadFile.write(data, len);
      }

      if (final) {
        Serial.printf("Upload end: %s (%u bytes)\n", filename.c_str(), (unsigned int)(index + len));
        uploadFile.close();
      }
    }
  );
}
 
void WebServerHandler::handleListSdcard() {
  server->on("/sdcard", HTTP_GET, [this](AsyncWebServerRequest * request) {
    String message = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    utilscds->mensagemLog(message);
    char filename[] = "/storageAndSdcard.html";
    String html = utilscds->lerArquivo(filename);
    if(html.isEmpty()){
      html=HTML_MISSING_DATA_UPLOAD;
    } else {
      File entry =  SD.open("/", FILE_WRITE);
      html.replace("API_MINION_TOKEN",apiToken);
      String payload = "";
      #ifdef USE_SDCARD
        payload = utilscds->listaArquivosSD(entry, 0, apiToken);
      #endif
      html.replace("MESSAGE","SdCard");
      html.replace("FILELIST",payload);
      entry.close();
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(filename), html);
  });
}

// handles uploads to sd card
void WebServerHandler::handleUploadSdCard() {
  server->on("/uploadSdcard", HTTP_POST,
    [this](AsyncWebServerRequest *request) {
      // Quando o upload termina, responde ao cliente
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), UPLOADED_FILE);
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      static File uploadFile;

      // Quando index == 0, é o início do upload
      if (index == 0) {
        Serial.printf("Iniciando upload para SD: %s\n", filename.c_str());

        // Garante que o SD está montado
        if (!SD.begin()) {
          Serial.println("Erro: SD não inicializado!");
          return;
        }

        // Remove arquivo existente com mesmo nome
        if (SD.exists("/" + filename)) {
          SD.remove("/" + filename);
        }

        // Cria novo arquivo no SD
        uploadFile = SD.open("/" + filename, FILE_WRITE);
        if (!uploadFile) {
          Serial.printf("Erro ao abrir %s no SD!\n", filename.c_str());
          return;
        }
      }

      // Escreve os dados recebidos no arquivo
      if (uploadFile) {
        uploadFile.write(data, len);
      }

      // Se chegou ao final do upload, fecha o arquivo
      if (final) {
        Serial.printf("Upload completo: %s (%u bytes)\n", filename.c_str(), (unsigned int)(index + len));
        if (uploadFile) {
          uploadFile.close();
        }
      }
    }
  );
}

void WebServerHandler::handleWiFiManager(void){
  // Página principal
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request){
    Serial.println("[HTTP] GET /");
    String html = utilscds->lerArquivo("/wifimanager.html");
    if(html.isEmpty()) {
      Serial.println("handleWiFiManager");
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), MSG_ARQUIVO_NAO_ENCONTRADO);
    }
    else {
      request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), html);
    }
  });
}

void WebServerHandler::handleSaveCredentials(void){
  // Salvar credenciais (usa PreferencesHandler do projeto)
  server->on("/save", HTTP_POST, [this](AsyncWebServerRequest* request){
    Serial.println("[HTTP] POST /");
    String ssid = request->arg("ssid");
    String pass = request->arg("pass");
    if (ssid.isEmpty()) { request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "SSID vazio"); return; }

    utilscds->salvaCredenciaisWiFi(ssid.c_str(), pass.c_str());
  
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), "Credenciais salvas. Reiniciando...");
    delay(300);
    ESP.restart();
  });
}

void WebServerHandler::handleOptions(){
  server->onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){});  
  server->on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
    request->send(HTTP_CODE_NO_CONTENT);
  });
}

void WebServerHandler::handleOnError(){
  server->onNotFound([this](AsyncWebServerRequest *request) {
    request->send(HTTP_CODE_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "Rota não encontrada");
  });
}

AsyncWebServer * WebServerHandler::getWebServer() {
  return server;  
}

void WebServerHandler::startWebServer() {
  // carrega sensores  
  bool load = loadSensorList();
  if(!load) {
    utilscds->mensagemLog("Nao foi possivel carregar a lista de sensores!");
  }
  
  #ifdef USE_SDCARD
    // carrega lista de arquivos de media no SDCARD
    if(utilscds->carregaMidiasSdCard(apiToken)) utilscds->iniciaSdCard(); //Configura e inicia o SPI para conexão com o cartão SD
  #endif
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
}

String WebServerHandler::treatTemperatureAndHumidity(String field, String value)
{
  String JSONmessage="{\""+field+"\": \""+value+"\"}";
  Serial.println(field+": "+JSONmessage);
  return JSONmessage;
}

String WebServerHandler::enviarMensagemParaChatGPT(String mensagem) {
  String resposta = "";
  String token = "";
  HTTPClient http;
  
  #ifdef USE_AUDIO
    token = utilscds->obtemTokenChatGpt();
  #endif
  if(token.isEmpty()) return API_TOKEN_CHAT_GPT_ERROR;

  // 1) begin() ANTES de addHeader()
  http.begin(chatGPTUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + token);
  http.setTimeout(15000); // timeout de 15 segundos
  
  // 2) Criar payload com ArduinoJson para evitar problemas com escape
  JsonDocument docRequest;
  docRequest["model"] = "gpt-4.1";
  docRequest["temperature"] = 0.7;
  docRequest["max_tokens"] = 100;
  docRequest["top_p"] = 0.9;
  docRequest["frequency_penalty"] = 0.5;
  docRequest["presence_penalty"] = 0.9;
  
  JsonArray messages = docRequest["messages"].to<JsonArray>();
  JsonObject message = messages.add<JsonObject>();
  message["role"] = "user";
  message["content"] = mensagem;
  
  String payload;
  serializeJson(docRequest, payload);
  
  // 3) Enviar POST
  int httpCode = http.POST(payload);
  Serial.println("httpCode: "+String(httpCode));
  // 4) Verificar resposta
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST para o ChatGPT retornou código: %d\n", httpCode);
    
    #ifdef ESP8266
      if (httpCode == HTTP_CODE_OK) {
    #else
      if (httpCode == HTTP_CODE_OK) {
    #endif
      String responseBody = http.getString();
      
      // Parse da resposta JSON
      JsonDocument docResponse;
      DeserializationError error = deserializeJson(docResponse, responseBody);
      
      if (error) {
        Serial.println("Erro ao parsear JSON: ");
        Serial.println(error.c_str());
        Serial.println("Resposta recebida: " + responseBody);
      } else {
        // Verificar se a estrutura existe antes de acessar
        if (docResponse["choices"][0]["message"]["content"].is<const char*>()) {
          resposta = String(docResponse["choices"][0]["message"]["content"].as<const char*>());
          Serial.println("Resposta ChatGPT: " + resposta);
        } else {
          Serial.println("Estrutura JSON inesperada");
          Serial.println("Resposta: " + responseBody);
        }
      }
    } else {
      Serial.printf("Erro HTTP: %d\n", httpCode);
    }
  } else {
    // Erro de conexão (timeout, DNS, etc)
    Serial.printf("Erro de conexão: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  return resposta;
}

// handles uploads to storage
void WebServerHandler::handleUploadStorage(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if(this->check_authorization_header(request)) {
    if(!this->utilscds->obtemArquivosStoragePermitidos(filename)) {
      request->send(HTTP_CODE_BAD_REQUEST, "text/plain", NOT_AUTHORIZED_EXTENTIONS);
      return;
    } else {
      String message = "Cliente:" + request->client()->remoteIP().toString() + "-" + request->url() + "-" + filename;      
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);
        Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
      }
      utilscds->mensagemLog(message);
      if (!index) {
        message = "Upload Iniciado: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = LittleFS.open("/" + filename, "w");
        utilscds->mensagemLog(message);
      }
    
      if (len) {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        message = "Escrevendo arquivo: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        utilscds->mensagemLog(message);
      }
    
      if (final) {
        message = "Upload Completo: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        utilscds->mensagemLog(message);
        request->send(HTTP_CODE_OK, "text/plain", UPLOADED_FILE);
      }
    }      
  } else {
    request->send(HTTP_CODE_UNAUTHORIZED, "text/plain", WRONG_AUTHORIZATION);
  }
}

void WebServerHandler::handleUploadSdcard(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (this->check_authorization_header(request)) {
    if (!this->utilscds->obtemArquivosStoragePermitidos(filename)) {
      request->send(HTTP_CODE_BAD_REQUEST, "text/plain", NOT_AUTHORIZED_EXTENTIONS);
      return;
    } else {
      String message = "Cliente:" + request->client()->remoteIP().toString() + "-" + request->url() + "-" + filename;      
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        const AsyncWebHeader* h = request->getHeader(i);        
        Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
      }
      Serial.println(message);
      if (!index) {
        message = "Upload Iniciado: " + String(filename);
        if (filename.endsWith(".wav") || filename.endsWith(".mp3")) {
          // open the file on first call and store the file handle in the request object
          request->_tempFile = SD.open("/" + filename, FILE_WRITE);        
        } else {
          // check if folder photos exists, if not create the folder
          if(!SD.exists("/photos")) SD.mkdir("/photos");
          // open the file on first call and store the file handle in the request object
          request->_tempFile = SD.open("/photos/" + filename, FILE_WRITE);          
        }
        utilscds->mensagemLog(message);
      }
    
      if (len) {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        message = "Escrevendo arquivo: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        utilscds->mensagemLog(message);
      }
    
      if (final) {
        message = "Upload Completo: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        utilscds->mensagemLog(message);
        request->send(HTTP_CODE_OK, "text/plain", SDCARD_PHOTO_WRITTEN);
      }    
    }
  } else {
    request->send(HTTP_CODE_UNAUTHORIZED, "text/plain", WRONG_AUTHORIZATION);
  }
}

/**********************************************
 *  AP + DNS cativo + portal
 **********************************************/
void WebServerHandler::startWebServerWifiManager(const String& apName) {
  
  Serial.println("==> Iniciando AP + DNS cativo + Portal"); 
  delay(500); // aguarda estabilização da alimentação
  WiFi.mode(WIFI_AP);
  delay(100);
  WiFi.softAP(apName.c_str());         // coloque senha se quiser: softAP(ssid, pass)
  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("AP '%s' em %s\n", apName.c_str(), apIP.toString().c_str());

  dns.start(53, "*", apIP);            // captive DNS

  registerPortalRoutes();
  server->begin();
  _apMode = true;
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

  Serial.printf("Tentando STA: ssid='%s' len=%d\n",
                savedSsid.c_str(), savedSsid.length());
  Serial.flush();

  WiFi.mode(WIFI_STA);
  delay(200);

  WiFi.begin(savedSsid.c_str(), savedPass.c_str());

  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    yield();
    Serial.printf("status=%d\n", WiFi.status());
    Serial.flush();
  }

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
    JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"name\": \""+arduinoSensorPort->name+"\",\"status\": \""+String(arduinoSensorPort->status ? LOW: HIGH)+"\"},";
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
  p->status = digitalRead(gpio); // estado inicial sem “fantasma”
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

void WebServerHandler::loop() {
  if (_apMode) dns.processNextRequest();
}