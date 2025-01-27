
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
    String name;
    int id;
    int gpio;
    int status; // 1-TRUE / 0-FALSE
};
/******************************************************/
