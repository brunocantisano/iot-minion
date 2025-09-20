#pragma once
#include <Arduino.h>
#include "StorageCompat.h"   // dá acesso a PrefsShim, LittleFS, File_t, FS_OPEN_*

class PreferencesHandler {
public:
  void putStringCompat(const char * key, const char * v);
  String getStringCompat(const char * key, const char * def);
  void saveDataPreferentials(const char * session, const char * name, const char * value);
  void removeDataPreferentials(const char * session, const char * name);
  String loadDataPreferentials(const char * session, const char * name, const char * value);
  bool sessionExists(const char * session, const char * itemName);
  void writeSession(const char * session, const char * itemName);
  void removeSession(const char * session, const char * itemName);
private:
  PrefsShim preferences;   // <<<<<< ESTE MEMBRO RESOLVE o “not declared in this scope”
};
