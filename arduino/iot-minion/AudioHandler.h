// AudioHandler.h
#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <Audio.h>
#include "Config.h"

//Pinos de conexão do ESP32-I2S e o módulo I2S/DAC CJMCU 1334
#define I2S_DOUT                     25
#define I2S_LRC                      26
#define I2S_BCLK                     27
//Volume
#define DEFAULT_VOLUME               20

enum AudioType {
  GOOGLE, // 0
  MARYTTS, // 1
  OPENAI,
};

class AudioHandler {
public:
  AudioHandler();
  void begin(String openIAKey);
  int getVolumeAudio();
  void setVolumeAudio(int volume);
  bool playSpeech(const char * mensagem, AudioType tipo=AudioType::GOOGLE);
  bool playMidia(const char * midia);
  void playRemoteMidia(const char * url);
  void loop();
private:  
  Audio audio; //objeto que representará o áudio
  String OPEN_IA_KEY;
};
#endif
