// StorageCompat.h
#pragma once

#if defined(ESP8266)
  #include <FS.h>
  #include <LittleFS.h>      // ESP8266 core 3.x
  #include <EEPROM.h>        // para emular Preferences
  using FS_t = fs::FS;
  using File_t = fs::File;

  // Modos de abertura no ESP8266: usar strings
  #define FS_OPEN_READ   "r"
  #define FS_OPEN_WRITE  "w"
  #define FS_OPEN_APPEND "a"

  inline bool FS_begin() {
    #ifdef USE_SD
      return SD.begin();
    #else
      bool ret = LittleFS.begin();
      if (!ret) {
        Serial.println(F("[FS] LittleFS.begin() FAIL — tente formatar"));
        LittleFS.format();          // opcional
        ret=LittleFS.begin();           // tente novamente    
      }
      return ret;
    #endif
  }

  // "Preferences" shim simples usando EEPROM (exemplo)
  class PrefsShim {
  public:
    bool begin(const char* ns = nullptr, bool readonly = false) {
      (void)ns; (void)readonly;
      EEPROM.begin(2048); // ajuste ao seu uso
      return true;
    }
    void end() { EEPROM.end(); }
    bool putString(const char* key, const String& val) {
      // grava em arquivo para simplicidade (melhor que EEPROM pura)
      File_t f = LittleFS.open(String("/") + key + ".txt", FS_OPEN_WRITE);
      if (!f) return false;
      f.print(val);
      f.close();
      return true;
    }
    String getString(const char* key, const String& def = "") {
      File_t f = LittleFS.open(String("/") + key + ".txt", FS_OPEN_READ);
      if (!f) return def;
      String s = f.readString();
      f.close();
      s.trim();
      return s.length() ? s : def;
    }
    bool remove(const char* key) {
      return LittleFS.remove(String("/") + key + ".txt");
    }
  };

#else // ESP32
  #include <FS.h>
  #include <LittleFS.h>      // ESP32 core 2/3.x
  #include <Preferences.h>
  using FS_t = fs::FS;
  using File_t = fs::File;

  // Modos de abertura no ESP32: constantes
  #define FS_OPEN_READ   FILE_READ
  #define FS_OPEN_WRITE  FILE_WRITE
  #define FS_OPEN_APPEND FILE_APPEND

  inline bool FS_begin() {
    // ESP32: pode pedir format on fail
    #ifdef USE_SD
      return SD.begin();
    #else
      return LittleFS.begin(true);
    #endif
  }

  using PrefsShim = Preferences; // usa a nativa

#endif
