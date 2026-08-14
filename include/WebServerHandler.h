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
 *  HTML fallback do portal (se /wifimanager.html não existir no LittleFS)
 **********************************************/
static const char HTML_FALLBACK[] PROGMEM = R"HTML(
<!doctype html><html lang="pt-BR"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Arquivo não encontrado</title><style>:root{--bg:#0f172a;--card:#1e293b;--card-border:#334155;--accent:#f59e0b;--text:#e2e8f0;--muted:#94a3b8;--code-bg:#0f172a;--warn:#f59e0b}*{box-sizing:border-box}body{font-family:system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;margin:0;min-height:100vh;display:flex;align-items:center;justify-content:center;background:radial-gradient(1200px 600px at 50% -10%,#1e293b 0,var(--bg) 60%);color:var(--text);padding:24px}.card{width:100%;max-width:460px;background:var(--card);border:1px solid var(--card-border);border-radius:16px;padding:32px 28px;box-shadow:0 20px 50px rgba(0,0,0,.45)}.brand{display:flex;align-items:center;gap:12px;margin-bottom:8px}.brand .icon{width:44px;height:44px;flex:0 0 44px;display:flex;align-items:center;justify-content:center;border-radius:12px;background:linear-gradient(135deg,var(--accent),#d97706)}.brand .icon svg{width:24px;height:24px}h1{font-size:19px;margin:0;font-weight:600}.sub{color:var(--muted);font-size:13px;margin:4px 0 0}.body-text{font-size:14px;line-height:1.6;color:var(--text);margin:22px 0 18px}.body-text strong{color:#fff}.steps{background:var(--code-bg);border:1px solid var(--card-border);border-radius:10px;padding:16px 18px;margin-bottom:20px}.steps p{margin:0 0 10px;font-size:12px;text-transform:uppercase;letter-spacing:.04em;color:var(--muted);font-weight:600}.steps ol{margin:0;padding-left:20px;font-size:13.5px;line-height:1.9;color:var(--text)}.steps code{background:#1e293b;border:1px solid var(--card-border);border-radius:4px;padding:2px 6px;font-size:12.5px;color:var(--accent);font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace}.foot{display:flex;align-items:center;gap:8px;margin-top:20px;color:var(--muted);font-size:12px;justify-content:center}.dot{width:8px;height:8px;border-radius:50%;background:var(--warn);box-shadow:0 0 8px var(--warn)}</style></head><body><div class="card"><div class="brand"><div class="icon"><svg viewBox="0 0 24 24" fill="none" stroke="#fff" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 9v4"/><path d="M12 17h.01"/><path d="M10.29 3.86 1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0Z"/></svg></div><div><h1>Página do portal não encontrada</h1><div class="sub">O arquivo <code style="background:#0f172a;border:1px solid #334155;border-radius:4px;padding:1px 5px;color:#f59e0b">wifimanager.html</code> não está no armazenamento</div></div></div><p class="body-text">Esta é uma <strong>página de emergência</strong> exibida automaticamente porque o firmware não encontrou o arquivo de interface do portal Wi-Fi no sistema de arquivos (LittleFS) do dispositivo. Isso geralmente acontece quando o firmware foi gravado sem o upload correspondente dos arquivos estáticos.</p><div class="steps"><p>Como resolver</p><ol><li>Confirme que o arquivo <code>wifimanager.html</code> existe na pasta de dados do projeto (<code>/data</code>).</li><li>Grave o sistema de arquivos no dispositivo, via PlatformIO: <code>pio run --target uploadfs</code></li><li>Reinicie o dispositivo após a gravação concluir.</li></ol></div><p class="body-text" style="margin:0;font-size:13px;color:var(--muted)">Se o problema persistir, verifique se o <code>board_build.filesystem</code> está configurado como <code>littlefs</code> no <code>platformio.ini</code> e se a partição de dados tem espaço suficiente.</p><div class="foot"><span class="dot"></span><span>Modo de configuração ativo — Ponto de acesso</span></div></div></body></html>
)HTML";

class WebServerHandler {
private:
  AsyncWebServer * server;
  AsyncWebSocket * ws;      // rota do websocket 
  StorageHandler * strhdl;
  UtilsHandler * utilshdl;
  ArduinoUtilsCds * utilscds;
  PreferencesHandler * prefshdl;
  DNSServer dns;
  String apiToken;
  String apiVersion;
  String host;
  String callerOrigin;
  bool _apMode;
  // Restart adiado apos /save: so reinicia quando o cliente confirmar que
  // recebeu a pagina de confirmacao (onDisconnect), com um prazo maximo de
  // seguranca caso a desconexao nunca chegue a disparar.
  bool _pendingRestartAfterSave = false;
  unsigned long _pendingRestartDeadline = 0;
  // /talk, /ask e /playRemote fazem chamadas de rede bloqueantes (TTS,
  // ChatGPT, stream de radio). Rodar isso direto no callback do
  // AsyncWebServer estoura o stack/watchdog da task e reinicia o ESP - por
  // isso a acao so e enfileirada aqui e executada de fato no loop() (mesmo
  // padrao do _pendingRestartAfterSave acima).
  enum class PendingAudioAction { None, Talk, Ask, PlayRemote };
  PendingAudioAction _pendingAudioAction = PendingAudioAction::None;
  String _pendingAudioPayload;
  // Envio de e-mail (SMTP) tambem e uma chamada de rede bloqueante - mesmo
  // motivo/mesmo padrao do _pendingAudioAction acima.
  bool _pendingEmailSend = false;
  String _pendingEmailFilename;
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
  void handleSendEmail();
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
  String saveApplicationList();
  bool addSensor(int id, int gpio, String name);
  ArduinoSensorPort * searchListSensor(int gpio);
  ArduinoSensorPort * searchListSensorById(int id);
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
  void atualizaApiToken(const String& token);
};

#endif
