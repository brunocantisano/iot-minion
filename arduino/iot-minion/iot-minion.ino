#include "CryptoHandler.h"
#include "CredentialsHandler.h"
#include "OtaHandler.h"
#include "WebServerHandler.h"

#define SERIAL_PORT       115200

// ====== Objetos do seu projeto ======
CredentialsHandler * credhdl;
CryptoHandler * crypto;
OtaHandler ota;
WebServerHandler * websrvhdl = nullptr;

Credentials creds;
String decrypted_userFirmware, decrypted_passFirmware, decrypted_apiToken, decrypted_userMqtt, decrypted_passMqtt, decrypted_openIA_Key;
bool isWiFiConnected = false;
//---------------------------------//
/**********************************************
 *  SETUP
 **********************************************/
 void setup() {
  Serial.begin(SERIAL_PORT);
  Serial.println("\nBoot...");

  // === Carrega credenciais de firmware/host etc. (credentials.txt) === 
  static const char* required[] = {
    "MQTT_BROKER", "MQTT_USERNAME", "MQTT_USERNAME_LENGTH", "MQTT_PASSORD", "MQTT_PASSORD_LENGTH", "MQTT_PORT",
    "USER_FIRMWARE", "USER_FIRMWARE_LENGTH", "PASS_FIRMWARE", "PASS_FIRMWARE_LENGTH",
    "HOST", "API_TOKEN", "API_TOKEN_LENGTH", "OPEN_IA_KEY", "OPEN_IA_KEY_LENGTH", "API_VERSION", "CALLER_ORIGIN"
  };
  const size_t requiredSize = sizeof(required) / sizeof(required[0]);
  credhdl = new CredentialsHandler();
  creds = credhdl->parseAndValidateCredentials(required, requiredSize);
  delete credhdl;
  if (creds.valid) {    
    crypto = new CryptoHandler();
    decrypted_userMqtt = crypto->decrypt(creds.mqttUsername, creds.mqttUsernameLength);
    decrypted_passMqtt = crypto->decrypt(creds.mqttPassord, creds.mqttPassordLength);
    decrypted_userFirmware = crypto->decrypt(creds.userFirmware, creds.userFirmwareLength);
    decrypted_passFirmware = crypto->decrypt(creds.passFirmware, creds.passFirmwareLength);
    decrypted_apiToken     = crypto->decrypt(creds.apiToken, creds.apiTokenLength);
    decrypted_openIA_Key   = crypto->decrypt(creds.openIA_Key, creds.openIA_KeyLength);

    Serial.println("decrypted_userMqtt: "+decrypted_userMqtt);
    Serial.println("decrypted_passMqtt: "+decrypted_passMqtt);
    Serial.println("decrypted_userFirmware: "+decrypted_userFirmware);
    Serial.println("decrypted_passFirmware: "+decrypted_passFirmware);
    Serial.println("decrypted_apiToken: "+decrypted_apiToken);

    const String hostName = creds.host.isEmpty() ? String("device") : creds.host;
  
    // === Servidor principal e OTA (só quando conectado) ===
    websrvhdl = new WebServerHandler(
      decrypted_apiToken.c_str(), 
      creds.apiVersion, 
      hostName,
      decrypted_userMqtt,
      decrypted_passMqtt,
      creds.mqttBroker,
      decrypted_openIA_Key,
      creds.callerOrigin
    );

    // === Wi-Fi: tenta STA; se falhar, abre portal ===
    isWiFiConnected = websrvhdl->connectSTA(hostName);
    if (!isWiFiConnected) {
      String apName = hostName.isEmpty() ? String("device-setup") : (hostName + "-setup");
      websrvhdl->startWebServerWifiManager(apName);
      Serial.println("WiFi não configurado!");
      Serial.println("Por favor, conecte-se em: " + apName + " e entre em: http://" + hostName + ".local para configuração do WiFi.");
    } else {                         
      pinMode(RelayEyes, OUTPUT);
      pinMode(RelayHat, OUTPUT);
      pinMode(RelayBlink, OUTPUT);
      pinMode(RelayShake, OUTPUT);
      pinMode(TemperatureHumidity, OUTPUT);
      
      websrvhdl->startWebServer();   // registra rotas no 'server' e chama server->begin() lá dentro
      ota.begin(websrvhdl->getWebServer(), decrypted_userFirmware, decrypted_passFirmware);
      Serial.println("OTA inicializado");

      const char * hostname = hostName.c_str();
      MDNS.end();
      if(!MDNS.begin(hostname)){
        Serial.println("mDNS falhou");
        delay(1000);
        delete crypto;
        delete websrvhdl;
        ESP.restart();
      }
      MDNS.addService("http", "tcp", 80);
      Serial.print(F("mDNS ok: http://"));
      Serial.print(hostname);     // hostname = const char* ou String
      Serial.println(F(".local"));
    }
  } else {
    Serial.println("Credenciais inválidas em /credentials.txt");
  }
}

/**********************************************
 *  LOOP
 **********************************************/
void loop() {
  if (isWiFiConnected) {
    //MDNS.update();
    ota.loop(); // se o seu OtaHandler exigir
    websrvhdl->loop();
  }
}
