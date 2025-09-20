#include "PreferencesHandler.h"

void PreferencesHandler::putStringCompat(const char * key, const char* v) {
  preferences.putString(key, String(v));
}

String PreferencesHandler::getStringCompat(const char * key, const char* def) {
  return preferences.getString(key, String(def));
}

void PreferencesHandler::saveDataPreferentials(const char * session, const char * name, const char * value) {
  preferences.begin(session, false);
  putStringCompat(name, value);
  preferences.end();
}

void PreferencesHandler::removeDataPreferentials(const char * session, const char * name) {
  preferences.begin(session, false);
  preferences.remove(name);
  preferences.end();
}

String PreferencesHandler::loadDataPreferentials(const char* session, const char * name, const char * value) {
  preferences.begin(session, true);
  String ret = getStringCompat(name, value);
  preferences.end();
  return ret;
}

bool PreferencesHandler::sessionExists(const char * session, const char * itemName) {
  preferences.begin(session, true); // read-only
  bool exists = preferences.getBool(itemName, false);
  preferences.end();
  return exists;
}

void PreferencesHandler::writeSession(const char * session, const char * itemName) {
  preferences.begin(session, false); // read-write
  preferences.putBool(itemName, true);
  preferences.end();
}

void PreferencesHandler::removeSession(const char * session, const char * itemName) {
  preferences.begin(session, false);
  preferences.remove(itemName);
  preferences.end();
}