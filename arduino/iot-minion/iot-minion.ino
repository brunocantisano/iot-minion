#include "Config.h"
#include "WebServerHandler.h"
// ====== Objetos do seu projeto ======
WiFiClient wifiClientMqtt;
ArduinoUtilsCds utilscds;
AsyncWebServer server(HTTP_REST_PORT);
WebServerHandler * websrvhdl = nullptr;
Credentials creds;
String decrypted_userFirmware;
String decrypted_passFirmware;
String decrypted_apiToken;
String decrypted_userMqtt;
String decrypted_passMqtt;
String decrypted_openIA_Key;
String decrypted_passAuthor;
bool isWiFiConnected = false;
unsigned long previousMillis;
unsigned long currentMillis;
//---------------------------------//
/**********************************************
 *  SETUP
 **********************************************/
 void setup() {
  Serial.begin(SERIAL_PORT);
  Serial.println("\nBoot...");
  
  // === Carrega credenciais de firmware/host etc. (credentials.enc) === 
  static const char* required[] = {
    "MQTT_BROKER", "MQTT_USERNAME", "MQTT_USERNAME_LENGTH", "MQTT_PASSWORD", "MQTT_PASSWORD_LENGTH", "MQTT_PORT",
    "USER_FIRMWARE", "USER_FIRMWARE_LENGTH", "PASS_FIRMWARE", "PASS_FIRMWARE_LENGTH",
    "HOST", "API_TOKEN", "API_TOKEN_LENGTH", "OPEN_IA_KEY", "OPEN_IA_KEY_LENGTH", "API_VERSION", "CALLER_ORIGIN",
    "SMTP_HOST", "SMTP_PORT", "AUTHOR_EMAIL", "AUTHOR_PASSWORD", "AUTHOR_PASSWORD_LENGTH", "RECIPIENT_NAME", "RECIPIENT_EMAIL"
  };
  const size_t requiredSize = sizeof(required) / sizeof(required[0]);
  String payload;
  if (!utilscds.iniciaStorage()) {
    Serial.println("ERRO: Falha ao inicializar o sistema de arquivos!");
    return;
  }
  // Verifica se o arquivo existe
  String credentialsPath = "/credentials.enc";
  
  if (utilscds.verificaArquivoExiste(credentialsPath)) {
    Serial.println("=== Arquivo encontrado! ===\n");
    
    // Lê e imprime o conteúdo do arquivo
    Serial.println("=== Conteúdo do arquivo credentials ===");
    Serial.println("-------------------------------------------");
    
    String content=utilscds.lerArquivo(credentialsPath);
    creds = utilscds.quebraValidaCredenciais(content, required, requiredSize, true, false);
    Serial.println("-------------------------------------------\n");

    if (creds.valid) {
      decrypted_userMqtt      = utilscds.decrypta(creds.mqttUsername, creds.mqttUsernameLength);
      decrypted_passMqtt      = utilscds.decrypta(creds.mqttPassword, creds.mqttPasswordLength);
      decrypted_userFirmware  = utilscds.decrypta(creds.userFirmware, creds.userFirmwareLength);
      decrypted_passFirmware  = utilscds.decrypta(creds.passFirmware, creds.passFirmwareLength);
      decrypted_apiToken      = utilscds.decrypta(creds.apiToken, creds.apiTokenLength);
      decrypted_openIA_Key    = utilscds.decrypta(creds.openIA_Key, creds.openIA_KeyLength);
      decrypted_passAuthor    = utilscds.decrypta(creds.authorPassword, creds.authorPasswordLength);

      Serial.println("decrypted_userMqtt: "+decrypted_userMqtt);
      Serial.println("decrypted_passMqtt: "+decrypted_passMqtt);
      Serial.println("decrypted_userFirmware: "+decrypted_userFirmware);
      Serial.println("decrypted_passFirmware: "+decrypted_passFirmware);
      Serial.println("decrypted_passAuthor: "+decrypted_passAuthor);
      Serial.println("decrypted_openIA_Key: "+decrypted_openIA_Key);
      
      const String hostName = creds.host.isEmpty() ? String("device") : creds.host;
      
      // === Servidor principal e OTA (só quando conectado) ===
      websrvhdl = new WebServerHandler(
        decrypted_apiToken.c_str(), 
        creds.apiVersion, 
        hostName,
        creds.callerOrigin,
        &utilscds
      );

      // === Wi-Fi: tenta STA; se falhar, abre portal ===
      isWiFiConnected = websrvhdl->connectSTA(hostName);
      if (!isWiFiConnected) {
        String apName = hostName.isEmpty() ? String("device-setup") : (hostName + "-setup");
        websrvhdl->startWebServerWifiManager(apName);
        Serial.println("WiFi não configurado!");
        Serial.println("Por favor, conecte-se em: " + apName + " e entre em: http://" + hostName + ".local para configuração do WiFi.");
      } else {
        char usuario[64];
        char senha[64];
        String ssid = WiFi.SSID();
        String pass = WiFi.psk();
        strncpy(usuario, ssid.c_str(), sizeof(usuario));
        strncpy(senha, pass.c_str(), sizeof(senha));
        utilscds.salvaCredenciaisWiFi(usuario, senha);

        pinMode(RelayEyes, OUTPUT);
        pinMode(RelayHat, OUTPUT);
        pinMode(RelayBlink, OUTPUT);
        pinMode(RelayShake, OUTPUT);
        pinMode(TemperatureHumidity, OUTPUT);
        
        websrvhdl->startWebServer();   // registra rotas no 'server' e chama server->begin() lá dentro
        Serial.println("Web Server inicializado");
        //utilscds.iniciaOta(&server, decrypted_userFirmware, decrypted_passFirmware);
        //Serial.println("OTA inicializado");

        const char * hostname = hostName.c_str();
        MDNS.end();
        // Atribuindo clock para conseguir usar datetime nos arquivos de log
        utilscds.atribuiRelogio();
      
        #ifdef USE_SDCARD
          // inicio sdcard
          utilscds.iniciaSdCard();
        #endif
        #ifdef USE_TEMPERATURE
          // inicio temperatura
          utilscds.iniciaTemperatura();
        #endif
        #ifdef USE_AUDIO
          // inicio audio
          utilscds.iniciaSound(decrypted_openIA_Key);
        #endif
        
        #ifdef USE_MQTT
          // inicio o mqtt
          utilscds.iniciaMqtt(&wifiClientMqtt, creds.mqttBroker, decrypted_userMqtt, decrypted_passMqtt);
        #endif
        
        if(!MDNS.begin(hostname)){
          Serial.println("mDNS falhou");
          delay(1000);
          delete websrvhdl;
          ESP.restart();
        }
        MDNS.addService("http", "tcp", 80);
        Serial.print(F("mDNS ok: http://"));
        Serial.print(hostname);     // hostname = const char* ou String
        Serial.println(F(".local"));
      }
    } else {
      Serial.println("Credenciais inválidas");
    }
  } else {
    Serial.println("Arquivo de credenciais não existe!");
  }
}

/**********************************************
 *  LOOP
 **********************************************/
void loop() {
  //unsigned long currentMillis = millis();  
  if (isWiFiConnected) {
    //MDNS.update();
    //utilscds.loopOta();    // se o seu OtaHandler exigir
    #ifdef USE_MQTT
      utilscds.atualizaMqtt();
    #endif    
    #ifdef USE_AUDIO
      //utilscds.loopAudio();  //Executa o loop interno da biblioteca audio
    #endif
    // Report every 1 minuto.
    if (currentMillis - previousMillis >= 60000) {
      previousMillis = currentMillis;
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      //utilscds.obtemDadosTemperatura();
    }
  }
}