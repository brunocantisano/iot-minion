// UtilsHandler.h
#ifndef UTIL_H
#define UTIL_H

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "ListaEncadeada.h"
#include "Tipos.h"
#include "Config.h"

class UtilsHandler {
public:
    UtilsHandler();
    void setClock();
    String IpAddress2String(const IPAddress& ipAddress);
    String getData(uint8_t *data, size_t len);
    String getDataHora();
    String getDateTimeString();
    String uint64ToText(uint64_t input);
    String getDoubleAsString(double valor);
    String getMimeType(const String& filename);
    String sanitizeFilename(const String& filename);
    String humanReadableSize(const size_t bytes);
private:
	  inline bool is_base64(unsigned char c);
    String base64_decode(const String &encoded_string);
};

#endif
