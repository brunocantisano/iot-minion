import React from 'react';
import useSpeechToText from 'react-hook-speech-to-text';
import './styles.scss';
import microphone from '../../assets/microphone.png';
import micrecording from '../../assets/mic-recording.png';

const SpeechMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL + '/ask';
  const {
    error,
    interimResult,
    isRecording,
    results,
    startSpeechToText,
    stopSpeechToText,
  } = useSpeechToText({
    continuous: true,
    useLegacyResults: false
  });

  if (error) return <p>Web Speech API is not available in this browser 🤷‍</p>;

  return (
    <div>
      <h1>gravando: {isRecording.toString()}</h1>
      <button onClick={isRecording ? stopSpeechToText : startSpeechToText}>
        <img src={isRecording ? micrecording : microphone} width="32" height="32" alt="ask minion"/>
      </button>      
        {/* {results.map((result) => (
          <li key={result.timestamp}>{result.transcript}</li>
        ))} */}
        {interimResult && <li>{interimResult}</li>}
    </div>
  );
}

export default SpeechMinion;