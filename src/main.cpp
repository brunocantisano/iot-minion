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
  utilscds = new ArduinoUtilsCds();
  Serial.println("\nBoot...");
  
  // === Carrega credenciais de firmware/host etc. (credentials.enc) === 
  const String decrypted_userFirmware  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "USER_FIRMWARE"),utilscds->getCampoCredencial(USER, "USER_FIRMWARE_LENGTH").toInt());
  const String decrypted_passFirmware  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "PASS_FIRMWARE"),utilscds->getCampoCredencial(USER, "PASS_FIRMWARE_LENGTH").toInt());
  
  const String mqttBroker = utilscds->getCampoCredencial(USER, "MQTT_BROKER");
  const String decrypted_mqttUser  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "MQTT_USERNAME"),utilscds->getCampoCredencial(USER, "MQTT_USERNAME_LENGTH").toInt());
  const String decrypted_mqttPass  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "MQTT_PASSWORD"),utilscds->getCampoCredencial(USER, "MQTT_PASSWORD_LENGTH").toInt());
  const int mqttPort = utilscds->getCampoCredencial(USER, "MQTT_PORT").toInt();
 
  const String hostName = utilscds->getCampoCredencial(USER, "HOST");
  const String apiVersion = utilscds->getCampoCredencial(USER, "API_VERSION");
  const String callerOrigin = utilscds->getCampoCredencial(USER, "CALLER_ORIGIN");
  const String decrypted_apiToken = utilscds->decrypta(utilscds->getCampoCredencial(USER, "API_TOKEN"), utilscds->getCampoCredencial(USER, "API_TOKEN_LENGTH").toInt());  
  const String decrypted_apiOpenAIToken  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "OPEN_IA_KEY"),utilscds->getCampoCredencial(USER, "OPEN_IA_KEY_LENGTH").toInt());
  
  const String smtpHost = utilscds->getCampoCredencial(USER, "SMTP_HOST");
  const int smtpPort = utilscds->getCampoCredencial(USER, "SMTP_PORT").toInt();
  const String authorEmail = utilscds->getCampoCredencial(USER, "AUTHOR_EMAIL");
  const String decrypted_emailAuthorPass  = utilscds->decrypta(utilscds->getCampoCredencial(USER, "AUTHOR_PASSWORD"),utilscds->getCampoCredencial(USER, "AUTHOR_PASSWORD_LENGTH").toInt());
  const String recipientName = utilscds->getCampoCredencial(USER, "RECIPIENT_NAME");
  const String recipientEmail = utilscds->getCampoCredencial(USER, "RECIPIENT_EMAIL");

  #ifdef DEBUG
    Serial.println("decrypted_userFirmware: "+decrypted_userFirmware);
    Serial.println("decrypted_passFirmware: "+decrypted_passFirmware);
    //Serial.println("decrypted_apiToken: "+decrypted_apiToken);
    Serial.println("host: http://"+hostName+".local");    
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
    Serial.printf("Heap livre antes do AP: %d bytes\n", ESP.getFreeHeap());
    websrvhdl->startWebServerWifiManager(apName);
    Serial.println("WiFi não configurado!");
    Serial.println("Por favor, conecte-se em: " + apName + " e entre em: http://" + hostName + ".local para configuração do WiFi.");
  } else {
    ElegantOTA.begin(websrvhdl->getWebServer(), decrypted_userFirmware.c_str(), decrypted_passFirmware.c_str());
    Serial.println("OTA inicializado");
    Serial.printf("Heap após OTA: %d\n", ESP.getFreeHeap());
    Serial.printf("Heap maior bloco: %d\n", ESP.getMaxAllocHeap());

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