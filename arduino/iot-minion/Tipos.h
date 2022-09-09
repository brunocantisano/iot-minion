
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
    char* name;
    int size;
    char* lastModified;
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
