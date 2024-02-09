import React from 'react';
import axios from 'axios';
import useSpeechToText from 'react-hook-speech-to-text';
import './styles.scss';
import microphone from '../../assets/microphone.png';
import micrecording from '../../assets/mic-recording.png';

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
    <div>
      <button onClick={isRecording ? stopSpeechToText : startSpeechToText}>
        <img src={isRecording ? micrecording : microphone} width="32" height="32" alt="ask minion"/>
      </button>
      <h6 className={isRecording?"blink":"notblink"}>{isRecording?"gravando":""}</h6>
        {/* {results.map((result) => (
          <li key={result.timestamp}>{result.transcript}</li>
        ))} */}
        {<p>{interimResult}</p>}
    </div>
  );
}

export default SpeechMinion;