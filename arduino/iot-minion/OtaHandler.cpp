// OtaHandler.cpp
#include "OtaHandler.h"

void OtaHandler::begin(AsyncWebServer * server, const String& username, const String& password) {
    ElegantOTA.begin(server, username.c_str(), password.c_str());
}

void OtaHandler::loop() {
    ElegantOTA.loop();
}
