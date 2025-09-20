#include "MqttHandler.h"

MqttHandler::MqttHandler(const String& aio_username,
                         const String& aio_key,
                         const String& ssid,
                         const String& passwd) {

  if (aio_username.isEmpty() || aio_key.isEmpty()) {
    Serial.println("Erro: credenciais AIO não configuradas");
    conectado = false;
    return;
  }
  io = new AdafruitIO_WiFi(aio_username.c_str(),
                           aio_key.c_str(),
                           ssid.c_str(),
                           passwd.c_str());

  io->connect();
  
  // espera até conectar (timeout 30s)
  unsigned long start = millis();
  while (io->status() < AIO_CONNECTED && millis() - start < 30000) {
    io->run();           // mantém alive
    Serial.println(".");
    delay(500);
  }

  if (io->status() >= AIO_CONNECTED) {
    conectado = true;
    Serial.println("\nConectado ao Adafruit IO!");    
  } else {
    Serial.println("Erro: não conseguiu conectar ao Adafruit IO.");
    conectado = false;
  }
}

MqttHandler::~MqttHandler() {
  delete io;  // Free the allocated memory
}

void MqttHandler::update() {
  // se desconectado, não chama run() em nullptr
  if (conectado) {
    io->run();
    Serial.println("Update Adafruit IO!");
  }
}

bool MqttHandler::isConnected() const {
  return conectado;
}

void MqttHandler::setFeed(const String& name, const String& value) { 
  feed = io->feed(name.c_str());
  if (feed) feed->save(value);
  Serial.println("Salva informações sobre feed:" + name + " no Adafruit IO!");
}

bool MqttHandler::createAdafruitFeed(const String& feedName, const String& aioKey) const {
  return true;
}

bool MqttHandler::adafruitFeedExists(const String& feedName,
                                     const String& username,
                                     const String& aioKey) const {
  HTTPClient http;
  String url = "https://io.adafruit.com/api/v2/" + username + "/feeds/" + feedName;

  http.begin(url);
  http.addHeader("X-AIO-Key", aioKey);
  http.setTimeout(5000);

  int code = http.GET();
  http.end();

  if (code == HTTP_OK)        return true;
  if (code == HTTP_NOT_FOUND) return false;

  Serial.printf("Erro %d ao verificar feed %s", code, feedName.c_str());
  return false;
}