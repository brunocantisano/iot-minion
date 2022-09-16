/*
  SD card test for esp32
 
  This example shows how use the utility libraries
 
  The circuit:
    SD card attached to SPI bus as follows:
        SD_CS = 5;
        MOSI  = 23;
        MISO  = 19;
        SCK   = 18;
 
 
   by Mischianti Renzo <https://www.mischianti.org>
  
   https://www.mischianti.org
*/
bool loadSdCardMedias() {
  Serial.print(F("\nInitializing SD card..."));

  if (!SD.begin(SD_CS)) {
    #ifdef DEBUG
      Serial.println(F("inicialização falhou. Lembre-se de checar:"));
      Serial.println(F("* o cartão está inserido?"));
      Serial.println(F("* as conexões dos fios estão corretas?"));
      Serial.println(F("* você mudou o pino chipSelect para corresponder ao seu chield ou módulo?"));
    #endif
    return false;
  } else {
    #ifdef DEBUG
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
    #endif
    sdcard_total = SD.totalBytes() / (1024 * 1024);
    sdcard_used =  SD.usedBytes() / (1024 * 1024);
    
    #ifdef DEBUG
      Serial.println(F("Listando arquivos armazenados no cartão SD"));
    #endif
    File dir =  SD.open("/");
    listFilesSD(dir, 0);
    dir.close();
    return true;
  }
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  String returnText = "";
  Serial.println(F("Listando arquivos armazenados no storage"));
  File root = LITTLEFS.open("/");
  File foundfile = root.openNextFile();
  if (ishtml) {
    returnText += "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th></tr>";
  }
  while (foundfile) {
    if (ishtml) {
      int tam = strlen(foundfile.name());
      char temp[tam];
      strncpy(temp,foundfile.name(),tam);
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td></tr>";
    } else {
      returnText += "Arquivo: " + String(foundfile.name()) + "\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  return returnText;
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFilesSD(File dir, int numTabs) {
  String returnText = "<table><tr><th align='left'>Nome</th><th align='left'>Tamanho</th><th align='left'>Modificação</th></tr>";
  String lastModified;
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    } else {
      // Serial.print(entry.name());
      if (entry.isDirectory()) {
        listFilesSD(entry, numTabs + 1);
      } else {
        // files have sizes, directories do not
        time_t lw = entry.getLastWrite();
        struct tm * tmstruct = localtime(&lw);
        lastModified = String((tmstruct->tm_year) + 1900,2)+"-"+String((tmstruct->tm_mon) + 1,2)+"-"+String(tmstruct->tm_mday,2)+" "+String(tmstruct->tm_hour,2)+":"+String(tmstruct->tm_min,2)+":"+String(tmstruct->tm_sec,2);
        addMedia(String(entry.name()), entry.size(), lastModified);
        returnText += "<tr align='left'><td>" + String(entry.name()) + "</td><td>" + humanReadableSize(entry.size()) + "</td><td>" + lastModified + "</td></tr>";
      }
    }    
    entry.close();
  }
  returnText += "</table>";
  return returnText;
}
