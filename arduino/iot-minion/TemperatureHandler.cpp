#include "TemperatureHandler.h"
/*
  Reading temperature or humidity takes about 250 milliseconds!
  Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
*/
TemperatureHandler::TemperatureHandler(): 
                                      previousMillis(0) {
  
}
void TemperatureHandler::begin() {
  dht = new DHT(TemperatureHumidity, DHT11);
}

float TemperatureHandler::getCelsius() {
  Serial.println("Buscando Celsius");  
  float c = dht->readTemperature(); // Le a temperatura como Celsius (padrao)
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if(isnan(c)) {
    Serial.println("Falha na leitura do celsius");
    return 0;
  }
  return c;
}

float TemperatureHandler::getFahrenheit() {
  Serial.println("Buscando fahrenheit");  
  float f = dht->readTemperature(true); // Le a temperatura como fahrenheit
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if(isnan(f)) {    
    Serial.println("Falha na leitura de fahrenheit");
    return 0;
  }
  return f;
}

float TemperatureHandler::getHumidity() {
  Serial.println("Buscando a umidade");  
  float h = dht->readHumidity(); // Le a umidade
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if(isnan(h)) {
    Serial.println("Falha na leitura da umidade");
    return 0;
  }
  return h;
}

float TemperatureHandler::getHeatIndexCelsius(){
  Serial.println("Buscando a temperatura da placa em celsius");
  // Le a temperatura como Celsius (padrao)
  float c = getCelsius();
  float f = getFahrenheit();
  float h = getHumidity();
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if (isnan(h) || isnan(c) || isnan(f)) {    
    Serial.println("Falha na leitura");
    return 0;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht->computeHeatIndex(c, h, false);  
  return hic;
}

float TemperatureHandler::getHeatIndexFahrenheit(){
  Serial.println("Buscando a temperatura da placa em fahrenheit");
  // Le a temperatura como Celsius (padrao)
  float c = dht->readTemperature();
  float f = dht->readTemperature(true);
  float h = dht->readHumidity();
  // Checa se qualquer leitura falha e saida mais cedo (para tentar de novo).
  if (isnan(h) || isnan(c) || isnan(f)) {    
    Serial.println("Falha na leitura");
    return 0;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht->computeHeatIndex(f, h);
  return hif;
}

void TemperatureHandler::getTemperatureData() {
  unsigned long currentMillis = millis();
  // Report every 1 minuto.
  if (currentMillis - previousMillis >= 60000) {
    previousMillis = currentMillis;
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    getHeatIndexCelsius();
  }
}
