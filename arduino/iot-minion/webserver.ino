#include "XT_DAC_Audio.h"

#define MAX_AUDIO_LENGTH          98216
unsigned char * audioSample;
XT_DAC_Audio_Class DacAudio(RelayAudio,0);

String HTML_MISSING_DATA_UPLOAD = "<!DOCTYPE html><html lang=\"en\"><head><title>Minion ESP32-Garagem Digital</title>" 
                "<meta charset=\"utf-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" 
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>"
                "<body><center><img src=\"https://cdn.icon-icons.com/icons2/458/PNG/128/evil-minion-icon_43747.png\" width=\"128\"/> </center>"
                "<div class=\"container\">Lembre-se que para rodar a aplicação será necessário, previamente, instalar o plugin: "
                "<b><a src=\"https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/\">ESP32 Filesystem Uploader in Arduino IDE\"</a></b>"
                " e utilizar o menu no Arduino IDE: <b>Ferramentas->ESP32 Sketch Data Upload</b>"
                " para gravar o conteúdo do web server (pasta: <b>/data</b>) no <b>SPIFFS</b>.</div></body></html>";
String PARSER_ERROR = "{\"message\": \"Erro ao fazer parser do json\"}";

void play()
{   
    //---------------------------------//  
    File root = SPIFFS.open("/audio.wav");
    if (root) {
      size_t sz = root.size();
      //size_t maxSizeFile = ESP.getFreeHeap()/2;
      #ifdef DEBUG
        Serial.printf("sz: %d\n", sz);
        Serial.println("FreeHeap: ");
        Serial.println(ESP.getFreeHeap());
      #endif    
      if(sz > MAX_AUDIO_LENGTH){
        // Erro estourou o limite do tamanho
        // de arquivo: 98k para alocação de aúdio 
        // com unsigned 8 bits PCM, 16000 Hz 
        #ifdef DEBUG
          Serial.println("Erro estourou o limite do tamanho");
        #endif
      }
      audioSample = (unsigned char *)malloc(sz);
      if (root.available()) {
        root.read(audioSample, sz);
        #ifdef DEBUG
          Serial.println("Li tudo");
        #endif
      }
      root.close();
      XT_Wav_Class Sound(audioSample);
      DacAudio.FillBuffer();
      if(Sound.Playing==false){
        // volume vai até 100
        //DacAudio.DacVolume=100;        
        DacAudio.Play(&Sound);
      }
      free(audioSample);
      #ifdef DEBUG
        Serial.println("Retornando OK da função: playAudio");      
        fileRemove("/audio.wav");
        Serial.println("Arquivo /audio.wav removido!");
      #endif
      #ifdef DEBUG
        Serial.printf("Tocou o arquivo com sucesso!");
      #endif
    } else {
      #ifdef DEBUG
        Serial.printf("Erro ao abrir audio.wav!");
      #endif
    }
}

void startWebServer() {
  /* Webserver para se comunicar via browser com ESP32  */
  Serial.println("\nConfiguring Webserver ...");

  server = new AsyncWebServer(HTTP_REST_PORT);

  /* 
   *  Rotas sem bloqueios de token na API
   *  Configura as páginas de login e upload 
   *  de firmware OTA 
   */
  // Rotas das imagens a serem usadas na página home e o Health (não estão com basic auth)
  handle_Audio();
  handle_MinionLogo();
  handle_UploadLogo();
  handle_GaragemLogo();
  handle_MinionIco();
  handle_MinionDevopsLogo();
  handle_MinionDevopsListLogo();
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
  handle_Audios();
  handle_Eyes();
  handle_Hats();
  handle_Blinks();
  handle_Shakes();
  handle_Lists();
  handle_Celsius();
  handle_Fahrenheit();
  handle_Humidity();
  handle_InsertTalk();
  handle_UpdateEye();
  handle_InsertPlay();
  handle_UpdateHat();
  handle_UpdateBlink();
  handle_UpdateShake();
  handle_InsertItemList();
  handle_DeleteItemList();
  handle_Certificate();
  // ------------------------------------ //
  // se não se enquadrar em nenhuma das rotas
  handle_OnError();
 
  // startup web server
  server->begin();

  #ifdef DEBUG
    Serial.println("Webserver started");
  #endif
}

void handle_Audio(){
  server->on("/audio", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(SPIFFS, "/audio.wav", "audio/wav");
  });
}

void handle_MinionLogo(){
  server->on("/minion-logo", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(SPIFFS, "/minion-logo.jpg", "image/jpg");
  });
}

void handle_UploadLogo(){
  server->on("/upload-logo", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(SPIFFS, "/upload-logo.png", "image/png");
  });
}

void handle_GaragemLogo(){
  server->on("/garagem-logo", HTTP_GET, [](AsyncWebServerRequest *request) {        
    request->send(SPIFFS, "/garagem-logo.jpg", "image/jpg");
  });
}

void handle_MinionIco(){
  server->on("/minion-ico", HTTP_GET, [](AsyncWebServerRequest *request) {    
    request->send(SPIFFS, "/minion-ico.ico", "image/ico");
  });
}

void handle_MinionDevopsLogo(){
  server->on("/minion-devops-logo", HTTP_GET, [](AsyncWebServerRequest *request) {        
    request->send(SPIFFS, "/minion-devops-logo.png", "image/png");
  });
}

void handle_MinionDevopsListLogo(){
  server->on("/minion-devops-list-logo", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/minion-devops-list-logo.png", "image/png");  
  });
}

void handle_Style(){
  server->on("/style", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");  
  });
}

void handle_Home(){
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = HTML_MISSING_DATA_UPLOAD;
    if(SPIFFS.exists("/home.html")){
      html = getContent("/home.html");
      // versao do firmware: https://semver.org/
      html.replace("0.0.0",String(version));
      html.replace("AIO_USERNAME",String(MQTT_USERNAME));
      html.replace("HOST_MINION",String(HOST));      
    }    
    request->send(HTTP_OK, "text/html", html);
  });
}

void handle_Certificate() {
  server->on("/certificado", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    #ifdef DEBUG
      Serial.println(logmessage);
    #endif
    String html = getContent("/upload.html");
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    else {
      html.replace("FILELIST",listFiles(true));
      html.replace("FREESPIFFS",humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
      html.replace("USEDSPIFFS",humanReadableSize(SPIFFS.usedBytes()));
      html.replace("TOTALSPIFFS",humanReadableSize(SPIFFS.totalBytes()));
    }
    request->send(HTTP_OK, "text/html", html);
  });

  // run handleUpload function when any file is uploaded
  server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(HTTP_OK);
      }, handleUpload);
}

void handle_CICD(){
  server->on("/cicd", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = getContent("/cicd.html");
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    html.replace("AIO_SERVER",String(MQTT_BROKER));
    html.replace("AIO_USERNAME",String(MQTT_USERNAME));
    html.replace("AIO_KEY",String(MQTT_PASSWORD));
    request->send(HTTP_OK, "text/html", html);
  });
}

void handle_Swagger(){
  server->on("/swagger.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = getContent("/swagger.json");
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    html.replace("0.0.0",version);
    html.replace("HOST_MINION",String(HOST)+".local");
    request->send(HTTP_OK, "application/json", html);
  });
}

void handle_SwaggerUI(){
  server->on("/swaggerUI", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = getContent("/swaggerUI.html");
    if(html.length() == 0) html=HTML_MISSING_DATA_UPLOAD;
    html.replace("HOST_MINION",String(HOST)+".local");
    request->send(HTTP_OK, "text/html", html);
  });  
}

void handle_Health(){
  server->on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
    String mqttConnected = client.connected()?"true":"false";
    String JSONmessage = "{\"greeting\": \"Bem vindo ao Minion ESP32 REST Web Server\",\"date\": \""+getDataHora()+"\",\"url\": \"/health\",\"mqtt\": \""+mqttConnected+"\",\"version\": \""+version+"\",\"ip\": \""+IpAddress2String(WiFi.localIP())+"\"}";
    request->send(HTTP_OK, "application/json", JSONmessage);
  });
}

void handle_Metrics(){
  server->on("/metrics", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(HTTP_OK, "text/plain", getMetrics());
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
      request->send(HTTP_OK, "application/json", '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });  
}

void handle_Audios(){
  server->on("/audios", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      String dados="[]";
      char elements[2048]="";
      if(Firebase.ready()) {
        #ifdef DEBUG
          Serial.println("FreeHeap: ");
          Serial.println(ESP.getFreeHeap());
        #endif
        if(Firebase.Storage.listFiles(&fbdo, FIREBASE_STORAGE_BUCKET_ID)) {
          if (fbdo.httpCode() != FIREBASE_ERROR_HTTP_CODE_OK) {
            request->send(HTTP_BAD_REQUEST, "plain/text", "Não conseguiu listar bucket do firebase");         
          }
          else {
            FileList *files = fbdo.fileList();
            for (size_t i = 0; i < files->items.size(); i++){
              strcat(elements, "\"");
              strcat(elements, files->items[i].name.c_str());
              strcat(elements, "\",");
            }
            if(strlen(elements) > 0) {
              dados="["+String(substr(elements,0, strlen(elements)-1))+"]";
            }
            request->send(HTTP_OK, "application/json", dados); 
          }
        } else {
          request->send(HTTP_BAD_REQUEST, "plain/text", "Erro ao executar a função: Firebase.Storage.listFile");
        }
      } else {
        request->send(HTTP_BAD_REQUEST, "plain/text", "Firebase não estava pronto");
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });  
}

void handle_Gets(char route[]) {
  server->on(route, HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      request->send(HTTP_OK, "application/json", readSensor(RelayEyes));
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_Eyes(){
  handle_Gets("/eyes");
}

void handle_Hats(){
  handle_Gets("/hats");
}

void handle_Blinks(){
  handle_Gets("/blinks");
}

void handle_Shakes(){
  handle_Gets("/shakes");
}

void handle_Lists(){
  server->on("/lists", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      getContent("/lista.json");
      String JSONmessage;
      Application *app;
      for(int i = 0; i < applicationListaEncadeada.size(); i++){
        // Obtem a aplicação da lista
        app = applicationListaEncadeada.get(i);
        JSONmessage += "{\"id\": "+String(i+1)+",\"name\": \""+app->name+"\",\"language\": \""+app->language+"\",\"description\": \""+app->description+"\"}"+',';
      }
      request->send(HTTP_OK, "application/json", '['+JSONmessage.substring(0, JSONmessage.length()-1)+']');
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_Celsius(){
  server->on("/celsius", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      request->send(HTTP_OK, "application/json", treatTemperatureAndHumidity("celsius", celsius));
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_Fahrenheit(){
  server->on("/fahrenheit", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      request->send(HTTP_OK, "application/json", treatTemperatureAndHumidity("fahrenheit", fahrenheit));
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_Humidity(){
  server->on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(check_authorization_header(request)) {
      request->send(HTTP_OK, "application/json", treatTemperatureAndHumidity("humidity", humidity));
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

String treatTemperatureAndHumidity(String field, float value)
{
  String JSONmessage="{\""+field+"\": \""+String(value,1)+"\"}";
  #ifdef DEBUG
    Serial.println(field+": "+JSONmessage);
  #endif
  char buffer [sizeof(int)*8+1];
  itoa (value,buffer,10);
  return JSONmessage;
}

void handle_InsertTalk(){
  server->on("/talk", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {      
      DynamicJsonDocument docText2Speech = treatDataString(getJsonText2Speech());
      if(docText2Speech.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        String text2SpeechHost = docText2Speech["text2SpeechHost"];
        DynamicJsonDocument doc = treatData(data, len);
        if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
        else {
          String mensagem = doc["mensagem"];
          String languageCode = docText2Speech["languageCode"];
          String voiceName = docText2Speech["voiceName"];
          String ssmlGender = docText2Speech["ssmlGender"];
          String audioEncoding = docText2Speech["audioEncoding"];
          String urlText2Speech="https://"+text2SpeechHost+"/v1beta1/text:synthesize?key="+String(API_TEXT2SPEECH_KEY);
          String body="{\"input\": {\"text\": \""+mensagem+"\"},\"voice\": {\"languageCode\": \""+languageCode+"\",\"name\": \""+voiceName+"\",\"ssmlGender\": \""+ssmlGender+"\"},\"audioConfig\": {\"audioEncoding\": \""+audioEncoding+"\", \"speakingRate\": 1.8, \"volumeGainDb\": 15, \"pitch\": 7, \"sampleRateHertz\": 16000}}";
          #ifdef DEBUG
            Serial.println("body: "+body);
          #endif
          // lendo do SPIFFS o certificado google para usar a api text2speech
          String payload = postUtil(urlText2Speech, body, getContent("/text2speech.crt"));          
          String buscaCampo="\"audioContent\": \"";
          int buscaCampoLength=buscaCampo.length();
          int inicioCampo = payload.indexOf(buscaCampo);
          int fimCampo = payload.indexOf("\"", inicioCampo + buscaCampoLength+1);
          #ifdef DEBUG
            Serial.printf("inicio:%d fim:%d\n",inicioCampo+buscaCampoLength,fimCampo);            
          #endif
          payload = payload.substring(inicioCampo+buscaCampoLength, fimCampo);
          if(!fileWriteDecodingBase64("/audio.wav",payload)) {
            Serial.println("resultado: erro na geração do audio.wav");
          } else {
            Serial.println("resultado: arquivo audio.wav gerado com sucesso");
          }
          // toca o audio
          //play();
          request->send(HTTP_OK, "plain/text", "Arquivo foi colocado para tocar.");
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_InsertPlay(){
  server->on("/play", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        const char * midia = doc["midia"];        
        #ifdef DEBUG
          Serial.printf("Arquivo: %s\n",midia);
        #endif

        // Thread para o download de arquivo do firebase
        xTaskCreate(taskDownloadFirebase, "taskDownloadFirebase", 10000, (void *) midia, 1, NULL);
        
        // toca o audio
        play();

        request->send(HTTP_OK, "plain/text", "Arquivo do firebase foi colocado para tocar.");
      }   
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_UpdateEye(){
  server->on("/eye", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        String JSONmessage;
        if(readBodySensorData(doc, RelayEyes)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(RelayEyes);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds")+String(request->url())).c_str(), arduinoSensorPort->status==0?"OFF":"ON");
          request->send(HTTP_OK, "application/json", JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, "plain/text", "Erro ao atualizar o status");
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_UpdateHat(){
  server->on("/hat", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        String JSONmessage;
        if(readBodySensorData(doc, RelayHat)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(RelayHat);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds")+String(request->url())).c_str(), arduinoSensorPort->status==0?"OFF":"ON");
          request->send(HTTP_OK, "application/json", JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, "plain/text", "Erro ao atualizar o status");
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_UpdateBlink(){
  server->on("/blink", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        String JSONmessage;
        if(readBodySensorData(doc, RelayBlink)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(RelayBlink);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds")+String(request->url())).c_str(), arduinoSensorPort->status==0?"OFF":"ON");
          request->send(HTTP_OK, "application/json", JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, "plain/text", "Erro ao atualizar o status");
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_UpdateShake(){
  server->on("/shake", HTTP_PUT, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        String JSONmessage;
        if(readBodySensorData(doc, RelayShake)) {
          ArduinoSensorPort *arduinoSensorPort = searchListSensor(RelayShake);
          if(arduinoSensorPort != NULL) {
            JSONmessage="{\"id\":\""+String(arduinoSensorPort->id)+"\",\"name\":\""+String(arduinoSensorPort->name)+"\",\"gpio\":\""+String(arduinoSensorPort->gpio)+"\",\"status\":\""+String(arduinoSensorPort->status)+"\"}";
          }
          digitalWrite(arduinoSensorPort->gpio, arduinoSensorPort->status);
          // publish
          client.publish((String(MQTT_USERNAME)+String("/feeds")+String(request->url())).c_str(), arduinoSensorPort->status==0?"OFF":"ON");
          request->send(HTTP_OK, "application/json", JSONmessage);
        } else {
          request->send(HTTP_BAD_REQUEST, "plain/text", "Erro ao atualizar o status");
        }
      }
    } else {
      request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
    }
  });
}

void handle_InsertItemList(){
  server->on("/list", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        //busco para checar se aplicacao já existe
        int index = searchList(doc);
        if(index == -1) {
          String JSONmessage;
          // não existe, então posso inserir
          // adiciona item na lista de aplicações jenkins 
          addApplication(doc["name"], doc["language"], doc["description"]);  

          // Grava no SPIFFS
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
          request->send(HTTP_OK, "application/json", JSONmessage);
        } else {
          request->send(HTTP_CONFLICT, "text/plain", "Item já existente na lista");
        }    
      }
   } else {
    request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
   }
  });
}

void handle_DeleteItemList(){
  server->on("/list/del", HTTP_DELETE, [](AsyncWebServerRequest * request){}, NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
    if(check_authorization_header(request)) {
      DynamicJsonDocument doc = treatData(data, len);
      if(doc.isNull()) request->send(HTTP_BAD_REQUEST, "application/json", PARSER_ERROR);
      else {
        //busco pela aplicacao a ser removida
        int index = searchList(doc);
        if(index != -1) {
          String JSONmessage;
          //removo
          applicationListaEncadeada.remove(index);
          
          // Grava no SPIFFS
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
          request->send(HTTP_OK, "text/plain", "Item removido da lista");
        } else {
          request->send(HTTP_NOT_FOUND, "text/plain", "Item não encontrado na lista");
        }
      }
   } else {
    request->send(HTTP_UNAUTHORIZED, "plain/text", "Authorization token errado");
   }
  });
}

void handle_OnError(){
  server->onNotFound([](AsyncWebServerRequest *request) {
    request->send(HTTP_NOT_FOUND, "text/plain", "Rota nao encontrada");
  });
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

void taskDownloadFirebase( void * parameter )
{
  #ifdef DEBUG
    Serial.print("download running on core ");
    Serial.println(xPortGetCoreID());
  #endif
    
  char *pcTaskName;
  pcTaskName = (char *) parameter;
  for( ;; ){
      Serial.printf("Task download: %s\n", pcTaskName);      
      if (Firebase.ready())
      {
    #if defined(ESP32)
          Firebase.sdBegin(13, 14, 2, 15); //SS, SCK,MISO, MOSI
    #elif defined(ESP8266)
          Firebase.sdBegin(15); //SS
    #endif
          //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
          Serial.printf("Download file... %s\n", 
          Firebase.Storage.download(&fbdo, FIREBASE_STORAGE_BUCKET_ID,
          pcTaskName,
          "/audio.wav",
          mem_storage_type_flash) ? "ok" : fbdo.errorReason().c_str());
          break;
      } else {
        break;
      }
  }
  Serial.println("Ending task download");
  vTaskDelete( NULL );
}
