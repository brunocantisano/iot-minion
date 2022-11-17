uint64_t sdcard_total;
uint64_t sdcard_used;

String getMetrics() {
  String p = "";

  int sketch_size = ESP.getSketchSize();
  int flash_size =  ESP.getFreeSketchSpace();
  int available_size = flash_size - sketch_size;
  int heap_size = ESP.getFreeContStack();
  int free_heap = ESP.getFreeHeap();
  
  setMetric(&p, "esp8266_uptime", String(millis()));
  setMetric(&p, "esp8266_wifi_rssi", String(WiFi.RSSI()));
  setMetric(&p, "esp8266_sketch_size", String(sketch_size));
  setMetric(&p, "esp8266_flash_size", String(flash_size));
  setMetric(&p, "esp8266_available_size", String(available_size));
  setMetric(&p, "esp8266_heap_size", String(heap_size));
  setMetric(&p, "esp8266_free_heap", String(free_heap));
  setMetric(&p, "esp8266_boot_counter", String(getBootCounter()));  
  setMetric(&p, "esp8266_celsius", String(iCelsius));
  setMetric(&p, "esp8266_fahrenheit", String(iFahrenheit));
  setMetric(&p, "esp8266_humidity", String(iHumidity));
  setMetric(&p, "esp8266_heat_celsius", String(iHeatIndexCelsius));
  setMetric(&p, "esp8266_heat_fahrenheit", String(iHeatIndexFahrenheit));
  setMetric(&p, "esp8266_eyes", String(readSensorStatus(RelayEyes)));
  setMetric(&p, "esp8266_hat", String(readSensorStatus(RelayHat)));
  setMetric(&p, "esp8266_blink", String(readSensorStatus(RelayBlink)));
  setMetric(&p, "esp8266_shake", String(readSensorStatus(RelayShake)));
  setMetric(&p, "esp8266_volume", String(getVolumeAudio()));
  setMetric(&p, "esp8266_sdcard_total", String(uint64ToText(sdcard_total)));
  setMetric(&p, "esp8266_sdcard_used", String(uint64ToText(sdcard_used)));

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

   # esp8266_uptime
   # TYPE esp8266_uptime gauge
   esp8266_uptime 23899

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
  //audio.setVolume(volume);
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

  String feedName = "temperature";
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  
  // Le a temperatura como Celsius (padrao)
  float c = dht.readTemperature();
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(c, h, false);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);

  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if (isnan(h) || isnan(c) || isnan(f) || isnan(hic) || isnan(hif)) {
    c=0;
    f=0;
    h=0;
    hic=0;
    hif=0;
  }
  #ifdef DEBUG
//    Serial.printf("Celsius: %f\n",c);
//    Serial.printf("Fahrenheit: %f\n",f);
//    Serial.printf("Humidity: %f\n",h);
//    Serial.printf("HeatIndexCelsius: %f\n",hic);
//    Serial.printf("HeatIndexFahrenheit: %f\n",hif);
  #endif
  
  char celsius [MAX_FLOAT];
  snprintf (celsius, MAX_FLOAT, "%d", (int)round(c));
  char fahrenheit [MAX_FLOAT];
  snprintf (fahrenheit, MAX_FLOAT, "%d", (int)round(f));
  char humidity [MAX_FLOAT];
  snprintf (humidity, MAX_FLOAT, "%d", (int)round(h));

  char heatIndexCelsius [MAX_FLOAT];
  snprintf (heatIndexCelsius, MAX_FLOAT, "%d", (int)round(hic));
  
  char heatIndexFahrenheit [MAX_FLOAT];
  snprintf (heatIndexFahrenheit, MAX_FLOAT, "%d", (int)round(hif));

  iCelsius = atoi(celsius);
  iFahrenheit = atoi(fahrenheit);
  iHumidity = atoi(humidity);
  iHeatIndexCelsius = atoi(heatIndexCelsius);
  iHeatIndexFahrenheit = atoi(heatIndexFahrenheit);

  #ifdef DEBUG
//    Serial.printf("Celsius: %d\n",iCelsius);
//    Serial.printf("Fahrenheit: %d\n",iFahrenheit);
//    Serial.printf("Humidity: %d\n",iHumidity);
//    Serial.printf("HeatIndexCelsius: %d\n",iHeatIndexCelsius);
//    Serial.printf("HeatIndexFahrenheit: %d\n",iHeatIndexFahrenheit);
  #endif
}
