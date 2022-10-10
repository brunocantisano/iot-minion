#ifndef Tipos_h
#define Tipos_h

/************* lista de aplicacoes jenkins *************/
class Application {
  public:
    String name;
    String language;
    String description;    
};
/******************************************************/
/************** lista de mídias no sdcard *************/
class Media {
  public:
    String name;
    int size;    
    String lastModified;
};
/******************************************************/
/******************* lista de sensores ****************/
class ArduinoSensorPort {
  public:
    char* name;
    byte id;
    byte gpio;
    byte status; // 1-TRUE / 0-FALSE
};
/******************************************************/
typedef enum {
  celsius,
  fahrenheit,
  humidity
} temperature_dht;

#endif
