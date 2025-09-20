//Config.h
#ifndef CONFIG_H
#define CONFIG_H

// ====== CONFIG BÁSICA ======
#define SERIAL_PORT       115200
#ifndef HTTP_REST_PORT
  #define HTTP_REST_PORT  80
#endif


//Volume
#define DEFAULT_VOLUME               20
#define RelayHat                     13
#define RelayEyes                    14
#define RelayBlink                   15
#define RelayShake                   22
#define TemperatureHumidity          33
//
#define MAX_STRING_LENGTH            20000
#define MAX_PATH                     256

#define MAX_BUFFER                 128

#endif // CONFIG_H
