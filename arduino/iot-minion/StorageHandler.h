// StorageHandler.h
#ifndef STORAGE_H
#define STORAGE_H

// --- Compat FS para ESP8266 x ESP32 ---
#if defined(ESP8266)
  #include <FS.h>
  #include <LittleFS.h>
  using File_t = fs::File;
  #define FS_OPEN_READ   "r"
  #define FS_OPEN_WRITE  "w"
  #define FS_OPEN_APPEND "a"
#else
  #include <FS.h>
  #include <LittleFS.h>
  using File_t = fs::File;
  #define FS_OPEN_READ   FILE_READ
  #define FS_OPEN_WRITE  FILE_WRITE
  #define FS_OPEN_APPEND FILE_APPEND
#endif

#include <Arduino.h>
#include <map>
#include <base64.h>
#include <vector>
#include <algorithm>
#include "StorageCompat.h"
#include "UtilsHandler.h"

#define FORMAT_LITTLEFS_IF_FAILED true

class StorageHandler {
public:
    StorageHandler();
    // Funções utilitárias de arquivos e diretórios
    bool begin();
    void listDir(fs::FS &fs, const char* dirname, uint8_t levels);
    void createDir(fs::FS &fs, const char * path);
    void removeDir(fs::FS &fs, const char * path);
    bool readFile(fs::FS &fs, const char* path, String& out);
    String base64EncodeFile(fs::FS &fs, const char * path);
    void writeFile(fs::FS &fs, const char* path, const char* message);
    void appendFile(fs::FS &fs, const char* path, const char* message);
    void renameFile(fs::FS &fs, const char * path1, const char * path2);    
    bool deleteFile(fs::FS &fs, const char * path);
    bool fileExists(fs::FS &fs, const char * path);
    void testFileIO(fs::FS &fs, const char* path);
    String listFiles();    
    bool createNewLogFile();
    void logMessage(String msg);
    void manageLogFiles();
    String getCurrentLogFile();    
private:
    String currentLogFile;
    static constexpr const char* LITTLEFS_ERROR = 
        "Erro ao montar LittleFS. Verifique se o sistema de arquivos foi iniciado com begin() e se os arquivos existem.";
    UtilsHandler utilshdl;
};
#endif
