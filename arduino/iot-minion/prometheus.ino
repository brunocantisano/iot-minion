uint64_t sdcard_total;
uint64_t sdcard_used;
  
String getMetrics() {
  String p = "";

  int sketch_size = ESP.getSketchSize();
  int flash_size =  ESP.getFreeSketchSpace();
  int available_size = flash_size - sketch_size;
  int heap_size = ESP.getHeapSize();
  int free_heap = ESP.getFreeHeap();
  int psram_size = ESP.getPsramSize();
  int free_psram_size = ESP.getFreePsram();
  
  float temperature = ((temprature_sens_read() - 32) / 1.8);
 
  setMetric(&p, "esp32_uptime", String(millis()));
  setMetric(&p, "esp32_wifi_rssi", String(WiFi.RSSI()));
  setMetric(&p, "esp32_heap_size", String(ESP.getHeapSize()));
  setMetric(&p, "esp32_free_heap", String(ESP.getFreeHeap()));
  setMetric(&p, "esp32_sketch_size", String(sketch_size));
  setMetric(&p, "esp32_flash_size", String(flash_size));
  setMetric(&p, "esp32_available_size", String(available_size));
  setMetric(&p, "esp32_heap_size", String(heap_size));
  setMetric(&p, "esp32_free_heap", String(free_heap));
  setMetric(&p, "esp32_psram_size", String(psram_size));
  setMetric(&p, "esp32_free_psram_size", String(free_psram_size));
  setMetric(&p, "esp32_temperature", String(temperature));
  setMetric(&p, "esp32_boot_counter", String(getBootCounter()));  
  setMetric(&p, "esp32_celsius", strCelsius);
  setMetric(&p, "esp32_fahrenheit", strFahrenheit);
  setMetric(&p, "esp32_humidity", strHumidity);
  setMetric(&p, "esp32_eyes", String(readSensorStatus(RelayEyes)));
  setMetric(&p, "esp32_hat", String(readSensorStatus(RelayHat)));
  setMetric(&p, "esp32_blink", String(readSensorStatus(RelayBlink)));
  setMetric(&p, "esp32_shake", String(readSensorStatus(RelayShake)));
  setMetric(&p, "esp32_volume", String(getVolumeAudio()));
  setMetric(&p, "esp32_sdcard_total", String(uint64ToText(sdcard_total)));
  setMetric(&p, "esp32_sdcard_used", String(uint64ToText(sdcard_used)));
  return p;
}

const char* uint64ToText(uint64_t input) {
  String result = "";
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
  return result.c_str();
}

/**
   Layout

   # esp32_uptime
   # TYPE esp32_uptime gauge
   esp32_uptime 23899

*/
void setMetric(String *p, String metric, String value) {
  *p += "# " + metric + "\n";
  *p += "# TYPE " + metric + " gauge\n";
  *p += "" + metric + " ";
  *p += value;
  *p += "\n";
}

int getVolumeAudio() {
  return preferences.getInt("volume");
}

void setVolumeAudio(int volume) { 
  preferences.putInt("volume", volume);
  //Ajusta o volume de saída
  audio.setVolume(volume);
}

int getBootCounter() {
  return preferences.getInt("boot");
}

void incrementBootCounter() {
  int boot = getBootCounter();
  preferences.putInt("boot", (boot + 1));
}

void setupStorage() {
  preferences.begin("storage", false);
}

void closeStorage() {
  preferences.end();
}

String getTemperatureHumidity(temperature_dht tipo) {
  float valor = 0.0;
  String feedName = "temperature";
  // Leituras do sensor podem demorar até 2 segundos (o sensor DHT11 é muito lento)
  switch(tipo) {
    case celsius:
      // Le a temperatura como Celsius (padrao)
      valor = dht.readTemperature();
      delay(2000);
      break;
    case fahrenheit:
      valor = dht.readTemperature(true);
      delay(2000);
      break;
    case humidity:
      valor = dht.readHumidity();
      feedName = "humidity";
      delay(2000);
      break;            
  }  
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if (isnan(valor)) {    
    #ifdef DEBUG
      Serial.println("Falha na leitura do tipo: "+String(tipo));
    #endif
    valor = 0.0;
  }
  char buffer [MAX_PATH];
  snprintf ( buffer, MAX_PATH, "%.1f", valor );   
  // publish
  client.publish((String(MQTT_USERNAME)+String("/feeds/")+feedName).c_str(), buffer);
  return String(buffer);
}
