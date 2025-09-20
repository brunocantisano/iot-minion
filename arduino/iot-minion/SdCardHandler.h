// SdCardHandler.h
#ifndef SDCARDHANDLER_H
#define SDCARDHANDLER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "UtilsHandler.h"

//Pinos de conexão do ESP32 e o módulo de cartão SD
#define SD_CS                        5
#define SCK                          18
#define MISO                         19
#define MOSI                         23

class SdCardHandler {
public:
  SdCardHandler();
  void loadI2S();
  bool loadSdCardMedias(String token);
  String listFilesSD(File dir, int numTabs, String token);
  String getSdcardTotal();
  String getSdcardUsed();
  bool getAllowedSdCardFiles(String filename);
  bool getAllowedStorageFiles(String filename);
private:
  UtilsHandler utilshdl;
  uint64_t sdcard_total;
  uint64_t sdcard_used;
};

#endif
