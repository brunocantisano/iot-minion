#include "AudioHandler.h"

AudioHandler::AudioHandler() {

}

void AudioHandler::begin(String openIAKey) {
  OPEN_IA_KEY = openIAKey;
  //Ajusta os pinos de conexão I2S para saída de áudio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  //Ajusta o volume de saída.
  audio.setVolume(DEFAULT_VOLUME); // 0...21
}
int AudioHandler::getVolumeAudio() {
  return audio.getVolume();
}

void AudioHandler::setVolumeAudio(int volume) { 
  //Ajusta o volume de saída
  audio.setVolume(volume);
}

bool AudioHandler::playSpeech(const char * mensagem, AudioType tipo)
{
  bool retorno = false;
  static const char instructions[] PROGMEM = "Voice: Gruff, fast-talking, and a little worn-out, like a New York cabbie who's seen it all but still keeps things moving.\n\n"
                            "Tone: Slightly exasperated but still functional, with a mix of sarcasm and no-nonsense efficiency.\n\n"
                            "Dialect: Strong New York accent, with dropped \"r\"s, sharp consonants, and classic phrases like whaddaa and lemme guess.\n\n"
                            "Pronunciation: Quick and clipped, with a rhythm that mimics the natural hustle of a busy city conversation.\n\n"
                            "Features: Uses informal, straight-to-the-point language, throws in some dry humor, and keeps the energy just on the edge of impatience but still";
  switch(tipo) {
    case GOOGLE:
      //Para executar uma síntese de voz
      audio.connecttospeech(mensagem, "pt");
      retorno = true;
      break;
    case MARYTTS:
      // voice speed: 74%
      // pitch: 52%
      //audio.connecttomarytts(mensagem, "it", "istc-lucia-hsmm");
      Serial.println("Foi removido o uso da Marytts no código recente");
      retorno = true;
      break;   
    case OPENAI:
      // Open IA Speech
      audio.openai_speech(OPEN_IA_KEY, "tts-1", mensagem, instructions, "fable", "mp3", "3.0"); //speed vai de 1.0 a 4.0
      retorno = true;
      break;
    default:
      Serial.println("Não encontrei o tipo de audio a ser executado!");
  }
  return retorno;
}

bool AudioHandler::playMidia(const char * midia)
{
  char filenameMidia[strlen(midia)+1];
  filenameMidia[0]='/';
  filenameMidia[1]='\0';
  strcat(filenameMidia, midia);
  #ifdef DEBUG
    Serial.printf("Arquivo a tocar: %s\n",filenameMidia);
  #endif  
  // exemplo: "/1.mp3"
  bool exists = SD.exists(filenameMidia);
  if(exists){
    audio.connecttoFS(SD, filenameMidia);
    return true;
  }
  return false;
}

void AudioHandler::playRemoteMidia(const char * url)
{ 
  audio.connecttohost(url); //  128k mp3
}

void AudioHandler::loop() {
  audio.loop(); 
}
