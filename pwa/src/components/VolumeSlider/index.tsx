import React, {useEffect, useState} from 'react';
import axios from 'axios';
import './styles.scss';

import {MinionSpeechVolume} from "../../models/MinionSpeechVolume";

interface VolumeSliderMinionProps {
  minionSpeechVolume: MinionSpeechVolume;
  callbackFromParent: Function;
}

const VolumeSlider: React.FC<VolumeSliderMinionProps> = (props: VolumeSliderMinionProps) => {
  const [volume, setVolume] = useState(50);
  const [muted, setMuted] = useState(false);
  
  async function callApi() {
    try {
      let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/volume':'';
      if(rota !== ''){
          const newMinionSpeechVolume: MinionSpeechVolume = {...props.minionSpeechVolume};
          props.callbackFromParent(newMinionSpeechVolume);
          // console.log('👉 Resultado:', props.minionSpeechVolume.volume);
          props.minionSpeechVolume.volume = volume;    
          const response = await axios.put(rota,
          {
            "intensidade": muted ? 0 : volume
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

  useEffect(() => {
    callApi();
  },);

  return (
    <main>
      <section>
        <input
          type="range"
          min={0}
          max={100}
          step={10}
          value={volume}
          onChange={(event) => {
            setVolume(event.target.valueAsNumber);
          }}
        />
        <button onClick={(event) => {
          setMuted((m) => !m);          
        }}>
          {muted ? "mutar" : "desmutar"}
        </button>
      </section>
      <section>
        <p>Volume: {muted ? 0 : volume}</p>
      </section>
    </main>
  );
}

export default VolumeSlider;