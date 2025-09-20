// TemperatureHandler.h
#ifndef TEMPERATURE_HANDLER_H
#define TEMPERATURE_HANDLER_H

#include <DHT.h>

#define TemperatureHumidity          33
#define MAX_TEXT                     50

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

class TemperatureHandler {
public:
  TemperatureHandler();
  void begin();
  float getCelsius();
  float getFahrenheit();
  float getHumidity();
  float getHeatIndexCelsius();
  float getHeatIndexFahrenheit();
  void getTemperatureData();

private:
  DHT * dht;
  unsigned long previousMillis;
  char celsius [MAX_TEXT];
  char fahrenheit [MAX_TEXT];
  char humidity [MAX_TEXT];
  char heatIndexCelsius [MAX_TEXT];
  char heatIndexFahrenheit [MAX_TEXT];
};
#endif
