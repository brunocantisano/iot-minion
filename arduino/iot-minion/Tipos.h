
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
    uint8_t id;
    uint8_t gpio;
    uint8_t status; // 1-TRUE / 0-FALSE
};
/******************************************************/
