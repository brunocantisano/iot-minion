#include <Arduino.h>
#include <ArduinoUtilsCds.h>
#include "WebServerHandler.h"
#include <ElegantOTA.h>
// ====== Objetos do seu projeto ======
WiFiClient wifiClientMqtt;
ArduinoUtilsCds * utilscds = nullptr;
String decrypted_userFirmware;
String decrypted_passFirmware;
String decrypted_userMqtt;
String decrypted_passMqtt;
String decrypted_apiToken;
String decrypted_apiOpenAIToken;
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
  Serial.println("\nBoot...");
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
  
  utilscds->iniciaStorage();
  utilscds->exibeMensagem("Inicializando o storage");
  // Carrega credenciais de texto simples
  String hostName = "minion";
  String version = String(API_VERSION);
  String callerOrigin = utilscds->carregaDado("api", "callerOrigin", "");
  utilscds->mensagemLog("[DEBUG] hostName.length() = %d", hostName.length());
  utilscds->mensagemLog("[DEBUG] version.length() = %d", version.length());
  utilscds->mensagemLog("[DEBUG] callerOrigin.length() = %d", callerOrigin.length());
  utilscds->mensagemLog("[DEBUG] hostName bytes: ");
  // Teste com array char em vez de String
  char hostBuffer[64];
  strncpy(hostBuffer, hostName.c_str(), 63);
  hostBuffer[63] = '\0';
  utilscds->mensagemLog("[DEBUG] HOST: %s", hostBuffer);  // Usa char[] em vez de String
  
  // URL final usando char buffer
  char urlBuffer[128];
  snprintf(urlBuffer, sizeof(urlBuffer), "URL esperada: http://%s.local", hostBuffer);
  utilscds->mensagemLog("[DEBUG] URL: %s", urlBuffer);

  if (version.isEmpty()) {
    utilscds->mensagemLog("[AVISO] API_VERSION não carregado, usando fallback '1.0.0'");
    version = "1.0.0";
  }
  
  if (callerOrigin.isEmpty()) {
    utilscds->mensagemLog("[AVISO] CALLER_ORIGIN não carregado, usando fallback '*'");
    callerOrigin = "*";
  }
  
  // === Servidor principal e OTA (só quando conectado) ===
  websrvhdl = new WebServerHandler(
    decrypted_apiToken.c_str(), 
    version, 
    hostName,
    callerOrigin,
    utilscds
  );

  // === Wi-Fi: tenta STA; se falhar, abre portal ===
  wifi_connected = websrvhdl->connectSTA(hostName);
  if (!wifi_connected) {
    String apName = hostName.isEmpty() ? String("device-setup") : (hostName + "-setup");
    utilscds->mensagemLog("[DEBUG] Heap livre antes do AP: %d bytes", ESP.getFreeHeap());
    websrvhdl->startWebServerWifiManager(apName);
    utilscds->mensagemLog("[DEBUG] WiFi não configurado!");
    utilscds->mensagemLog("[DEBUG] Por favor, conecte-se em: %s", apName.c_str());
    char urlBuffer[128];
    snprintf(urlBuffer, sizeof(urlBuffer), "URL esperada: http://%s.local", hostBuffer);
    utilscds->mensagemLog("[DEBUG] Entre em: http://%s.local para configuração do WiFi.", urlBuffer);
  } else {
    
    // === Carrega credenciais de firmware/host etc. (credentials.enc) ===     
    // Carrega valores brutos
    int userFirmwareLen               = utilscds->carregaDado("firmware", "userLen", "0").toInt();
    String userFirmare                = utilscds->carregaDado("firmware", "user", "");
    if (!userFirmare.isEmpty()) {
        decrypted_userFirmware = utilscds->decrypta(userFirmare, userFirmwareLen);
    } else {
        utilscds->mensagemLog("[ERRO] Falha ao usar credenciais de firmware!");
        delay(2000);
    }

    int passFirmwareLen               = utilscds->carregaDado("firmware", "passLen", "0").toInt();
    String passFirmware                = utilscds->carregaDado("firmware", "pass", "");
    if (!passFirmware.isEmpty()) {
        decrypted_passFirmware = utilscds->decrypta(passFirmware, passFirmwareLen);
    } else {
        utilscds->mensagemLog("[ERRO] Falha ao usar credenciais de firmware!");
        delay(2000);
    }

    int apiTokenLen               = utilscds->carregaDado("api", "tokenLen", "0").toInt();
    String apiToken               = utilscds->carregaDado("api", "token", "");
    if (!apiToken.isEmpty()) {
      decrypted_apiToken = utilscds->decrypta(apiToken, apiTokenLen);
      websrvhdl->atualizaApiToken(decrypted_apiToken);
    } else {
      utilscds->mensagemLog("[ERRO] Falha ao usar credenciais de firmware!");
      delay(2000);
    }
    
    int openaiTokenLen  = utilscds->carregaDado("openai", "tokenLen", "0").toInt();
    String openaiToken  = utilscds->carregaDado("openai", "token", "");
    if (!openaiToken.isEmpty()) {
      decrypted_apiOpenAIToken = utilscds->decrypta(openaiToken, openaiTokenLen);
    }
    
    // === MQTT: credenciais salvas pelo portal Wi-Fi (namespace "mqtt") ===
    String mqttBroker = utilscds->carregaDado("mqtt", "broker", "io.adafruit.com");
    uint16_t mqttPort  = (uint16_t)utilscds->carregaDado("mqtt", "port", "1883").toInt();

    int mqttUsernameLen  = utilscds->carregaDado("mqtt", "usernameLen", "0").toInt();
    String mqttUsername  = utilscds->carregaDado("mqtt", "username", "");
    if (!mqttUsername.isEmpty()) {
        decrypted_userMqtt = utilscds->decrypta(mqttUsername, mqttUsernameLen);
    }

    int mqttPasswordLen  = utilscds->carregaDado("mqtt", "passwordLen", "0").toInt();
    String mqttPassword  = utilscds->carregaDado("mqtt", "password", "");
    if (!mqttPassword.isEmpty()) {
        decrypted_passMqtt = utilscds->decrypta(mqttPassword, mqttPasswordLen);
    }

    #ifdef USE_MQTT
      if (!decrypted_userMqtt.isEmpty() && !decrypted_passMqtt.isEmpty()) {
        utilscds->iniciaMqtt(&wifiClientMqtt, mqttBroker, mqttPort, decrypted_userMqtt, decrypted_passMqtt);
        utilscds->mensagemLog("[DEBUG] MQTT inicializado (broker=%s:%u)", mqttBroker.c_str(), mqttPort);
      } else {
        utilscds->mensagemLog("[AVISO] Credenciais MQTT nao configuradas; MQTT desabilitado.");
      }
    #endif

    String smtpHost = utilscds->carregaDado("email", "smtp_host", "");
    int smtpPort = utilscds->carregaDado("email", "smtp_port", "0").toInt();
    String smtpAuthorEmail = utilscds->carregaDado("email","smtp_author_email", "");        
    String decrypted_authorPassword = utilscds->decrypta(utilscds->carregaDado("email","authorPassword", ""), utilscds->carregaDado("email","authorPasswordLen", "0").toInt());
    String recipientEmail = utilscds->carregaDado("email","recipientEmail", "");
    String recipientName = utilscds->carregaDado("email","recipientName", "");

    #ifdef USE_EMAIL
      if (!smtpHost.isEmpty() && !smtpAuthorEmail.isEmpty() && !decrypted_authorPassword.isEmpty() && smtpPort > 0) {
        utilscds->iniciaEmail(smtpAuthorEmail.c_str(), decrypted_authorPassword.c_str(), smtpAuthorEmail.c_str(),
                               "Minion ESP32", smtpHost.c_str(), (uint16_t)smtpPort);
        utilscds->mensagemLog("[DEBUG] Email inicializado (smtp=%s:%d)", smtpHost.c_str(), smtpPort);
      } else {
        utilscds->mensagemLog("[AVISO] Credenciais de email nao configuradas; envio de email desabilitado.");
      }
    #endif

    if (!userFirmare.isEmpty() && !passFirmware.isEmpty() && !apiToken.isEmpty()) {
      utilscds->mensagemLog("[DEBUG] Credenciais carregadas com sucesso!");
      utilscds->mensagemLog("[DEBUG] Credenciais brutas carregadas:");
      utilscds->mensagemLog("[DEBUG] USER_FIRMWARE (hex): %s (len param: %d)", decrypted_userFirmware.c_str(), userFirmwareLen);
      utilscds->mensagemLog("[DEBUG] PASS_FIRMWARE (hex): %s (len param: %d)", decrypted_passFirmware.c_str(), passFirmwareLen);
      utilscds->mensagemLog("[DEBUG] API_TOKEN (hex): %s (len param: %d)", decrypted_apiToken.c_str(), apiTokenLen);
    } else {
      utilscds->mensagemLog("[ERRO] Falha ao carregar credenciais!");
      delay(2000);
    }
    // Descriptografa com comprimentos validados   
    // ✅ Validação de credenciais descriptografadas
    if (decrypted_userFirmware.isEmpty() || decrypted_passFirmware.isEmpty() || decrypted_apiToken.isEmpty()) {
      utilscds->mensagemLog("[ERRO] Falha ao descriptografar credenciais de firmware e de api!");
      utilscds->mensagemLog("Causa possível:");
      utilscds->mensagemLog("  1. Chave AES (AES_KEY_HEX) está incorreta");
      utilscds->mensagemLog("  2. Valores hexadecimais são inválidos");
      delay(2000);
    }
      
    utilscds->mensagemLog("[DEBUG] [✓ Credenciais Carregadas]");
    utilscds->mensagemLog("[DEBUG] Credenciais:");
    utilscds->mensagemLog("[DEBUG] USER_FIRMWARE (decrypted): %s", decrypted_userFirmware.c_str());
    utilscds->mensagemLog("[DEBUG] PASS_FIRMWARE (decrypted): %s", decrypted_passFirmware.c_str());
    delay(50);
    websrvhdl->startWebServer();   // registra rotas no 'server' e chama server->begin() lá dentro
    Serial.println(F("[DEBUG-SERIAL] Web Server inicializado"));
    utilscds->mensagemLog("[DEBUG] Web Server inicializado");
    utilscds->mensagemLog("[DEBUG] Heap após startWebServer: %d", ESP.getFreeHeap());
    utilscds->mensagemLog("[DEBUG] Heap maior bloco: %d", ESP.getMaxAllocHeap());
    
    ElegantOTA.begin(websrvhdl->getWebServer(), decrypted_userFirmware.c_str(), decrypted_passFirmware.c_str());
    Serial.println(F("[DEBUG-SERIAL] OTA inicializado"));
    utilscds->mensagemLog("[DEBUG] OTA inicializado");
    utilscds->mensagemLog("[DEBUG] Heap após OTA: %d", ESP.getFreeHeap());
    utilscds->mensagemLog("[DEBUG] Heap maior bloco: %d", ESP.getMaxAllocHeap());
        
    utilscds->iniciaStorage();
    utilscds->exibeMensagem("Inicializando o storage");
    
    utilscds->iniciaSdCard();
    utilscds->exibeMensagem("Inicializando o sdcard");
    
    utilscds->mensagemLog("[HEAP] Livre: %d bytes", ESP.getFreeHeap());
    utilscds->mensagemLog("[HEAP] Maior bloco: %d bytes", ESP.getMaxAllocHeap());
    utilscds->iniciaTemperatura(DHT_11, TemperatureHumidity);
    utilscds->exibeMensagem("Inicializando a temperatura");
    
    utilscds->iniciaSound(decrypted_apiOpenAIToken, 25, 26, 27);
    utilscds->exibeMensagem("Inicializando o audio");
    
    pinMode(RelayEyes, OUTPUT);
    pinMode(RelayHat, OUTPUT);
    pinMode(RelayBlink, OUTPUT);
    pinMode(RelayShake, OUTPUT);
    pinMode(TemperatureHumidity, OUTPUT);

    // Atribuindo clock para conseguir usar datetime nos arquivos de log
    utilscds->atribuiRelogio();
    const char * hostname = hostName.c_str();
    MDNS.end();
    
    if(!MDNS.begin(hostname)){
      Serial.println(F("[DEBUG-SERIAL] mDNS falhou"));
      utilscds->mensagemLog("[DEBUG] mDNS falhou");
      delay(1000);
      delete websrvhdl;
      ESP.restart();
    }
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[DEBUG-SERIAL] mDNS ok: http://%s.local\n", hostname);
    utilscds->mensagemLog("[DEBUG] mDNS ok: http://");
    utilscds->mensagemLog("[DEBUG] %s.local", hostname);
  }
  Serial.println(F("[DEBUG-SERIAL] setup() concluido"));
}

/**********************************************
 *  LOOP
 **********************************************/
void loop() {
  currentMillis = millis();  
  // websrvhdl->loop() precisa rodar tanto em modo AP (portal cativo, restart
  // apos /save) quanto em modo STA - ele mesmo decide o que fazer via _apMode.
  if (websrvhdl) websrvhdl->loop();
  if (wifi_connected) {
    #ifdef USE_MQTT
      utilscds->atualizaMqtt();
    #endif
    #ifdef USE_AUDIO
      utilscds->loopAudio();  //Executa o loop interno da biblioteca audio
    #endif
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