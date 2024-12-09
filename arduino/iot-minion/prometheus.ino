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
  setMetric(&p, "esp32_heat_celsius", strHeatIndexCelsius);
  setMetric(&p, "esp32_heat_fahrenheit", strHeatIndexFahrenheit);  
  setMetric(&p, "esp32_eyes", readSensorStatus(RelayEyes));
  setMetric(&p, "esp32_hat", readSensorStatus(RelayHat));
  setMetric(&p, "esp32_blink", readSensorStatus(RelayBlink));
  setMetric(&p, "esp32_shake", readSensorStatus(RelayShake));
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
  return audio.getVolume();
}

void setVolumeAudio(int volume) { 
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

void getTemperatureHumidity() {
  
  Serial.println("Buscando a temperatura e a umidade");
  
  String feedName = "temperature";
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  // Le a temperatura como Celsius (padrao)
  float c = dht.readTemperature();
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();

  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if (isnan(h) || isnan(c) || isnan(f)) {    
    #ifdef DEBUG
      Serial.println("Falha na leitura");
    #endif
    timeSinceLastRead = 0;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(c, h, false);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);

  char celsius [MAX_PATH];
  snprintf (celsius, MAX_PATH, "%.1f", c);
  char fahrenheit [MAX_PATH];
  snprintf (fahrenheit, MAX_PATH, "%.1f", f);
  char humidity [MAX_PATH];
  snprintf (humidity, MAX_PATH, "%.1f", h);

  char heatIndexCelsius [MAX_PATH];
  snprintf (heatIndexCelsius, MAX_PATH, "%.1f", hic);

  char heatIndexFahrenheit [MAX_PATH];
  snprintf (heatIndexFahrenheit, MAX_PATH, "%.1f", hif);

  strCelsius = String(celsius);
  strFahrenheit = String(fahrenheit);
  strHumidity = String(humidity);
  strHeatIndexCelsius = String(heatIndexCelsius);
  strHeatIndexFahrenheit = String(heatIndexFahrenheit);

  timeSinceLastRead = 0;

  // publish
  client.publish((String(MQTT_USERNAME)+String("/feeds/temperature")).c_str(), celsius);
  client.publish((String(MQTT_USERNAME)+String("/feeds/humidity")).c_str(), humidity);
}
