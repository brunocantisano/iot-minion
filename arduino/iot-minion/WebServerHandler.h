// WebServerHandler.h
#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_PUT
#undef HTTP_DELETE

/*******************************
 *  Build para ESP32
 *******************************/
#if defined(ESP32)
  #include <WiFi.h>               // em vez de ESP8266WiFi.h
  #include <AsyncTCP.h>           // em vez de ESPAsyncTCP.h
  #include <ESPAsyncWebServer.h>  // igual
  #include <ESPAsyncDNSServer.h>  // ou <DNSServer.h> se preferir síncrono
#elif defined(ESP8266)
/*******************************
 *  Build para ESP8266
 *******************************/
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <ESPAsyncDNSServer.h>
#else
  #error "Plataforma não suportada"
#endif

#include "Tipos.h"
#include "WebMessages.h"
#include "StorageHandler.h"
#include "UtilsHandler.h"
#include "PreferencesHandler.h"
#include "HttpStatusCodes.h"
#include "AudioHandler.h"
#include "MqttHandler.h"
#include "SdCardHandler.h"
#include "TemperatureHandler.h"
#include <pgmspace.h>   // PROGMEM
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define HTTP_REST_PORT             80
#define MAX_PAYLOAD_SIZE           2000

class WebServerHandler {
private:
    AsyncWebServer * server;
    AsyncWebSocket * ws;      // rota do websocket
    StorageHandler * strhdl;
    UtilsHandler * utilshdl;
    AudioHandler * audiohdl;
    MqttHandler * mtthdl;
    TemperatureHandler * temphdl;
    SdCardHandler * sdcardhdl;
    PreferencesHandler * prefshdl;
    AsyncDNSServer dns;
    String apiToken;
    String apiVersion;
    String host;
    String apiChatGptToken;
    String chatGPTUrl;
    String mqttUser;
    String mqttPass;
    String mqttBroker;
    String callerOrigin;
    uint64_t sdcard_total;
    uint64_t sdcard_used;
    String savedSsid;
    String savedPass;
 
    String obtemEstadoSensor(int pin);
    String obtemMetricas();
    void atribuiMetrica(String *p, String metric, String value);
    int obtemContagemBoots();
    void incrementaContagemBoots();
    bool check_authorization_header(AsyncWebServerRequest * request);
    char payloadBuffer[MAX_PAYLOAD_SIZE];
    void handleFileServing();
    void handleHomeRaw();
    void handleHome();
    void handleCiCd();
    void handleSwagger();
    void handleSwaggerUI();
    void handleHealth();
    void handleMetrics();
    void handlePorts();
    void handleAudios();
    void handleSensors();
    void handleUpdateSensors();
    void handleLists();
    void handleTemperatureAndHumidity();
    void handleInsertTalk();
    void handleInsertAsk();
    void handleInsertPlay();
    void handleInsertPlayRemote();
    void handleVolume();
    void handleInsertItemList();
    void handleDeleteItemList();
    void handleDeleteFile();
    void handleListStorage();
    void handleUploadStorage();
    void handleListSdcard();
    void handleUploadSdCard();
    void handleInsertJigSaw();
    void handleOptions();
    void handleOnError();    
    void registerPortalRoutes();
    String treatTemperatureAndHumidity(String field, String value);
    String enviarMensagemParaChatGPT(String mensagem);
    void handleUploadStorage(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void handleUploadSdcard(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    //void notifySensors(const String& id, bool s25, bool s50, bool s75, bool s100);
    //void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
public:
    WebServerHandler(
        const String& token, 
        const String& version, 
        const String& hostServer, 
        const String& chatGptToken,
        const String& mqttUsername,
        const String& mqttPassword,
        const String& mqttBrokerHost,
        const String& caller);
    
    ~WebServerHandler();

    void startWebServer(void);
    bool connectSTA(const String& hostForMDNS);
    void startWebServerWifiManager(const String& apName);
    void loop();
    AsyncWebServer * getWebServer();
};

#endif
