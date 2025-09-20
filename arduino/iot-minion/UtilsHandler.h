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
    void addApplication(String name, String language, String description);
    void removeApplication(int index);
    void addMedia(String name, int size, String lastModified);
    String listApplicationJson();
    String listMediaJson();
    String listSensorJson();
    bool loadSensorList();
    String saveApplicationList();
    bool addSensor(int id, int gpio, String name);
    ArduinoSensorPort * searchListSensor(int gpio);
    int searchList(String name, String language);
    bool readSensorStable(int pin, uint8_t samples = 7, uint16_t gap_ms = 3);
private:
    ListaEncadeada<Media*> mediaListaEncadeada = ListaEncadeada<Media*>();                              // Lista de media no sdcard
    ListaEncadeada<ArduinoSensorPort*> sensorListaEncadeada = ListaEncadeada<ArduinoSensorPort*>();     // Lista de sensores
    ListaEncadeada<Application*> applicationListaEncadeada = ListaEncadeada<Application*>();            // Lista de aplicacoes do jenkins
	  inline bool is_base64(unsigned char c);
    String base64_decode(const String &encoded_string);
};

#endif
