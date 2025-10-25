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
  #include <HTTPClient.h>
  #include <AsyncTCP.h>           // em vez de ESPAsyncTCP.h
  #include <ESPAsyncWebServer.h>  // igual
  #include <ESPAsyncDNSServer.h>  // ou <DNSServer.h> se preferir síncrono
#elif defined(ESP8266)
/*******************************
 *  Build para ESP8266
 *******************************/
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <ESPAsyncDNSServer.h>
#else
  #error "Plataforma não suportada"
#endif
#include <ArduinoUtilsCds.h>
#include <pgmspace.h>   // PROGMEM
#include <ArduinoJson.h>
#include "WebMessages.h"

#define HTTP_REST_PORT               80

//Volume
#define DEFAULT_VOLUME               20
#define RelayHat                     13
#define RelayEyes                    14
#define RelayBlink                   15
#define RelayShake                   22
#define TemperatureHumidity          33

class WebServerHandler {
private:
    AsyncWebServer * server;
    AsyncWebSocket * ws;      // rota do websocket
    AsyncDNSServer dns;
    String apiToken;
    String apiVersion;
    String host;
    ArduinoUtilsCds * utilscds;
    String callerOrigin;
    String chatGPTUrl;
    String savedSsid;
    String savedPass;
    ListaEncadeada<Media*> mediaListaEncadeada = ListaEncadeada<Media*>();                              // Lista de media no sdcard
    ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();     // Lista de sensores
    ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();            // Lista de aplicacoes do jenkins
    String obtemEstadoSensor(int pin);
    String obtemMetricas();
    void atribuiMetrica(String *p, String metric, String value);
    int obtemContagemBoots();
    void incrementaContagemBoots();
    bool check_authorization_header(AsyncWebServerRequest * request);
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
    void handleWiFiManager();
    void handleSaveCredentials();
    void handleOptions();
    void handleOnError();    
    void registerPortalRoutes();
    String treatTemperatureAndHumidity(String field, String value);
    String enviarMensagemParaChatGPT(String mensagem);
    void handleUploadStorage(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void handleUploadSdcard(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void addApplication(String name, String language, String description);
    void removeApplication(int index);
    void addMedia(String name, int size, String lastModified);
    String listApplicationJson();
    String listMediaJson();
    String listSensorJson();
    String saveApplicationList();
    bool addSensor(int id, int gpio, String name);
    ArduinoSensorPort * searchListSensor(int gpio);
    int searchList(String name, String language);
public:
    WebServerHandler(
        const String& token, 
        const String& version, 
        const String& hostServer, 
        const String& caller,
        ArduinoUtilsCds * cds);
    
    ~WebServerHandler();

    void startWebServer(void);
    bool connectSTA(const String& hostForMDNS);
    bool loadSensorList();
    void startWebServerWifiManager(const String& apName);
    AsyncWebServer * getWebServer();
};

#endif
