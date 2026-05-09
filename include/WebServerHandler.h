// WebServerHandler.h
#ifndef WEBSERVERHANDLER_H
#define WEBSERVERHANDLER_H

#undef HTTP_GET
#undef HTTP_POST
#undef HTTP_PUT
#undef HTTP_DELETE

#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <pgmspace.h>   // PROGMEM
#include <ArduinoJson.h>
#include "Tipos.h"
#include "StorageHandler.h"
#include "UtilsHandler.h"
#include "GnssZedf9pHandler.h"
#include "PreferencesHandler.h"
#include "HttpStatusCodes.h"
#include <ArduinoUtilsCds.h>
#include "WebMessages.h"

//Volume
#define DEFAULT_VOLUME               20
#define RelayHat                     13
#define RelayEyes                    14
#define RelayBlink                   15
#define RelayShake                   22
#undef TemperatureHumidity
#define TemperatureHumidity          33

#define HTTP_REST_PORT               80

/**********************************************
 *  HTML fallback do portal (se /wifimanager.html não existir)
 **********************************************/
static const char HTML_FALLBACK[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Configurar Wi-Fi</title>
<style>
body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial;margin:24px}
form{max-width:420px;margin:auto}
input,button{width:100%;padding:12px;margin:8px 0;font-size:16px}
button{cursor:pointer}
</style></head><body>
<h2>Configurar Wi-Fi</h2>
<form method="POST" action="/save">
  <label>SSID <input name="ssid" required></label>
  <label>Senha <input name="pass" type="password" required></label>
  <button type="submit">Salvar e reiniciar</button>
</form>
</body></html>
)HTML";

class WebServerHandler {
private:
  AsyncWebServer * server;
  AsyncWebSocket * ws;      // rota do websocket
  bool _apMode = false;
  DNSServer dns;
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
  void loop();
};

#endif
