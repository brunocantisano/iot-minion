#include "Config.h"
#include <SdCardHandler.h>

SdCardHandler::SdCardHandler():
  sdcard_total(0), sdcard_used(0){}

void SdCardHandler::loadI2S() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SCK, MISO, MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);
}

bool SdCardHandler::loadSdCardMedias(String token) {
  Serial.print(F("\nInitializing SD card..."));

  if (!SD.begin(SD_CS)) {    
      Serial.println(F("inicialização falhou. Lembre-se de checar:"));
      Serial.println(F("* o cartão está inserido?"));
      Serial.println(F("* as conexões dos fios estão corretas?"));
      Serial.println(F("* você mudou o pino chipSelect para corresponder ao seu chield ou módulo?"));
    return false;
  } else {
      Serial.println(F("conexões dos fios estão corretas e o cartão está presente."));
      Serial.println();
      Serial.print(F("Tipo de Cartão:         "));
      switch (SD.cardType()) {
        case CARD_NONE:
          Serial.println(F("Nenhum"));
          break;
        case CARD_MMC:
          Serial.println(F("MMC"));
          break;
        case CARD_SD:
          Serial.println(F("SD"));
          break;
        case CARD_SDHC:
          Serial.println(F("SDHC"));
          break;
        default:
          Serial.println(F("Desconhecido"));
      }    
    sdcard_total = SD.totalBytes() / (1024 * 1024);
    sdcard_used =  SD.usedBytes() / (1024 * 1024);
    
    Serial.println(F("Listando arquivos armazenados no cartão SD"));
    File dir =  SD.open("/", FILE_WRITE);
    listFilesSD(dir, 0, token);
    dir.close();
    return true;
  }
}

// list all of the files, if ishtml=true, return html rather than simple text
String SdCardHandler::listFilesSD(File dir, int numTabs, String token) {
  String returnText = "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th><th align='left'>Modificação</th><th align='center'></th><th align='center'>Remover</th></tr>";
  String lastModified;
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    } else {
      // Serial.print(entry.name());
      if (entry.isDirectory()) {
        listFilesSD(entry, numTabs + 1, token);
      } else {
        // files have sizes, directories do not
        time_t lw = entry.getLastWrite();
        struct tm * tmstruct = localtime(&lw);
        char lastModified[MAX_BUFFER];  // Buffer para armazenar a string formatada

        // Usando sprintf para formatar a data com zeros à esquerda
        sprintf(lastModified, "%04d-%02d-%02d %02d:%02d:%02d",
          tmstruct->tm_year + 1900,
          tmstruct->tm_mon + 1,
          tmstruct->tm_mday,
          tmstruct->tm_hour,
          tmstruct->tm_min,
          tmstruct->tm_sec);

        // Chama a função com a string formatada
        utilshdl.addMedia(String(entry.name()), entry.size(), String(lastModified));

        String filename = String(entry.name());
        int tam = filename.length();
        String ext =  filename.substring(tam-3, tam);
        if (ext == "wav" || ext == "mp3") {       
          returnText += "<tr align='left'><td>" + filename + "</td><td>" + utilshdl.humanReadableSize(entry.size()) + "</td><td>" + lastModified + "</td><td align='center'><img src='get-file?name="+ext+".webp' height='32' width='32'/></td><td align='center'><a onclick=deleteFile('sdcard','"+filename+"','"+token+"')><img src='get-file?name=delete.webp' height='32' width='32'/></a></td></tr>"; 
        }
        else  {
          returnText += "<tr align='left'><td>" + filename + "</td><td>" + utilshdl.humanReadableSize(entry.size()) + "</td><td>" + lastModified + "</td><td align='center'><img src='get-file?name="+filename+"' height='192' width='108'/></td><td align='center'><a onclick=deleteFile('sdcard','"+filename+"','"+token+"')><img src='get-file?name=delete.webp' height='32' width='32'/></a></td></tr>";  
        }
      }
    }    
    entry.close();
  }
  returnText += "</table>";
  return returnText;
}

String SdCardHandler::getSdcardTotal() {
  return utilshdl.uint64ToText(sdcard_total);
}

String SdCardHandler::getSdcardUsed() {
  return utilshdl.uint64ToText(sdcard_used);
}

bool SdCardHandler::getAllowedSdCardFiles(String filename) {  
  if (filename.endsWith(".png") || filename.endsWith(".jpg") || filename.endsWith(".bmp") || filename.endsWith(".gif")) return true;
  else if (filename.endsWith(".wav") || filename.endsWith(".mp3")) return true;
  return false;
}

bool SdCardHandler::getAllowedStorageFiles(String filename) {  
  if (filename.endsWith(".crt")) return true;
  return false;
}
