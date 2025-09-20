// CredentialsHandler.h
#ifndef CREDENTIALS_HANDLER_H
#define CREDENTIALS_HANDLER_H

#include <Arduino.h>
#include "StorageHandler.h"
#include <map>
#include <iterator>

struct Credentials {
  String mqttBroker;
  String mqttUsername;
  int mqttUsernameLength;
  String mqttPassord;
  int mqttPassordLength;
  int mqttPort;
  // Base
  String userFirmware;
  int userFirmwareLength;  
  String passFirmware;
  int passFirmwareLength;    
  String host;
  String openIA_Key; /* Your private API Key see https://bit.ly/OpenAI-Dev to get setup */
  int openIA_KeyLength;
  String apiToken;
  int apiTokenLength;
  String callerOrigin;
  // Base
  String apiVersion;  
  bool valid;      
};

class CredentialsHandler {
public:
    CredentialsHandler();
    ~CredentialsHandler();
    Credentials parseAndValidateCredentials(const char** requiredFields, size_t count);
private:
    StorageHandler * strhdl;
};
#endif
