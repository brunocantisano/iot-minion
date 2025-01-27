import React, { useState } from 'react';
import axios from 'axios';
import './styles.scss'

const PushButtonMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/play':'';
  const [midias] = useState([
    "Ahahaha.mp3",
    "Bababa_Banana.mp3",
    "Bat_Bat.mp3",
    "Batatinha-Frita-123.mp3",
    "Go_to_Sleep.mp3",
    "Hehehe.mp3",
    "Hello.mp3",
    "Hmhm.mp3",
    "Look_at_You_Sound.mp3",
    "Minions_Van_Halen.mp3",
    "Restless.mp3",
    "Risada-JigSaw.mp3",
    "Uh-oh.mp3",
    "What.mp3"
  ]);

  async function handleClick() {
    try {
      if(rota !== ''){
        // escolhendo um audio aleatoriamente
        let random = Math.floor(Math.random() * midias.length);
        const response = await axios.post(rota,
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
      }
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