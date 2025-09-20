// UtilsHandler.cpp
#include "UtilsHandler.h"

UtilsHandler::UtilsHandler() {
  
}

void UtilsHandler::setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  delay(3000);
}

String UtilsHandler::IpAddress2String(const IPAddress& ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

String UtilsHandler::getData(uint8_t *data, size_t len) {
  return String((const char*)data).substring(0, len);
}

String UtilsHandler::getDataHora() {
    // Busca tempo no NTP. Padrao de data: ISO-8601
    time_t nowSecs = time(nullptr);
    struct tm timeinfo;
    char buffer[80];
    while (nowSecs < 8 * 3600 * 2) {
      delay(500);
      nowSecs = time(nullptr);
    }
    gmtime_r(&nowSecs, &timeinfo);
    // ISO 8601: 2021-10-04T14:12:26+00:00
    strftime (buffer,80,"%FT%T%z",&timeinfo);
    return String(buffer);
}

String UtilsHandler::getDateTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "unknown";
  }

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);
  return String(buffer);
}

String UtilsHandler::uint64ToText(uint64_t input) {
  String result = String(input);
  uint8_t base = 10;

  do {
    char c = input % base;
    input /= base;

    if (c < 10)
      c +='0';
    else
      c += 'A' - 10;
    result = c + result;
  } while (input);
  return result;
}

String UtilsHandler::getDoubleAsString(double valor) {
  char mensagemTxt[100];  // Buffer para armazenar a string formatada
  snprintf(mensagemTxt, sizeof(mensagemTxt), "%.6f", valor);
  return String(mensagemTxt);
}

String UtilsHandler::getMimeType(const String& filename) {
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".gif")) return "image/gif";
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  if (filename.endsWith(".ico")) return "image/x-icon";
  if (filename.endsWith(".svg")) return "image/svg+xml";
  if (filename.endsWith(".json")) return "application/json";
  if (filename.endsWith(".txt")) return "text/plain";
  if (filename.endsWith(".woff2")) return "font/woff2";
  return "application/octet-stream";  // padrão seguro para arquivos desconhecidos
}

String UtilsHandler::sanitizeFilename(const String& filename) {
  String sanitized = filename;

  // Remover tentativas explícitas de path traversal
  sanitized.replace("..", "");
  sanitized.replace("/", "");
  sanitized.replace("\\", "");

  // Substituir espaços por underscores
  sanitized.replace(" ", "_");

  // Remover caracteres considerados inválidos para nomes de arquivos
  const char* invalidChars = "<>:\"|?*";
  for (size_t i = 0; i < strlen(invalidChars); i++) {
    sanitized.replace(String(invalidChars[i]), "");
  }

  // Substituir caracteres não permitidos (qualquer coisa que não seja letra, número, '_', '-', '.')
  for (size_t i = 0; i < sanitized.length(); i++) {
    char c = sanitized.charAt(i);
    if (!isalnum(c) && c != '_' && c != '-' && c != '.') {
      sanitized.setCharAt(i, '_');
    }
  }

  // Garantir que o nome de arquivo não fique vazio
  if (sanitized.length() == 0) {
    sanitized = "default.txt";
  }

  return sanitized;
}

// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String UtilsHandler::humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

void UtilsHandler::addApplication(String name, String language, String description) {
  if(searchList(name, language)== -1) {
    Application *app = new Application();
    app->name = name;
    app->language = language;
    app->description = description;
    // Adiciona a aplicação na lista
    applicationListaEncadeada.add(app);
  }
}

void UtilsHandler::removeApplication(int index) {
  applicationListaEncadeada.remove(index);
}

void UtilsHandler::addMedia(String name, int size, String lastModified) {
  Media *media = new Media();
  media->name = name;
  media->size = size;
  media->lastModified=lastModified;

  // Adiciona a aplicação na lista
  mediaListaEncadeada.add(media);
}

String UtilsHandler::listApplicationJson() {
  String JSONmessage;
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"id\": "+String(i+1)+",\"name\": \""+app->name+"\",\"language\": \""+app->language+"\",\"description\": \""+app->description+"\"}"+',';
  }
  return JSONmessage;
}

String UtilsHandler::listMediaJson() {
  String JSONmessage;
  Media *media;    
  for(int i = 0; i < mediaListaEncadeada.size(); i++){
    // Obtem a midia da lista de midias
    media = mediaListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(media->name)+"\",\"size\": \""+String(media->size)+"\",\"lastModified\": \""+String(media->lastModified)+"\"},";
  }
  return JSONmessage;
}

String UtilsHandler::listSensorJson(){
  String JSONmessage;
  ArduinoSensorPort *arduinoSensorPort;    
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    arduinoSensorPort = sensorListaEncadeada.get(i);
    JSONmessage += "{\"id\": \""+String(arduinoSensorPort->id)+"\",\"gpio\": \""+String(arduinoSensorPort->gpio)+"\",\"name\": \""+arduinoSensorPort->name+"\",\"status\": \""+String(arduinoSensorPort->status)+"\"},";
  }
  return JSONmessage;
}

bool UtilsHandler::loadSensorList(){
  // 1 → RelayEyes | 2 → RelayHat | 3 → RelayBlink | 4 → RelayShake | 5 → TemperatureHumidity
  if(!addSensor(1, RelayEyes, "Eyes")) return false;
  if(!addSensor(2, RelayHat, "Hat")) return false;
  if(!addSensor(3, RelayBlink, "Blink")) return false;
  if(!addSensor(4, RelayShake, "Shake")) return false; 
  if(!addSensor(5, TemperatureHumidity, "TemperatureHumidity")) return false;
  return true;  
}

String UtilsHandler::saveApplicationList() {
  Application *app;
  String JSONmessage;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    JSONmessage += "{\"name\": \""+String(app->name)+"\",\"language\": \""+String(app->language)+"\",\"description\": \""+String(app->description)+"\"}"+',';
  }
  JSONmessage = '['+JSONmessage.substring(0, JSONmessage.length()-1)+']';

  return JSONmessage;
}

bool UtilsHandler::addSensor(int id, int gpio, String name) {
  ArduinoSensorPort *p = new ArduinoSensorPort();
  pinMode(gpio, INPUT_PULLUP);
  delay(10); // estabiliza após configurar o pino

  p->id = id;
  p->gpio = gpio;
  p->name = name;
  p->status = readSensorStable(gpio); // estado inicial sem “fantasma”
  sensorListaEncadeada.add(p);
  return true;
}

ArduinoSensorPort * UtilsHandler::searchListSensor(int gpio) {
  for(int i = 0; i < sensorListaEncadeada.size(); i++){
    ArduinoSensorPort *p = sensorListaEncadeada.get(i);
    if (gpio == p->gpio) return p;
  }
  return nullptr;
}

int UtilsHandler::searchList(String name, String language) {
  Application *app;
  for(int i = 0; i < applicationListaEncadeada.size(); i++){
    // Obtem a aplicação da lista
    app = applicationListaEncadeada.get(i);
    if (name == app->name && language==app->language) {
      return i;
    }
  }
  return -1;
}

// Lê várias vezes e decide por maioria: robusto contra ruído/boot
bool UtilsHandler::readSensorStable(int pin, uint8_t samples, uint16_t gap_ms) {
  uint8_t trues = 0;
  for (uint8_t i = 0; i < samples; i++) {
    int v = digitalRead(pin);
    bool on = (v == LOW);
    if (on) trues++;
    delay(gap_ms);
  }
  return (trues > samples/2);
}

inline bool UtilsHandler::is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

String UtilsHandler::base64_decode(const String &encoded_string) {
    const String base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    auto is_base64 = [](char c) {
        return isalnum(c) || (c == '+') || (c == '/');
    };

    int in_len = encoded_string.length();
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    String ret = "";

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.indexOf(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                ret += (char)char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.indexOf(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < (i - 1); j++) ret += (char)char_array_3[j];
    }

    return ret;
}
