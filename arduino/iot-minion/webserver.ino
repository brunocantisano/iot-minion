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
const char HTML_MISSING_DATA_UPLOAD[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><title>Minion ESP32-Garagem Digital</title>" 
                "<meta charset=\"utf-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" 
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>"
                "<body><center><img src=\"http://www.imagenspng.com.br/wp-content/uploads/2015/07/minions-52-roxo.png\" width=\"128\"/> </center>"
                "<div class=\"container\">Lembre-se que para rodar a aplicação será necessário, previamente, instalar o plugin: "
                "<b><a src=\"https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/\">Install ESP32 Filesystem Uploader in Arduino IDE\"</a></b>"
                " e utilizar o menu no Arduino IDE: <b>Ferramentas->ESP32 Sketch Data Upload</b>"
                " para gravar o conteúdo do web server (pasta: <b>/data</b>) no <b>Storage</b>.</div></body></html>";

void handle_OnError(){
  server.onNotFound([](AsyncWebServerRequest *request) {
    if(request->method() == HTTP_OPTIONS) {
      request->send(HTTP_NO_CONTENT);
    }
      char filename[] = "/error.html";
      request->send(HTTP_NOT_FOUND, getContentType(filename), getContent(filename)); // otherwise, respond with a 404 (Not Found) error
  });
}

void handle_MinionLogo(){
  server.on("/minion-logo", HTTP_GET, [](AsyncWebServerRequest *request) {    
    char filename[] = "/minion-logo.png";
    request->send(LITTLEFS, filename, getContentType(filename));
  });
}

void handle_MinionList(){
  server.on("/minion-list", HTTP_GET, [](AsyncWebServerRequest *request) {    
    char filename[] = "/minion-list.png";
    request->send(LITTLEFS, filename, getContentType(filename));
  });
}
  
  
void handle_MinionIco(){
  server.on("/minion-ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    char filename[] = "/minion-ico.ico";
    request->send(LITTLEFS, filename, getContentType(filename));
  });
}

void handle_Style(){
  server.on("/style", HTTP_GET, [](AsyncWebServerRequest *request) {    
    char filename[] = "/style.css";
    request->send(LITTLEFS, filename, getContentType(filename));
  });
}

void handle_Home(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    char filename[] = "/home.html";
    String html = getContent(filename);
    if(html.length() > 0) {
      // versao do firmware: https://semver.org/
      html.replace("0.0.0",String(version));
      html.replace("AIO_USERNAME",String(MQTT_USERNAME));
      html.replace("HOST_MINION",String(HOST));
    } else {
      html = HTML_MISSING_DATA_UPLOAD;  
    }
    request->send(HTTP_OK, getContentType(filename), html);
  });  
}

void handle_CICD(){
  server.on("/cicd", HTTP_GET, [](AsyncWebServerRequest *request) {
    char filename[] = "/cicd.html";    
    String html = getContent(filename);
    if(html.length() > 0) {
      html.replace("AIO_SERVER",String(MQTT_BROKER));
      html.replace("AIO_USERNAME",String(MQTT_USERNAME));
      html.replace("AIO_KEY",String(MQTT_PASSWORD));
    } else {
      html = HTML_MISSING_DATA_UPLOAD;  
    }
    request->send(HTTP_OK, getContentType(filename), html);
  });
}

void handle_Swagger(){
  server.on("/swagger.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    char filename[] = "/swagger.json";
    String json = getContent(filename);
    if(json.length() > 0) {
      json.replace("0.0.0",version);
      json.replace("HOST_MINION",String(HOST)+".local");
    } else {
      json = HTML_MISSING_DATA_UPLOAD;  
    }
    request->send(HTTP_OK, getContentType(filename), json);
  });
}

void handle_SwaggerUI(){
  server.on("/swaggerUI", HTTP_GET, [](AsyncWebServerRequest *request) {
    char filename[] = "/swaggerUI.html";
    String html = getContent(filename);    
    if(html.length() > 0) {
      html.replace("HOST_MINION",String(HOST)+".local");
    } else {
      html = HTML_MISSING_DATA_UPLOAD;
    }
    request->send(HTTP_OK, getContentType(filename), html);
  });
}

void handle_Health(){
  server.on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
    String mqttConnected = client.connected()?"true":"false";
    String JSONmessage = "{\"greeting\": \"Bem vindo ao Minion ESP32 REST Web Server\",\"date\": \""+String(getDataHora())+"\",\"url\": \"/health\",\"mqtt\": \""+mqttConnected+"\",\"version\": \""+version+"\",\"ip\": \""+String(IpAddress2String(WiFi.localIP()))+"\"}";
    request->send(HTTP_OK, getContentType(".json"), JSONmessage);
  });
}

void handle_Metrics(){
  server.on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(HTTP_OK, getContentType(".txt"), getMetrics());
  });
}

void handle_Ports(){
  server.on("/ports", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage;
      ArduinoSensorPort *arduinoSensorPort;    
      for(int i = 0; i < sensorListaEncadeada.size(); i++){
        // Obtem a aplicação da lista
        arduinoSensorPort = sensorListaEncadeada.get(i);
        arduinoSensorPort->status = digitalRead(arduinoSensorPort->gpio);
        JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"status\": \""+String(arduinoSensorPort->status)+"\",\"name\": \""+String(arduinoSensorPort->name)+"\"},";
      }
      request->send(HTTP_OK, getContentType(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void handle_Audios(){
  server.on("/audios", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage;
      Media *media;    
      for(int i = 0; i < mediaListaEncadeada.size(); i++){
        // Obtem a midia da lista de midias
        media = mediaListaEncadeada.get(i);
        JSONmessage += "{\"name\": \""+String(media->name)+"\",\"size\": \""+String(media->size)+"\",\"lastModified\": \""+String(media->lastModified)+"\"},";
      }
      request->send(HTTP_OK, getContentType(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });  
}

void handle_Sensors() {
  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request) {
    //"/sensors?type=eye"
    //"/sensors?type=hat"
    //"/sensors?type=blink"
    //"/sensors?type=shake"
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
        else if (strcmp("shake", p->value().c_str())==0){
          relayPin = RelayShake;
        }        
      }
      request->send(HTTP_OK, getContentType(".json"), readSensor(relayPin));
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_Lists(){
  server.on("/lists", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String JSONmessage;
      Application *app;
      for(int i = 0; i < applicationListaEncadeada.size(); i++){
        // Obtem a aplicação da lista
        app = applicationListaEncadeada.get(i);
        JSONmessage += "{\"id\": "+String(i+1)+",\"name\": \""+app->name+"\",\"language\": \""+app->language+"\",\"description\": \""+app->description+"\"}"+',';
      }
      request->send(HTTP_OK, getContentType(".json"), '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_TemperatureAndHumidity(){
  //http://minion.local/climate?type=celsius
  server.on("/climate", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      int paramsNr = request->params();
      for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(strcmp("celsius", p->value().c_str())==0){
          request->send(HTTP_OK, getContentType(".json"), treatTemperatureAndHumidity("celsius", strCelsius));
        } else if (strcmp("fahrenheit", p->value().c_str())==0){
          request->send(HTTP_OK, getContentType(".json"), treatTemperatureAndHumidity("fahrenheit", strFahrenheit));
        }
        else if (strcmp("humidity", p->value().c_str())==0){
          request->send(HTTP_OK, getContentType(".json"), treatTemperatureAndHumidity("humidity", strHumidity));
        }
      }
      request->send(HTTP_BAD_REQUEST, getContentType(".txt"), WRONG_CLIMATE);
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
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

void handle_InsertTalk(){
  server.on("/talk", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {      
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
          const char * mensagem = doc["mensagem"];
          #ifdef DEBUG
            Serial.printf("Mensagem: %s\n",mensagem);
          #endif        
          String feedName="talk";
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), mensagem);        
          // toca o audio
          playSpeech(mensagem);
          doc.clear();
          request->send(HTTP_OK, getContentType(".txt"), PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_InsertPlay(){
  server.on("/play", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
        const char * midia = doc["midia"];
        #ifdef DEBUG
          Serial.printf("Arquivo: %s\n",midia);
        #endif        
        String feedName="play";
        // publish
        client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), midia);        
        // toca o audio
        playMidia(midia);
        doc.clear();
        request->send(HTTP_OK, getContentType(".txt"), PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_InsertPlayRemote(){
  server.on("/playRemote", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
            DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
        const char * url = doc["url"];        
        #ifdef DEBUG
          Serial.printf("URL: %s\n",url);
        #endif
        // toca o audio
        // exemplos:
        // 1- http://mp3.ffh.de/radioffh/hqlivestream.mp3
        // 2- http://stream.friskyradio.com:9000/frisky_mp3_h
        playRemoteMidia(url);
        doc.clear();
        request->send(HTTP_OK, getContentType(".txt"), PLAYED);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_Volume(){
  //"/volume"
  server.on("/volume", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {     
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
        String feedName="volume";
        setVolumeAudio(doc["intensidade"]);
        char buffer [MAX_PATH];
        snprintf ( buffer, MAX_PATH, "%d", getVolumeAudio() );   
        
        // publish
        client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), buffer);
        snprintf ( buffer, MAX_PATH, "Intensidade do volume foi alterada para: %d", getVolumeAudio());  
        doc.clear();      
        request->send(HTTP_OK, getContentType(".txt"), buffer);
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_UpdateSensors(){
  //"/sensor?type=eye"
  //"/sensor?type=hat"
  //"/sensor?type=blink"
  //"/sensor?type=shake"
  server.on("/sensor", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
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
        else if (strcmp("shake", p->value().c_str())==0){
          sensor = RelayShake;
        }
      }
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
        String JSONmessage;
        if(readBodySensorData(doc["status"], sensor)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(sensor);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), arduinoSensorPort->status==0?"OFF":"ON");
          doc.clear();
          request->send(HTTP_OK, getContentType(".json"), JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, getContentType(".txt"), WRONG_STATUS);
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
    }
  });
}

void handle_InsertItemList(){
  server.on("/list", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
        //busco para checar se aplicacao já existe
        int index = searchList(doc["name"],doc["language"]);
        if(index == -1) {
          String JSONmessage;
          // não existe, então posso inserir
          // adiciona item na lista de aplicações jenkins 
          addApplication(doc["name"], doc["language"], doc["description"]);  

          // Grava no Storage
          saveApplicationList();
        
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
          // Grava no adafruit
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds/list")).c_str(), JSONmessage.c_str());
          doc.clear();
          request->send(HTTP_OK, getContentType(".json"), JSONmessage);
        } else {
          request->send(HTTP_CONFLICT, getContentType(".txt"), EXISTING_ITEM);
        }
      }
   } else {
    request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
   }
  });
}

void handle_DeleteItemList(){
  server.on("/list/del", HTTP_DELETE, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc(MAX_STRING_LENGTH);
      String JSONmessageBody = getData(data, len);
      DeserializationError error = deserializeJson(doc, JSONmessageBody);
      if(error) {
        request->send(HTTP_BAD_REQUEST, getContentType(".json"), PARSER_ERROR);
      } else {
      //busco pela aplicacao a ser removida
      int index = searchList(doc["name"],doc["language"]);
      if(index != -1) {
        String JSONmessage;
        //removo
        applicationListaEncadeada.remove(index);
        
        // Grava no Storage
        saveApplicationList();
        
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
        client.publish((String(MQTT_USERNAME)+String("/feeds/list")).c_str(), JSONmessage.c_str());
        doc.clear();
        request->send(HTTP_OK, getContentType(".txt"), REMOVED_ITEM);
      } else {
        request->send(HTTP_NOT_FOUND, getContentType(".txt"), NOT_FOUND_ITEM);
      }
    }
   } else {
    request->send(HTTP_UNAUTHORIZED, getContentType(".txt"), WRONG_AUTHORIZATION);
   }
  });
}

void handle_UploadStorage() {
  server.on("/storage", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    char filename[] = "/uploadStorage.html";
    String html = String(getContent(filename));
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    else {
      html.replace("FILELIST",listFiles(true));
      html.replace("FREESTORAGE",humanReadableSize((LITTLEFS.totalBytes() - LITTLEFS.usedBytes())));
      html.replace("USEDSTORAGE",humanReadableSize(LITTLEFS.usedBytes()));
      html.replace("TOTALSTORAGE",humanReadableSize(LITTLEFS.totalBytes()));
    }
    request->send(HTTP_OK, getContentType(filename), html);
  });

  // run handleUpload function when any file is uploaded
  server.on("/uploadStorage", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(HTTP_OK);
      }, handleUploadStorage);
}


void handle_ListSdcard() {
  server.on("/sdcard", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    char filename[] = "/listSdcard.html";
    String html = String(getContent(filename));
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    else {
      File entry =  SD.open("/");
      html.replace("FILELIST",listFilesSD(entry, 0));
      html.replace("FREESDCARD",humanReadableSize((SD.totalBytes() - SD.usedBytes())));
      html.replace("USEDSDCARD",humanReadableSize(SD.usedBytes()));
      html.replace("TOTALSDCARD",humanReadableSize(SD.totalBytes()));
      entry.close();
    }
    request->send(HTTP_OK, getContentType(filename), html);
  });
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

void startWebServer() {
  /* Webserver para se comunicar via browser com ESP32  */
  Serial.println(WEB_SERVER_CONFIG);
  /* 
   *  Rotas sem bloqueios de token na API
   *  Configura as páginas de login e upload 
   *  de firmware OTA 
   */
  // Rotas das imagens a serem usadas na página home e o Health (não estão com basic auth)
  handle_MinionLogo();
  handle_MinionList();
  handle_MinionIco();
  handle_Style();
  handle_Health();
  handle_Metrics();

  handle_Home();
  handle_CICD();
  handle_Swagger();
  handle_SwaggerUI();
  
  /*
   * Rotas bloqueadas pelo token authorization
   */
  handle_Ports();
  handle_Volume();
  handle_Audios();
  handle_Sensors();
  handle_Lists();
  handle_TemperatureAndHumidity();
  handle_InsertTalk();
  handle_InsertPlay();
  handle_InsertPlayRemote();
  handle_UpdateSensors();
  handle_InsertItemList();
  handle_DeleteItemList();
  handle_UploadStorage();
  handle_ListSdcard();
  // ------------------------------------ //
  // se não se enquadrar em nenhuma das rotas
  handle_OnError();

  // permitindo todas as origens. O ideal é trocar o '*' pela url do frontend poder utilizar a api com maior segurança
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Access-Control-Allow-Headers, Origin, Accept, X-Requested-With, Content-Type, Access-Control-Request-Method, Access-Control-Request-Headers, Authorization");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET,HEAD,OPTIONS,POST,PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  // startup web server
  server.begin();
  
  #ifdef DEBUG
    Serial.println(WEB_SERVER_STARTED);
  #endif
}

void startWifiManagerServer() {
  Serial.println("\nConfigurando o gerenciador de Wifi ...");
  
  handle_Style();
  handle_WifiManager();
  handle_WifiInfo();
  server.serveStatic("/", LITTLEFS, "/");
  
  server.begin();
}

void handle_WifiManager(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LITTLEFS, "/wifimanager.html", getContentType("/wifimanager.html"));
  });
}

void handle_WifiInfo(){
  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST ssid value
        if (p->name() == "ssid") {
          String ssid = p->value().c_str();
          Serial.print("SSID set to: ");
          Serial.println(ssid);
          preferences.putString("ssid", ssid.c_str());
        }
        // HTTP POST pass value
        if (p->name() == "pass") {
          String pass = p->value().c_str();
          Serial.print("Password set to: ");
          Serial.println(pass);
          preferences.putString("pass", pass.c_str());
        }
        // HTTP POST ip value
        if (p->name() == "ip") {
          String ip = p->value().c_str();
          Serial.print("IP Address set to: ");
          Serial.println(ip);
          preferences.putString("ip", ip.c_str());
        }
        // HTTP POST gateway value
        if (p->name() == "gateway") {
          String gateway = p->value().c_str();
          Serial.print("Gateway set to: ");
          Serial.println(gateway);
          preferences.putString("gateway", gateway.c_str());
        }
        //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }
    request->send(200, "text/plain", "Concluido. O ESP vai reiniciar, entao conecte-se em seu roteador e va para o endereco: http://" + String(HOST) + ".local");
    delay(3000);
    ESP.restart();
  });
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
