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
  strhdl   = utilscds->obtemStorage();
  utilshdl = utilscds->obtemUtilitarios();
  prefshdl = utilscds->obtemPreferences();
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
  for(int i=0;i<headers;i++){
    const AsyncWebHeader* h = request->getHeader(i);
    if(h->name().equalsIgnoreCase("Authorization") && h->value()=="Basic "+String(apiToken)){
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
      html=String(MSG_ARQUIVO_NAO_ENCONTRADO);
    } else {
      html.replace("{{API_VERSION}}", apiVersion);
      html.replace("{{HOST_WATER_LEVEL}}", host + ".local");
      // Slug do dashboard no Adafruit IO (io.adafruit.com/<user>/dashboards/<slug>) -
      // nao tem relacao com o hostname mDNS do dispositivo.
      html.replace("{{AIO_DASHBOARD}}", "minion");

      String mqttStatus = "";
      #ifdef USE_MQTT
        String mqttUser = utilscds->obtemMqttUser();
        html.replace("{{AIO_USERNAME}}", mqttUser);
        if (utilscds->obtemMqttCredenciaisInvalidas()) {
          mqttStatus = "<strong style=\"color:#b00020\">Configuração MQTT inválida: usuário ou senha incorretos. "
                       "Corrija em <a href=\"/wifimanager.html\">/wifimanager.html</a>.</strong>";
        }
      #endif
      html.replace("{{MQTT_STATUS}}", mqttStatus);
    }
    request->send(HTTP_OK, utilshdl->getMimeType(".html"), html);
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
      html.replace("{{MQTT_BROKER}}", mqttBroker);
      html.replace("{{MQTT_USERNAME}}", mqttUser);
      html.replace("{{MQTT_PASSWORD}}", mqttPass);
    }
    request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".html"), html);
  });
}

void WebServerHandler::handleSwagger(){
  server->on("/swagger.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
    if (!LittleFS.exists("/swagger.json")) {
      request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), MSG_ARQUIVO_NAO_ENCONTRADO);
      return;
    }
    // Streaming direto do LittleFS (sem carregar o arquivo inteiro num String)
    // - o swagger.json nao tem nenhum '%' fora dos placeholders, entao o
    // template engine do ESPAsyncWebServer e seguro aqui (ver home.html
    // pra saber por que isso NAO seria seguro num arquivo com '%' legitimo).
    String apiVersionCopy = apiVersion;
    String hostname = host + ".local";
    request->send(LittleFS, "/swagger.json", "application/json", false,
      [apiVersionCopy, hostname](const String& var) -> String {
        if (var == "API_VERSION") return apiVersionCopy;
        if (var == "HOST_WATER_LEVEL") return hostname;
        return String();
      });
  });
}

void WebServerHandler::handleSwaggerUI(){
  server->on("/swaggerUI", HTTP_GET, [this](AsyncWebServerRequest *request) {
    String html = utilscds->lerArquivo("/swaggerUI.html");
    if(html.isEmpty()) {
      html=String(MSG_ARQUIVO_NAO_ENCONTRADO);
    } else {
      html.replace("{{HOST_WATER_LEVEL}}",host+".local");  
    }
    request->send(HTTP_OK, utilscds->obtemTipoMime(".html"), html);
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
      const int total = sensorListaEncadeada.size();
      String JSONmessage = "[";
      for(int i = 0; i < total; i++){
        // Obtem a aplicação da lista
        ArduinoSensorPort *arduinoSensorPort = sensorListaEncadeada.get(i);
        if (!arduinoSensorPort) continue; // defensivo: nao deveria acontecer dentro de [0,size())
        if (JSONmessage.length() > 1) JSONmessage += ",";
        JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"name\": \""+String(arduinoSensorPort->name)+"\"}";
      }
      JSONmessage += "]";
      request->send(HTTP_OK, utilshdl->getMimeType(".json"), JSONmessage);
    } else {
      // WRONG_AUTHORIZATION e PROGMEM (WebMessages.h) - send() normal faz
      // String::operator=(const char*), que chama strlen() comum (nao
      // safe pra flash) antes de copiar; send_P() evita isso (mesma causa
      // do crash que já corrigimos no HTML_FALLBACK do portal AP).
      request->send(HTTP_UNAUTHORIZED, utilshdl->getMimeType(".txt"), WRONG_AUTHORIZATION);
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
      request->send(HTTP_UNAUTHORIZED, utilshdl->getMimeType(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) { request->send(HTTP_BAD_REQUEST, utilshdl->getMimeType(".txt"), "missing sensor"); return; }

    // Valida que "sensor" e um inteiro antes de buscar na lista - sem isso um
    // valor invalido (ausente, texto) cairia num id que nao existe sem avisar
    // o cliente com uma mensagem clara.
    String sensorStr = pSensor->value();
    bool numeric = sensorStr.length() > 0;
    for (size_t i = 0; i < sensorStr.length() && numeric; i++) {
      if (!isDigit(sensorStr.charAt(i))) numeric = false;
    }
    int sensorId = numeric ? sensorStr.toInt() : -1;

    ArduinoSensorPort * s = searchListSensorById(sensorId);
    if (!s) {
      request->send(HTTP_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "Sensor nao encontrado");
      return;
    }

    bool on = digitalRead(s->gpio);
    s->status = on;

    JsonDocument doc;
    doc["sensor"] = s->id;
    doc["pin"] = String(s->gpio);
    doc["value"] = on ? 1 : 0;
    doc["ts"] = (uint32_t)time(nullptr);
    String resp;
    serializeJson(doc, resp);
    request->send(HTTP_OK, utilscds->obtemTipoMime(".json"), resp);
  });
}

void WebServerHandler::handleUpdateSensors() {
  server->on("/sensors", HTTP_PUT, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {

    if (!check_authorization_header(request)) {
      request->send(HTTP_UNAUTHORIZED, utilshdl->getMimeType(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    // Parâmetro sensor obrigatório na query
    const AsyncWebParameter* pSensor = request->getParam("sensor");
    if (!pSensor) {
      request->send(HTTP_BAD_REQUEST, utilshdl->getMimeType(".txt"), "missing sensor");
      return;
    }

    String sensorStr = pSensor->value();
    bool numeric = sensorStr.length() > 0;
    for (size_t i = 0; i < sensorStr.length() && numeric; i++) {
      if (!isDigit(sensorStr.charAt(i))) numeric = false;
    }
    int sensorId = numeric ? sensorStr.toInt() : -1;

    ArduinoSensorPort * s = searchListSensorById(sensorId);
    if (!s) {
      request->send(HTTP_NOT_FOUND, utilshdl->getMimeType(".txt"), "Sensor nao encontrado");
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
    if (err) {
      request->send(HTTP_BAD_REQUEST, utilshdl->getMimeType(".txt"), "invalid json");
      return;
    }

    if (!(doc["value"].is<int>())) {
      request->send(HTTP_BAD_REQUEST, utilshdl->getMimeType(".txt"), "missing or invalid 'value'");
      return;
    }

    int newValue = doc["value"].as<int>();
    if (newValue != 0 && newValue != 1) {
      request->send(HTTP_BAD_REQUEST, utilshdl->getMimeType(".txt"), "invalid value");
      return;
    }

    // Atualiza o pino do sensor identificado por "sensor" (query), nao mais
    // o valor 0/1 usado incorretamente como numero de pino.
    pinMode(s->gpio, OUTPUT);
    int n = newValue == 0 ? LOW : HIGH;
    digitalWrite(s->gpio, n);
    s->status = n;

    JsonDocument respDoc;
    respDoc["sensor"] = s->id;
    respDoc["pin"] = String(s->gpio);
    respDoc["value"] = newValue;
    respDoc["ts"] = (uint32_t)time(nullptr);
    String resp;
    serializeJson(respDoc, resp);
    request->send(HTTP_OK, utilshdl->getMimeType(".json"), resp);
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
        utilscds->mensagemLog("%s", message.c_str());
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
      } else if (!doc["mensagem"].is<const char*>() || String((const char*)doc["mensagem"]).isEmpty()) {
        // Sem isso, "mensagem" nulo (campo ausente/tipo errado) chega ate o
        // strlen(speech) dentro do connecttospeech() da lib de audio, que nao
        // valida null e derruba o ESP.
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Campo 'mensagem' ausente ou vazio");
      } else if (_pendingAudioAction != PendingAudioAction::None) {
        request->send(HTTP_CODE_CONFLICT, utilscds->obtemTipoMime(".txt"), "Ja existe um comando de audio em processamento");
      } else {
          const char * mensagem = doc["mensagem"];
          utilscds->adicionaNoArquivo("/talk.log", utilscds->obtemDataHoraTexto() + " - mensagem: " + String(mensagem) + "\n");
          utilscds->mensagemLog("%s", ("Mensagem: "+String(mensagem)).c_str());
          String feedName="talk";
          host +="->"+String(mensagem);
          #ifdef USE_MQTT
            // Grava no Adafruit
            utilscds->atribuiFeed(feedName, host);
          #endif

          #ifdef USE_AUDIO
            // A chamada de rede (TTS) roda no loop() principal, nao aqui -
            // ver comentario em _pendingAudioAction no header.
            _pendingAudioPayload = mensagem;
            _pendingAudioAction = PendingAudioAction::Talk;
            request->send(HTTP_CODE_ACCEPTED, utilscds->obtemTipoMime(".txt"), ACCEPTED_PROCESSING);
          #else
            request->send(HTTP_CODE_BAD_REQUEST, "text/plain", NOT_LOADED_AUDIO);
          #endif
          doc.clear();
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
      // Sem isso, "mensagem" nulo (campo ausente/tipo errado) chega ate o
      // strlen(speech) dentro do connecttospeech() da lib de audio, que nao
      // valida null e derruba o ESP.
      if (!doc["mensagem"].is<const char*>() || String((const char*)doc["mensagem"]).isEmpty()) {
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Campo 'mensagem' ausente ou vazio");
        return;
      }

      if (_pendingAudioAction != PendingAudioAction::None) {
        request->send(HTTP_CODE_CONFLICT, utilscds->obtemTipoMime(".txt"), "Ja existe um comando de audio em processamento");
        return;
      }

      #ifdef USE_AUDIO
        // A chamada ao ChatGPT (HTTPClient com timeout de 15s) e a fala da
        // resposta rodam no loop() principal, nao aqui - ver comentario em
        // _pendingAudioAction no header. A resposta e logada em /ask.log logo
        // apos ser recebida, ver PendingAudioAction::Ask no loop().
        const char * mensagem = doc["mensagem"];
        utilscds->adicionaNoArquivo("/ask.log", utilscds->obtemDataHoraTexto() + " - mensagem: " + String(mensagem) + "\n");
        _pendingAudioPayload = mensagem;
        _pendingAudioAction = PendingAudioAction::Ask;
        request->send(HTTP_CODE_ACCEPTED, utilscds->obtemTipoMime(".txt"), ACCEPTED_PROCESSING);
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
        utilscds->mensagemLog("%s", message.c_str());
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
        utilscds->mensagemLog("%s", ("Arquivo: "+String(midia)).c_str());

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
      } else if (!doc["url"].is<const char*>() || String((const char*)doc["url"]).isEmpty()) {
        // Sem isso, "url" nula (campo ausente/tipo errado) chega ate o
        // strlen(host) dentro do connecttohost() da lib de audio.
        request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Campo 'url' ausente ou vazio");
      } else if (_pendingAudioAction != PendingAudioAction::None) {
        request->send(HTTP_CODE_CONFLICT, utilscds->obtemTipoMime(".txt"), "Ja existe um comando de audio em processamento");
      } else {
        const char * url = doc["url"];
        utilscds->mensagemLog("%s", ("URL: "+String(url)).c_str());
        #ifdef USE_AUDIO
          // A conexao ao stream remoto roda no loop() principal, nao aqui -
          // ver comentario em _pendingAudioAction no header.
          // Exemplos de radios web validas: ver definicao RemoteMidia no swagger.json.
          _pendingAudioPayload = url;
          _pendingAudioAction = PendingAudioAction::PlayRemote;
          request->send(HTTP_CODE_ACCEPTED, utilscds->obtemTipoMime(".txt"), ACCEPTED_PROCESSING);
        #else
          request->send(HTTP_CODE_OK, utilscds->obtemTipoMime(".txt"), URL_PLAYED);
        #endif
        doc.clear();
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
          utilscds->mensagemLog("%s", ("handleInsertItemList:"+JSONmessage).c_str());
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
        utilscds->mensagemLog("%s", ("handleDeleteItemList:"+JSONmessage).c_str());
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

void WebServerHandler::handleSendEmail(){
  server->on("/sendEmail", HTTP_POST, [this](AsyncWebServerRequest * request){}, NULL,
    [this](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if (!check_authorization_header(request)) {
      request->send(HTTP_CODE_UNAUTHORIZED, utilscds->obtemTipoMime(".txt"), WRONG_AUTHORIZATION);
      return;
    }

    if (index == 0) request->_tempObject = new String();
    String *body = reinterpret_cast<String*>(request->_tempObject);
    body->reserve(total);
    body->concat((const char*)data, len);

    if (index + len != total) return;

    JsonDocument doc; // ArduinoJson v7
    DeserializationError err = deserializeJson(doc, *body);
    delete body;
    request->_tempObject = nullptr;
    if (err) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "JSON invalido");
      return;
    }

    String filename = doc["filename"] | "";
    filename.trim();
    // Somente arquivos .log podem ser enviados por email (mesma restricao
    // aplicada aos botoes na pagina /storage).
    if (filename.isEmpty() || !filename.endsWith(".log")) {
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Somente arquivos .log podem ser enviados por email");
      return;
    }
    if (!LittleFS.exists("/" + filename)) {
      request->send(HTTP_CODE_NOT_FOUND, utilscds->obtemTipoMime(".txt"), NOT_FOUND_ITEM);
      return;
    }
    if (_pendingEmailSend) {
      request->send(HTTP_CODE_CONFLICT, utilscds->obtemTipoMime(".txt"), "Ja existe um envio de email em processamento");
      return;
    }

    #ifdef USE_EMAIL
      // O envio de fato (SMTP) roda no loop() principal, nao aqui - ver
      // comentario em _pendingEmailSend no header (mesmo motivo do
      // _pendingAudioAction: chamada de rede bloqueante dentro do callback
      // do AsyncWebServer estoura stack/watchdog e reinicia o ESP).
      _pendingEmailFilename = filename;
      _pendingEmailSend = true;
      request->send(HTTP_CODE_ACCEPTED, utilscds->obtemTipoMime(".txt"), ACCEPTED_PROCESSING);
    #else
      request->send(HTTP_CODE_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Suporte a email nao habilitado no firmware");
    #endif
  });
}

void WebServerHandler::handleListStorage() {
  server->on("/storage", HTTP_GET, [this](AsyncWebServerRequest * request) {
    String message = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    utilscds->mensagemLog("%s", message.c_str());
    char filename[] = "/storageAndSdcard.html";
    String html = utilscds->lerArquivo(filename);
    if(html.isEmpty()) {
      html = HTML_MISSING_DATA_UPLOAD;
    } else {
      html.replace("API_MINION_TOKEN",apiToken);
      String jsonPayload = utilscds->listaArquivos();
      html.replace("MESSAGE","Storage");
      html.replace("IS_STORAGE_ROUTE","true");
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
    utilscds->mensagemLog("%s", message.c_str());
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
      html.replace("IS_STORAGE_ROUTE","false");
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
    Serial.println("[HTTP] POST /save");
    String ssid = request->arg("ssid");
    String pass = request->arg("pass");
    String userFirmware = request->arg("user_firmware");
    String passFirmware = request->arg("pass_firmware");
    String apiUser       = request->arg("api_user");
    String apiPass       = request->arg("api_pass");
    String openaiKey     = request->arg("openai_key");

    String callerOrigin  = request->arg("caller_origin");
    String mqttBroker    = request->arg("mqtt_broker");
    String mqttPort      = request->arg("mqtt_port");
    String mqttUsername  = request->arg("mqtt_username");
    String mqttPassword  = request->arg("mqtt_password");

    String smtpHost = request->arg("smtp_host");
    String smtpPort = request->arg("smtp_port");
    String smtpAuthorEmail = request->arg("smtp_author_email");
    String authorPassword = request->arg("author_password");
    String recipientEmail = request->arg("recipient_email");
    String recipientName = request->arg("recipient_name");

    // Todos os campos sao obrigatorios agora - nenhum fica "em branco mantem valor anterior".
    const struct { const char* name; const String& value; } requiredFields[] = {
      {"ssid", ssid}, {"pass", pass}, {"user_firmware", userFirmware}, {"pass_firmware", passFirmware},
      {"api_user", apiUser}, {"api_pass", apiPass}, {"openai_key", openaiKey}, {"caller_origin", callerOrigin},
      {"mqtt_broker", mqttBroker}, {"mqtt_port", mqttPort}, {"mqtt_username", mqttUsername}, {"mqtt_password", mqttPassword},
      {"smtp_host", smtpHost}, {"smtp_port", smtpPort}, {"smtp_author_email", smtpAuthorEmail}, {"author_password", authorPassword},
      {"recipient_email", recipientEmail}, {"recipient_name", recipientName}
    };
    for (const auto &f : requiredFields) {
      if (f.value.isEmpty()) {
        request->send(HTTP_BAD_REQUEST, utilscds->obtemTipoMime(".txt"), "Campo obrigatório ausente: " + String(f.name));
        return;
      }
    }

    prefshdl->saveDataPreferentials("wifi", "ssid", ssid.c_str());
    prefshdl->saveDataPreferentials("wifi", "pass", pass.c_str());

    if (!userFirmware.isEmpty()) {
      prefshdl->saveDataPreferentials("firmware", "user", utilscds->encrypta(userFirmware));
      prefshdl->saveDataPreferentials("firmware", "userLen", String(userFirmware.length()).c_str());
    }
    if (!passFirmware.isEmpty()) {
      prefshdl->saveDataPreferentials("firmware", "pass", utilscds->encrypta(passFirmware));
      prefshdl->saveDataPreferentials("firmware", "passLen", String(passFirmware.length()).c_str());
    }
    // Token de API no formato HTTP Basic: base64("usuario:senha"), depois criptografado.
    if (!apiUser.isEmpty() && !apiPass.isEmpty()) {
      String basicAuthPlain = apiUser + ":" + apiPass;
      String basicAuthB64 = base64::encode(basicAuthPlain);
      prefshdl->saveDataPreferentials("api", "token", utilscds->encrypta(basicAuthB64));
      prefshdl->saveDataPreferentials("api", "tokenLen", String(basicAuthB64.length()).c_str());
    }
	  if (!callerOrigin.isEmpty()) {
      prefshdl->saveDataPreferentials("api", "callerOrigin", callerOrigin.c_str());
    }
    if (!openaiKey.isEmpty()) {
      prefshdl->saveDataPreferentials("openai", "token", utilscds->encrypta(openaiKey));
      prefshdl->saveDataPreferentials("openai", "tokenLen", String(openaiKey.length()).c_str());
    }
        // Token de API no formato HTTP Basic: base64("usuario:senha"), depois criptografado.
    if (!apiUser.isEmpty() && !apiPass.isEmpty()) {
      String basicAuthPlain = apiUser + ":" + apiPass;
      String basicAuthB64 = base64::encode(basicAuthPlain);
      prefshdl->saveDataPreferentials("api", "token", utilscds->encrypta(basicAuthB64));
      prefshdl->saveDataPreferentials("api", "tokenLen", String(basicAuthB64.length()).c_str());
    }
	  if (!callerOrigin.isEmpty()) {
      prefshdl->saveDataPreferentials("api", "callerOrigin", callerOrigin.c_str());
    }
    if (!mqttBroker.isEmpty()) {
      prefshdl->saveDataPreferentials("mqtt", "broker", mqttBroker.c_str());
    }
    if (!mqttPort.isEmpty()) {
      prefshdl->saveDataPreferentials("mqtt", "port", mqttPort.c_str());
    }
    if (!mqttUsername.isEmpty()) {
      prefshdl->saveDataPreferentials("mqtt", "username", utilscds->encrypta(mqttUsername));
      prefshdl->saveDataPreferentials("mqtt", "usernameLen", String(mqttUsername.length()).c_str());
    }
    if (!mqttPassword.isEmpty()) {
      prefshdl->saveDataPreferentials("mqtt", "password", utilscds->encrypta(mqttPassword));
      prefshdl->saveDataPreferentials("mqtt", "passwordLen", String(mqttPassword.length()).c_str());
    }

    if (!smtpHost.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "smtp_host", smtpHost.c_str());
    }
    if (!smtpPort.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "smtp_port", smtpPort.c_str());
    }
    if (!smtpAuthorEmail.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "smtp_author_email", smtpAuthorEmail.c_str());
    }
    if (!authorPassword.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "authorPassword", utilscds->encrypta(authorPassword));
      prefshdl->saveDataPreferentials("email", "authorPasswordLen", String(authorPassword.length()).c_str());
    }
    if (!recipientEmail.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "recipientEmail", recipientEmail.c_str());
    }
    if (!recipientName.isEmpty()) {
      prefshdl->saveDataPreferentials("email", "recipientName", recipientName.c_str());
    }

    String mdnsHost = host.isEmpty() ? "minion" : host;
    String html = F(
      "<!doctype html><html lang=\"pt-BR\"><head><meta charset=\"utf-8\">"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
      "<title>Configuração salva</title><style>:root{--bg:#0f172a;--card:#1e293b;"
      "--card-border:#334155;--accent:#38bdf8;--text:#e2e8f0;--muted:#94a3b8;--ok:#22c55e}"
      "*{box-sizing:border-box}body{font-family:system-ui,-apple-system,\"Segoe UI\",Roboto,Arial,sans-serif;"
      "margin:0;min-height:100vh;display:flex;align-items:center;justify-content:center;"
      "background:radial-gradient(1200px 600px at 50% -10%,#1e293b 0,var(--bg) 60%);color:var(--text);padding:24px}"
      ".card{width:100%;max-width:420px;background:var(--card);border:1px solid var(--card-border);"
      "border-radius:16px;padding:32px 28px;box-shadow:0 20px 50px rgba(0,0,0,.45);text-align:center}"
      "h1{font-size:20px;margin:0 0 8px}p{color:var(--muted);font-size:14px;line-height:1.6;margin:0 0 20px}"
      "a.url{display:inline-block;color:#06283d;background:var(--accent);padding:12px 18px;"
      "border-radius:10px;font-weight:600;text-decoration:none;font-size:15px}"
      ".foot{display:flex;align-items:center;gap:8px;margin-top:20px;color:var(--muted);"
      "font-size:12px;justify-content:center}.dot{width:8px;height:8px;border-radius:50%;"
      "background:var(--ok);box-shadow:0 0 8px var(--ok)}</style></head><body><div class=\"card\">"
      "<h1>Configuração salva!</h1><p>O dispositivo vai reiniciar e conectar na sua rede Wi-Fi. "
      "Depois de alguns segundos, acesse o endereço abaixo pelo navegador:</p>"
      "<a class=\"url\" href=\"http://HOST.local\">http://HOST.local</a>"
      "<div class=\"foot\"><span class=\"dot\"></span><span>Reiniciando...</span></div>"
      "</div></body></html>"
    );
    html.replace("HOST", mdnsHost);
    // request->send() e assincrono - so enfileira o envio, nao garante que os
    // bytes ja chegaram no navegador. Em vez de um delay() as cegas (que
    // tanto pode reiniciar cedo demais - pagina em branco - quanto demorar
    // mais que o necessario), reinicia so quando o cliente desconectar (a
    // resposta ja sai com "Connection: close", entao isso dispara assim que
    // o navegador terminar de receber a pagina). O prazo abaixo e so uma
    // rede de seguranca caso o onDisconnect nunca chegue a disparar.
    this->_pendingRestartAfterSave = true;
    this->_pendingRestartDeadline = millis() + 5000;
    request->onDisconnect([this]() {
      if (this->_pendingRestartAfterSave) {
        this->_pendingRestartAfterSave = false;
        ESP.restart();
      }
    });
    request->send(HTTP_OK, utilshdl->getMimeType(".html"), html);
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
    if (request->method() == HTTP_OPTIONS) {
      request->send(HTTP_NO_CONTENT); // responde ao preflight com 204
      return;
    }
    request->send(HTTP_NOT_FOUND, utilscds->obtemTipoMime(".txt"), "Rota não encontrada");
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
  handleSendEmail();
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
      utilscds->mensagemLog("%s", message.c_str());
      if (!index) {
        message = "Upload Iniciado: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = LittleFS.open("/" + filename, "w");
        utilscds->mensagemLog("%s", message.c_str());
      }
    
      if (len) {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        message = "Escrevendo arquivo: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        utilscds->mensagemLog("%s", message.c_str());
      }
    
      if (final) {
        message = "Upload Completo: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        utilscds->mensagemLog("%s", message.c_str());
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
        utilscds->mensagemLog("%s", message.c_str());
      }
    
      if (len) {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        message = "Escrevendo arquivo: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        utilscds->mensagemLog("%s", message.c_str());
      }
    
      if (final) {
        message = "Upload Completo: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        utilscds->mensagemLog("%s", message.c_str());
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
  
  #ifdef DEBUG
    Serial.printf("server: %p\n", server);
  #endif

  delay(500); // aguarda estabilização da alimentação
  WiFi.mode(WIFI_AP);
  delay(100);
  /**
   * Dava o erro: Brownout detector was triggered
   * O problema é 100% elétrico. O rádio WiFi do ESP32 ao ligar puxa um pico de corrente que derruba a tensão.
   * Com USB 3.0 e cabo novo ainda acontece porque a causa mais provável: A placa Heltec WiFi Kit 32 v2.
   * Essa placa tem um regulador de tensão interno (normalmente HT7333 ou similar) que tem limitação de 
   * corrente de pico. Mesmo com fonte boa, o regulador interno não aguenta o transitório do rádio WiFi. 
   **/
  WiFi.softAP(apName.c_str());         // coloque senha se quiser: softAP(ssid, pass)
  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("AP '%s' em %s\n", apName.c_str(), apIP.toString().c_str());

  dns.start(53, "*", apIP);            // captive DNS

  registerPortalRoutes();              // <<<<< REGISTRAR ANTES do begin()
  server->begin();

  _apMode = true;
}

/**********************************************
 *  Conexão STA (Wi-Fi do roteador)
 **********************************************/
// Traduz wl_status_t em texto legível para diagnosticar falha de conexão
// (senha errada, SSID fora de alcance, etc.) — ver wl_definitions.h.
static const char* wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:     return "IDLE_STATUS (ainda tentando/sem resultado)";
    case WL_NO_SSID_AVAIL:   return "NO_SSID_AVAIL (SSID nao encontrado no ar - fora de alcance, oculto ou banda errada)";
    case WL_SCAN_COMPLETED:  return "SCAN_COMPLETED";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED (provavel senha incorreta ou modo de seguranca incompativel)";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "desconhecido";
  }
}

static const char* wifiEncTypeToString(uint8_t encType) {
  switch (encType) {
    case WIFI_AUTH_OPEN:            return "aberta (sem senha)";
    case WIFI_AUTH_WEP:             return "WEP";
    case WIFI_AUTH_WPA_PSK:         return "WPA/PSK";
    case WIFI_AUTH_WPA2_PSK:        return "WPA2/PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2 misto";
    case WIFI_AUTH_WPA3_PSK:        return "WPA3/PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:   return "WPA2/WPA3 misto";
    default:                        return "desconhecido";
  }
}

bool WebServerHandler::connectSTA(const String& hostForMDNS) {
  (void)hostForMDNS;
  String savedSsid, savedPass;

  savedSsid = utilscds->carregaDado("wifi", "ssid", savedSsid.c_str());
  savedPass = utilscds->carregaDado("wifi", "pass", savedPass.c_str());

  if (savedSsid.isEmpty()) {
    Serial.println(F("Sem credenciais salvas."));
    return false;
  }

  Serial.printf("Tentando STA: ssid='%s' (senha com %d caracteres)\n", savedSsid.c_str(), savedPass.length());

  // Diagnóstico: procura a rede salva no ar antes de tentar conectar, para
  // distinguir "SSID nao existe/fora de alcance/so 5GHz" de "senha errada".
  WiFi.mode(WIFI_STA);
  int redesEncontradas = WiFi.scanNetworks();
  bool redeEncontrada = false;
  for (int i = 0; i < redesEncontradas; i++) {
    if (WiFi.SSID(i) == savedSsid) {
      redeEncontrada = true;
      Serial.printf("  Rede encontrada no scan: RSSI=%ddBm canal=%d seguranca=%s\n",
                     WiFi.RSSI(i), WiFi.channel(i), wifiEncTypeToString(WiFi.encryptionType(i)));
    }
  }
  if (!redeEncontrada) {
    Serial.println(F("  AVISO: SSID salvo NAO apareceu no scan (fora de alcance, oculto, ou so 5GHz - ESP8266 nao suporta 5GHz)."));
  }
  WiFi.scanDelete();

  WiFi.persistent(false);
  WiFi.begin(savedSsid.c_str(), savedPass.c_str());

  for (int i = 0; i < 30 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Falha na conexao STA. status=%d (%s)\n", WiFi.status(), wifiStatusToString(WiFi.status()));
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

ArduinoSensorPort * WebServerHandler::searchListSensorById(int id) {
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    ArduinoSensorPort *p = sensorListaEncadeada.get(i);
    if (id == p->id) return p;
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
  if (_apMode) dns.processNextRequest();  // mesmo método, compatível
  if (_pendingRestartAfterSave && (long)(millis() - _pendingRestartDeadline) >= 0) {
    _pendingRestartAfterSave = false;
    ESP.restart();
  }

  if (_pendingAudioAction != PendingAudioAction::None) {
    PendingAudioAction action = _pendingAudioAction;
    String payload = _pendingAudioPayload;
    _pendingAudioAction = PendingAudioAction::None;
    _pendingAudioPayload = "";

    switch (action) {
      case PendingAudioAction::Talk:
        #ifdef USE_AUDIO
          utilscds->tocaFala(payload.c_str());
        #endif
        break;
      case PendingAudioAction::Ask: {
        String retorno = enviarMensagemParaChatGPT(payload);
        if (!retorno.isEmpty()) {
          utilscds->adicionaNoArquivo("/ask.log", utilscds->obtemDataHoraTexto() + " - mensagem: " + retorno + "\n");
        }
        #ifdef USE_AUDIO
          if (!retorno.isEmpty()) utilscds->tocaFala(retorno.c_str());
        #endif
        break;
      }
      case PendingAudioAction::PlayRemote:
        #ifdef USE_AUDIO
          utilscds->tocaMidiaRemota(payload.c_str());
        #endif
        break;
      default:
        break;
    }
  }

  if (_pendingEmailSend) {
    _pendingEmailSend = false;
    String filename = _pendingEmailFilename;
    _pendingEmailFilename = "";
    #ifdef USE_EMAIL
      String recipientEmail = utilscds->carregaDado("email", "recipientEmail", "");
      if (recipientEmail.isEmpty()) {
        utilscds->mensagemLog("[AVISO] Envio de email cancelado: destinatario nao configurado");
      } else {
        String path = "/" + filename;
        bool enviado = utilscds->enviaEmail(recipientEmail, "Log do Minion ESP32: " + filename,
                                             "Segue em anexo o arquivo de log solicitado.",
                                             true, false, path.c_str());
        utilscds->mensagemLog(enviado ? "[DEBUG] Email enviado: %s" : "[ERRO] Falha ao enviar email: %s", filename.c_str());
        // Guarda a resposta do envio (sucesso ou erro do servidor SMTP) logo
        // apos o envio, pra diagnosticar quando o email nao chega de fato.
        utilscds->adicionaNoArquivo("/email.log", utilscds->obtemDataHoraTexto() + " - " + utilscds->obtemUltimaRespostaEmail() + "\n");
      }
    #endif
  }
}

void WebServerHandler::atualizaApiToken(const String& token) {
  apiToken = token;
}