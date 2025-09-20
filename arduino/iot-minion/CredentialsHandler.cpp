// CredentialsHandler.cpp
#include "StorageHandler.h"
#include "CredentialsHandler.h"

CredentialsHandler::CredentialsHandler() {
  strhdl = new StorageHandler();
}

CredentialsHandler::~CredentialsHandler() {

}

Credentials CredentialsHandler::parseAndValidateCredentials(const char** requiredFields, size_t count) {
    Credentials creds;
    creds.valid = false;
    String payload;
    std::map<String, String> credentialsMap;
    strhdl->begin();
    if (strhdl->readFile(LittleFS, "/credentials.txt", payload)) {
      // Parse manual do formato KEY=VALUE por linha
      size_t start = 0;
      while (start < payload.length()) {
          int end = payload.indexOf('\n', start);
          if (end == -1) end = payload.length();
  
          String line = payload.substring(start, end);
          line.trim();
  
          if (line.length() > 0) {
              int sep = line.indexOf('=');
              if (sep != -1) {
                  String key = line.substring(0, sep);
                  String value = line.substring(sep + 1);
                  key.trim();
                  value.trim();
                  credentialsMap[key] = value;
              }
          }
          start = end + 1;
      }
  
      // Validação dos campos obrigatórios
      for (size_t i = 0; i < count; ++i) {
          const char* key = requiredFields[i];
          if (credentialsMap.count(key) == 0 || credentialsMap[key].isEmpty()) {
              return creds; // Campo ausente ou vazio
          }
      }
  
      // Atribuição dos campos se existirem
      
      // Campos BASE obrigatórios
      if (credentialsMap.count("MQTT_BROKER")) creds.mqttBroker = credentialsMap["MQTT_BROKER"];
      if (credentialsMap.count("MQTT_USERNAME")) creds.mqttUsername = credentialsMap["MQTT_USERNAME"];
      if (credentialsMap.count("MQTT_USERNAME_LENGTH")) creds.mqttUsernameLength = credentialsMap["MQTT_USERNAME_LENGTH"].toInt();
      if (credentialsMap.count("MQTT_PASSWORD")) creds.mqttPassord = credentialsMap["MQTT_PASSWORD"];
      if (credentialsMap.count("MQTT_PASSWORD_LENGTH")) creds.mqttPassordLength = credentialsMap["MQTT_PASSWORD_LENGTH"].toInt();
      if (credentialsMap.count("MQTT_PORT")) creds.mqttPort = credentialsMap["MQTT_PORT"].toInt();

      if (credentialsMap.count("USER_FIRMWARE")) creds.userFirmware = credentialsMap["USER_FIRMWARE"];
      if (credentialsMap.count("USER_FIRMWARE_LENGTH")) creds.userFirmwareLength = credentialsMap["USER_FIRMWARE_LENGTH"].toInt();
      if (credentialsMap.count("PASS_FIRMWARE")) creds.passFirmware = credentialsMap["PASS_FIRMWARE"];
      if (credentialsMap.count("PASS_FIRMWARE_LENGTH")) creds.passFirmwareLength = credentialsMap["PASS_FIRMWARE_LENGTH"].toInt();
      if (credentialsMap.count("HOST")) creds.host = credentialsMap["HOST"];
      if (credentialsMap.count("OPEN_IA_KEY")) creds.openIA_Key = credentialsMap["OPEN_IA_KEY"];
      if (credentialsMap.count("OPEN_IA_KEY_LENGTH")) creds.openIA_KeyLength = credentialsMap["OPEN_IA_KEY_LENGTH"].toInt();
      if (credentialsMap.count("API_TOKEN")) creds.apiToken = credentialsMap["API_TOKEN"];
      if (credentialsMap.count("API_TOKEN_LENGTH")) creds.apiTokenLength = credentialsMap["API_TOKEN_LENGTH"].toInt();
      if (credentialsMap.count("CALLER_ORIGIN")) creds.callerOrigin = credentialsMap["CALLER_ORIGIN"];   
      if (credentialsMap.count("API_VERSION")) creds.apiVersion = credentialsMap["API_VERSION"];
      
      creds.valid = true;
    }
    return creds;
}
