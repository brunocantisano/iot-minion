import React from 'react';
import axios from 'axios';
import useSpeechToText from 'react-hook-speech-to-text';
import './styles.scss';

const microphone = 'https://i.ibb.co/23mMRr9J/microphone.png';
const micrecording = 'https://i.ibb.co/PvStwx4d/mic-recording.png';

const SpeechMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/ask':'';
  const {
    error,
    interimResult,
    isRecording,
    startSpeechToText,
    stopSpeechToText,
  } = useSpeechToText({
    continuous: true,
    useLegacyResults: false
  });

  if (error) return <p>Web Speech API is not available in this browser 🤷‍</p>;

  async function handleAsk() {
    if(!isRecording) return;
    try {
      if(rota !== '') {
        const response = await axios.post(rota,
          {
            "mensagem": interimResult
          },
          {
            headers: {
              'Content-Type': 'application/json',
              'Accept': 'application/json',
              'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
            }
          });
          console.log('👉 Returned data:', response);
      }
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  return (
    <div className="speech-minion">
      <button
        className={isRecording ? "speech-minion__mic speech-minion__mic--recording" : "speech-minion__mic"}
        onClick={handleAsk}
        onClickCapture={isRecording ? stopSpeechToText : startSpeechToText}
      >
        <img src={isRecording ? micrecording : microphone} width="24" height="24" alt="ask minion"/>
      </button>
      <h6 className={isRecording ? "blink speech-minion__status" : "notblink speech-minion__status"}>{isRecording ? "gravando" : ""}</h6>
      {interimResult && <p className="speech-minion__interim">{interimResult}</p>}
    </div>
  );
}

export default SpeechMinion;