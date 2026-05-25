#include <Arduino.h>
#include <ArduinoUtilsCds.h>
#include "WebServerHandler.h"
#include <ElegantOTA.h>
// ====== Objetos do seu projeto ======
WiFiClient wifiClientMqtt;
ArduinoUtilsCds* utilscds = nullptr;
AsyncWebServer server(HTTP_REST_PORT);
WebServerHandler * websrvhdl = nullptr;
bool wifi_connected = false;
unsigned long previousMillis;
unsigned long currentMillis;
//---------------------------------//
/**********************************************
 *  SETUP
 **********************************************/
 void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(100);

  // Força inicialização completa do subsistema Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin("init", "init");  // credenciais inválidas, só para inicializar
  delay(500);
  WiFi.disconnect(false);  // false = não apaga credenciais do NVS
  WiFi.mode(WIFI_OFF);
  delay(300);
  Serial.println("WiFi subsystem ok");
  Serial.flush();

  utilscds = new ArduinoUtilsCds();
  Serial.println("\nBoot...");
  
   // === 1. Leitura de TODOS os campos brutos primeiro ===
  const String raw_userFirmware       = utilscds->getCampoCredencial(USER, "USER_FIRMWARE");
  const int    len_userFirmware       = utilscds->getCampoCredencial(USER, "USER_FIRMWARE_LENGTH").toInt();

  const String raw_passFirmware       = utilscds->getCampoCredencial(USER, "PASS_FIRMWARE");
  const int    len_passFirmware       = utilscds->getCampoCredencial(USER, "PASS_FIRMWARE_LENGTH").toInt();

  const String mqttBroker             = utilscds->getCampoCredencial(USER, "MQTT_BROKER");
  const String raw_mqttUser           = utilscds->getCampoCredencial(USER, "MQTT_USERNAME");
  const int    len_mqttUser           = utilscds->getCampoCredencial(USER, "MQTT_USERNAME_LENGTH").toInt();
  const String raw_mqttPass           = utilscds->getCampoCredencial(USER, "MQTT_PASSWORD");
  const int    len_mqttPass           = utilscds->getCampoCredencial(USER, "MQTT_PASSWORD_LENGTH").toInt();
  const int    mqttPort               = utilscds->getCampoCredencial(USER, "MQTT_PORT").toInt();

  const String hostName               = utilscds->getCampoCredencial(USER, "HOST");
  const String apiVersion             = utilscds->getCampoCredencial(USER, "API_VERSION");
  const String callerOrigin           = utilscds->getCampoCredencial(USER, "CALLER_ORIGIN");
  const String raw_apiToken           = utilscds->getCampoCredencial(USER, "API_TOKEN");
  const int    len_apiToken           = utilscds->getCampoCredencial(USER, "API_TOKEN_LENGTH").toInt();

  const String raw_apiOpenAIToken     = utilscds->getCampoCredencial(USER, "OPEN_IA_KEY");
  const int    len_apiOpenAIToken     = utilscds->getCampoCredencial(USER, "OPEN_IA_KEY_LENGTH").toInt();

  const String smtpHost               = utilscds->getCampoCredencial(USER, "SMTP_HOST");
  const int    smtpPort               = utilscds->getCampoCredencial(USER, "SMTP_PORT").toInt();
  const String authorEmail            = utilscds->getCampoCredencial(USER, "AUTHOR_EMAIL");
  const String raw_emailAuthorPass    = utilscds->getCampoCredencial(USER, "AUTHOR_PASSWORD");
  const int    len_emailAuthorPass    = utilscds->getCampoCredencial(USER, "AUTHOR_PASSWORD_LENGTH").toInt();

  const String recipientName          = utilscds->getCampoCredencial(USER, "RECIPIENT_NAME");
  const String recipientEmail         = utilscds->getCampoCredencial(USER, "RECIPIENT_EMAIL");

  // === 2. Descriptografia depois, com todos os dados já em memória ===
  const String decrypted_userFirmware    = utilscds->decrypta(raw_userFirmware, len_userFirmware);
  const String decrypted_passFirmware    = utilscds->decrypta(raw_passFirmware, len_passFirmware);
  const String decrypted_mqttUser        = utilscds->decrypta(raw_mqttUser, len_mqttUser);
  const String decrypted_mqttPass        = utilscds->decrypta(raw_mqttPass, len_mqttPass);
  const String decrypted_apiToken        = utilscds->decrypta(raw_apiToken, len_apiToken);
  const String decrypted_apiOpenAIToken  = utilscds->decrypta(raw_apiOpenAIToken, len_apiOpenAIToken);
  const String decrypted_emailAuthorPass = utilscds->decrypta(raw_emailAuthorPass, len_emailAuthorPass);

  #ifdef DEBUG
    Serial.println("userFirmware: "    + decrypted_userFirmware);
    Serial.println("mqttBroker: "      + mqttBroker);
    Serial.println("mqttUser: "        + decrypted_mqttUser);
    Serial.println("mqttUserLength: "  + String(len_mqttUser));
    Serial.println("mqttPass: "        + decrypted_mqttPass);
    Serial.println("apiToken: "        + decrypted_apiToken);
    Serial.println("openAIToken: "     + decrypted_apiOpenAIToken);
  #endif
  
  // === Servidor principal e OTA (só quando conectado) ===
  websrvhdl = new WebServerHandler(
    decrypted_apiToken.c_str(), 
    apiVersion, 
    hostName,
    callerOrigin,
    utilscds
  );

  // === Wi-Fi: tenta STA; se falhar, abre portal ===
  wifi_connected = websrvhdl->connectSTA(hostName);
  if (!wifi_connected) {
    String apName = hostName.isEmpty() ? String("device-setup") : (hostName + "-setup");
    #ifdef DEBUG
      Serial.printf("Heap livre antes do AP: %d bytes\n", ESP.getFreeHeap());
    #endif
    websrvhdl->startWebServerWifiManager(apName);
    #ifdef DEBUG
      Serial.println("WiFi não configurado!");
      Serial.println("Por favor, conecte-se em: " + apName + " e entre em: http://" + hostName + ".local para configuração do WiFi.");
    #endif
  } else {
    pinMode(RelayEyes, OUTPUT);
    pinMode(RelayHat, OUTPUT);
    pinMode(RelayBlink, OUTPUT);
    pinMode(RelayShake, OUTPUT);
    pinMode(TemperatureHumidity, OUTPUT);
    
    const char * hostname = hostName.c_str();
    MDNS.end();

    // Atribuindo clock para conseguir usar datetime nos arquivos de log
    utilscds->atribuiRelogio();

    utilscds->iniciaOled();
    utilscds->exibeMensagem("Inicializando o oled");

    utilscds->iniciaStorage();
    utilscds->exibeMensagem("Inicializando o storage");

    utilscds->iniciaSdCard();
    utilscds->exibeMensagem("Inicializando o sdcard");

    utilscds->iniciaTemperatura();
    utilscds->exibeMensagem("Inicializando a temperatura");
    
    utilscds->iniciaSound(decrypted_apiOpenAIToken);
    utilscds->exibeMensagem("Inicializando o audio");
    
    utilscds->iniciaMqtt(&wifiClientMqtt, mqttBroker, decrypted_mqttUser, decrypted_mqttPass);
    utilscds->exibeMensagem("Inicializando o mqtt");

    websrvhdl->startWebServer();   // registra rotas no 'server' e chama server->begin() lá dentro

    #ifdef DEBUG
      Serial.println("Web Server inicializado");
      Serial.printf("Heap após startWebServer: %d\n", ESP.getFreeHeap());
      Serial.printf("Heap maior bloco: %d\n", ESP.getMaxAllocHeap());
    #endif
    
    ElegantOTA.begin(websrvhdl->getWebServer(), decrypted_userFirmware.c_str(), decrypted_passFirmware.c_str());
    #ifdef DEBUG
      Serial.println("OTA inicializado");
      Serial.printf("Heap após OTA: %d\n", ESP.getFreeHeap());
      Serial.printf("Heap maior bloco: %d\n", ESP.getMaxAllocHeap());
      Serial.println("host: http://"+hostName+".local");
    #endif

    if(!MDNS.begin(hostname)){
      #ifdef DEBUG
        Serial.println("mDNS falhou");
      #endif
      delay(1000);
      delete websrvhdl;
      ESP.restart();
    }
    MDNS.addService("http", "tcp", 80);
    #ifdef DEBUG
      Serial.print(F("mDNS ok: http://"));
      Serial.print(hostname);     // hostname = const char* ou String
      Serial.println(F(".local"));
    #endif
  }
}

/**********************************************
 *  LOOP
 **********************************************/
void loop() {
  currentMillis = millis();  
  if (wifi_connected) {
    utilscds->atualizaMqtt();
    utilscds->loopAudio();  //Executa o loop interno da biblioteca audio
    // Report every 1 minuto.
    if (currentMillis - previousMillis >= 60000) {
      previousMillis = currentMillis;
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      utilscds->obtemDadosTemperatura();
    }
    ElegantOTA.loop();
    websrvhdl->loop();
  }
}