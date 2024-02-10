import React, { useState } from 'react';
import axios from 'axios';
import './styles.scss'
import {MinionBehavior} from "../../models/MinionBehavior";

interface SwitchButtonMinionProps {
  minionBehavior: MinionBehavior;
  callbackFromParent: Function;
}

const PushButtonListening: React.FC<SwitchButtonMinionProps> = (props: SwitchButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/playRemote':'';
  const [midias] = useState([
    "http://mp3.ffh.de/radioffh/hqlivestream.mp3",
    "http://stream.friskyradio.com:9000/frisky_mp3_h",
    "https://tunein.com/radio/Rdio-Antena-1-Rio-1037-s18656",
    "https://tunein.com/radio/Radio-Cidade-s220658",
    "https://tunein.com/radio/Rdio-Mar-Aberto-987-s158563",
    "https://tunein.com/radio/Rdio-Transamrica-Rio-1013-s144939",
    "0n-80s.radionetz.de:8000/0n-70s.mp3",
    "mediaserv30.live-streams.nl:8000/stream",
    "www.surfmusic.de/m3u/100-5-das-hitradio,4529.m3u",
    "stream.1a-webradio.de/deutsch/mp3-128/vtuner-1a",
    "mp3.ffh.de/radioffh/hqlivestream.aac", //  128k aac
    "www.antenne.de/webradio/antenne.m3u",
    "listen.rusongs.ru/ru-mp3-128",
    "edge.audio.3qsdn.com/senderkw-mp3",
    "macslons-irish-pub-radio.com/media.asx",
  ]);

  async function handleClick() {
    try {
      if(rota !== ''){
        props.minionBehavior.listening = !props.minionBehavior.listening;
        // escolhendo um audio aleatoriamente
        let random = Math.floor(Math.random() * midias.length);
        const response = await axios.post(rota,
          {
            "url": midias[random]
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
  function MusicBody() {
    return <img id="music" alt="" className='play music' onClick={handleClick} />;
  }
  function StopMusicBody() {
    return <img id="music" alt="" className='play' onClick={handleClick} />;
  }

  function DrawMusicBody() {
    if (props.minionBehavior.listening) {
      return <MusicBody />;
    }
    return <StopMusicBody />;
  }
  return (
    <div className="music">
      <DrawMusicBody />
    </div>
  )
}

export default PushButtonListening;