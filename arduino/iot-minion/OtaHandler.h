// OtaHandler.h
#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1

#include <ElegantOTA.h>

class OtaHandler {
public:
    void begin(AsyncWebServer * server, const String& username, const String& password);
    void loop();
};

#endif
