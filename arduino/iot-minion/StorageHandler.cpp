// StorageHandler.cpp
#include "StorageHandler.h"

StorageHandler::StorageHandler() {
  
}

bool StorageHandler::begin() {
  return FS_begin(); // unificado
}

void StorageHandler::listDir(fs::FS &fs, const char* dirname, uint8_t levels) {
#if defined(ESP8266)
  Dir dir = LittleFS.openDir(dirname);
  while (dir.next()) {
    String name = dir.fileName();
    size_t size = dir.fileSize();
    Serial.printf("  FILE: %s  SIZE: %u\n", name.c_str(), (unsigned)size);

    // No ESP8266, detectar diretório é limitado; se precisar recursão,
    // você pode tentar abrir com trailing '/' e ver se lista.
    if (levels && name.endsWith("/")) {
      String next = String(dirname) + "/" + name;
      listDir(fs, next.c_str(), levels - 1);
    }
  }
#else
  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) return;
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.printf("  DIR : %s\n", file.name());
      if (levels) listDir(fs, file.name(), levels - 1);
    } else {
      Serial.printf("  FILE: %s  SIZE: %u\n", file.name(), (unsigned)file.size());
    }
    file = root.openNextFile();
  }
#endif
}

void StorageHandler::createDir(fs::FS &fs, const char * path) {
    Serial.println("Creating Dir: " + String(path));
    Serial.println(fs.mkdir(path) ? "Dir created" : "mkdir failed");
}

void StorageHandler::removeDir(fs::FS &fs, const char * path) {
    Serial.println("Removing Dir: " + String(path));
    Serial.println(fs.rmdir(path) ? "Dir removed" : "rmdir failed");
}

bool StorageHandler::readFile(fs::FS &fs, const char* path, String& out) {
  File file = fs.open(path, FS_OPEN_READ);
  if (!file) {
    Serial.printf("OPEN FAIL: %s\n", path);
    return false;
  }
  out = "";
  while (file.available()) {
      int c = file.read();
      if (c < 0) break;
      out += (char)c;
  }
  //Serial.printf("DEBUG: conteudo lido = [%s]\n", out.c_str());
  //Serial.printf("Tamanho arquivo: %u\n", file.size());
  file.close();
  //Serial.printf("READ OK %s (%u bytes)\n", path, out.length());
  out=String(out);
  return true;
}

String StorageHandler::base64EncodeFile(fs::FS &fs, const char * path) {
    Serial.println("Reading file: " + String(path));

    const size_t bufferSize = 512;  // segura para ambos ESP32 e ESP8266
    uint8_t buffer[bufferSize];
  
    File file = fs.open(path, FS_OPEN_READ);
    if (!file) {
      Serial.println("Erro ao abrir o arquivo.");
      return "";
    }
  
    String result = "";
    while (file.available()) {
      size_t bytesRead = file.read(buffer, bufferSize);
      result += base64::encode(buffer, bytesRead);
    }
  
    file.close();
    return result;
}

void StorageHandler::writeFile(fs::FS &fs, const char* path, const char* message) {
  File file = fs.open(path, FS_OPEN_WRITE);
  if (!file) return;
  file.print(message);
  file.close();
}

void StorageHandler::appendFile(fs::FS &fs, const char* path, const char* message) {
  File file = fs.open(path, FS_OPEN_APPEND);
  if (!file) {
    // fallback: cria
    file = fs.open(path, FS_OPEN_WRITE);
    if (!file) return;
  }
  file.print(message);
  file.close();
}

void StorageHandler::renameFile(fs::FS &fs, const char * path1, const char * path2) {
    Serial.println("Renaming file "+String(path1)+" to "+String(path2));
    Serial.println(fs.rename(path1, path2) ? "- file renamed" : "- rename failed");
}

bool StorageHandler::deleteFile(fs::FS &fs, const char * path) {
  Serial.println("Deleting file: "+String(path));
  if(!fs.remove(path)){
    Serial.println("- delete failed");
    return false;
  }
  Serial.println("- file deleted");
  return true;
}

bool StorageHandler::fileExists(fs::FS &fs, const char * path) {
  Serial.println("Checking file: " + String(path));
  if (!fs.exists(path)) {
    Serial.println("- file does not exist");
    return false;
  }
  Serial.println("- file exists");
  return true;
}

void StorageHandler::testFileIO(fs::FS &fs, const char* path) {
  // escrita grande
  File file = fs.open(path, FS_OPEN_WRITE);
  static const size_t BUFSZ = 512;
  if (file) {
    uint8_t buf[BUFSZ];
    for (size_t i = 0; i < BUFSZ; ++i) buf[i] = i & 0xFF;
    for (int i = 0; i < 2048; ++i) file.write(buf, BUFSZ);
    file.close();
  }

  // leitura
  file = fs.open(path, FS_OPEN_READ);
  if (file) {
    while (file.available()) { (void)file.read(); }
    file.close();
  }
}

bool StorageHandler::createNewLogFile() {
  char name[32];
  snprintf(name, sizeof(name), "/log_%lu.txt", (unsigned long)millis());
  currentLogFile = name;
  File f = LittleFS.open(currentLogFile, FS_OPEN_WRITE);
  
  if (!f) return false;
  f.println("=== NOVO LOG ===");
  f.close();
  return true;
}

void StorageHandler::logMessage(String msg) {
  if (currentLogFile.isEmpty()) createNewLogFile();
  File f = LittleFS.open(currentLogFile, FS_OPEN_APPEND);
  if (!f) return;
  f.println(msg);
  f.close();
}

void StorageHandler::manageLogFiles() {
#if defined(ESP8266)
  // Lista todos e, por exemplo, mantém no máx. 10 arquivos
  struct Entry { String name; size_t size; };
  std::vector<Entry> files;

  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    if (dir.fileName().startsWith("log_")) {
      files.push_back({dir.fileName(), dir.fileSize()});
    }
  }
  // ordenar por nome ou timestamp embutido
  std::sort(files.begin(), files.end(),
            [](const Entry&a, const Entry&b){ return a.name < b.name; });
  while (files.size() > 10) {
    LittleFS.remove(files.front().name);
    files.erase(files.begin());
  }
#else
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) return;

  // Mesma lógica no ESP32 usando openNextFile()
  struct Entry { String name; size_t size; };
  std::vector<Entry> files;

  File f = root.openNextFile();
  while (f) {
    if (!f.isDirectory()) {
      String nm = f.name();
      if (nm.startsWith("/log_")) files.push_back({nm, (size_t)f.size()});
    }
    f = root.openNextFile();
  }
  std::sort(files.begin(), files.end(),
            [](const Entry&a, const Entry&b){ return a.name < b.name; });
  while (files.size() > 10) {
    LittleFS.remove(files.front().name);
    files.erase(files.begin());
  }
#endif
}

String StorageHandler::getCurrentLogFile() {
  return currentLogFile;
}

// list all of the files
String StorageHandler::listFiles() {
  String out = "";
  Serial.println(F("Listando arquivos armazenados no storage"));

#if defined(ESP8266)
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    String filename = dir.fileName();
    size_t fsize = (size_t)dir.fileSize();

    // Considere logs como "log_*.txt"
    bool isLog = filename.startsWith("log_") && filename.endsWith(".txt");

    if (isLog) {
      String content;
      // Garante prefixo '/'
      String full = filename.startsWith("/") ? filename : ("/" + filename);
      if (readFile(LittleFS, full.c_str(), content)) {
        content.replace("\r\n", "<br>");
        out += "{ nome: \"" + filename + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"" + content + "\"},";
      } else {
        out += "{ nome: \"" + filename + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"Falha ao ler\"},";
      }
    } else {
      out += "{ nome: \"" + filename + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"Não é possível visualizar este conteúdo\"},";
    }
  }

#else
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    return "[];";
  }

  File f = root.openNextFile();
  while (f) {
    String filename = String(f.name());
    size_t fsize = (size_t)f.size();

    bool isLog = filename.startsWith("/log_") && filename.endsWith(".txt");

    if (!f.isDirectory()) {
      if (isLog) {
        String content;
        // filename já tem '/'
        if (readFile(LittleFS, filename.c_str(), content)) {
          content.replace("\r\n", "<br>");
          out += "{ nome: \"" + filename.substring(1) + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"" + content + "\"},";
        } else {
          out += "{ nome: \"" + filename.substring(1) + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"Falha ao ler\"},";
        }
      } else {
        out += "{ nome: \"" + filename.substring(1) + "\", tamanho: \"" + utilshdl.humanReadableSize(fsize) + "\", conteudo: \"Não é possível visualizar este conteúdo\"},";
      }
    }

    f = root.openNextFile();
  }
#endif

  if (out.length() > 0) {
    out.remove(out.length() - 1);     // tira a vírgula final
    out = "[" + out + "];";
  } else {
    out = "[];";
  }
  return out;
}
