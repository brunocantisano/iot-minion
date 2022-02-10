import React from 'react';
import axios from 'axios';
import './styles.scss'

const PushButtonMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL === undefined ? '' : process.env.REACT_APP_URL + '/play';
  let midias: [
    "Ahahaha.wav",
    "Bababa_Banana.wav",
    "Bat_Bat.wav",
    "Batatinha-Frita-123.wav",
    "Go_to_Sleep.wav",
    "Hehehe.wav",
    "Hello.wav",
    "Hmhm.wav",
    "Look_at_You_Sound.wav",
    "Minions_Van_Halen.wav",
    "Restless.wav",
    "Risada-JigSaw.wav",
    "Uh-oh.wav",
    "What.wav"
  ];

  async function handleClick() {
    try {
      // escolhendo um audio aleatoriamente
      let random = Math.floor(Math.random() * midias.length);
      const response = await axios.put(rota,
        {
          "midia": midias[random]
        },
        {
          headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json',
            'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
          }
        });
      console.log('👉 Returned data:', response);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }
  return (
    <div className="button-minion">
      <button className="push--skeuo">
        <div className="minion">
          <div className="minion-body">
            <div className="dungarees-pocket" onClick={handleClick} >
              <div className="logo">
                <span></span>
              </div>
            </div>
          </div>
        </div>
      </button>
      
    </div>
  )
}

export default PushButtonMinion;