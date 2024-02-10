import React, { useEffect, useState } from 'react';
import Minion from './components/Minion';
import PushButtonMinion from './components/PushButtonMinion';
import InputMinion from './components/InputMinion';
import SwitchButtonMinion from './components/SwitchButtonMinion';
import SwitchButtonHatMinion from './components/SwitchButtonHatMinion';
import PushButtonBanana from './components/PushButtonBanana';
import PushButtonFreezing from './components/PushButtonFreezing';
import HatMinion from './components/HatMinion';
import { MinionBehavior } from "./models/MinionBehavior";
import { MinionTalk } from "./models/MinionTalk";
import { MinionSpeechVolume } from './models/MinionSpeechVolume';
import { Climate } from "./models/Climate";
import { Temperature, Humidity } from 'react-environment-chart';
import logo_garagem from './assets/garagem-logo.gif';
import logo_ipiranga from './assets/ipiranga.png';
import axios from 'axios';
import './App.css';
import './assets/styles/global.css';
import packageInfo from '../package.json';
import VolumeSlider from './components/VolumeSlider';
import PushButtonListening from './components/PushButtonListening';
import SpeechMinion from './components/SpeechMinion';
import { useTimer } from 'use-timer';

function App() {
  const [minionBehavior, setMinionBehavior] = useState<MinionBehavior>({ freezing: false, hungry: false, stress: false, wakeUp: false, listening: false });
  const [minionTalk, setMinionTalk] = useState<MinionTalk>({ message: "olá, tudo bem?" });
  const [minionSpeechVolume, setMinionSpeechVolume] = useState<MinionSpeechVolume>({ volume: 50 });
  
  const [celsius, setCelsius] = useState(25);
  //const [fahrenheit, setFahrenheit] = useState(75);
  const [humidity, setHumidity] = useState(80);
  // const { time, start } = useTimer({
  const { start } = useTimer({
    endTime: 120, // a cada 2 minutos eu checo a temperatura e umidade
    onTimeOver: async () => {
      // alert('chamo');
      await getTemperatureCelsius();
      await getHumidity();
      start();
    },
    initialTime: 1,
    autostart: true
  });
  async function getTemperatureCelsius() {
    try {
      let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/climate?type=celsius':'';
      if(rota !== '') {
        let dados: any = await axios.get(rota,
          {
            headers: {
              'Content-Type': 'application/json',
              'Accept': 'application/json',
              'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
            }
          });
        let data:Climate = dados.data;
        setCelsius(data.celsius);
        console.log('celsius: '+ celsius);
      }
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }
  async function getHumidity() {
    try {
      let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/climate?type=humidity':'';
      if(rota !== '') {
        let dados: any = await axios.get(rota,      
          {
            headers: {
              'Content-Type': 'application/json',
              'Accept': 'application/json',
              'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
            }
          });
        let data:Climate = dados.data; 
        setHumidity(data.humidity);
        console.log('humidity: '+ humidity);
      }
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }
 
  useEffect(() => {
    setCelsius(celsius);
    setHumidity(humidity);
    console.log("Iniciando componente");
    setMinionBehavior({ freezing: false, hungry: false, stress: false, wakeUp: false , listening: false});
  }, [celsius, humidity]);

  const changeBehavior = (newMinionBehavior: MinionBehavior) => {
    setMinionBehavior(newMinionBehavior);
  }
  const changeTalk = (newMinionTalk: MinionTalk) => {
    setMinionTalk(newMinionTalk);
  }
  const changeSpeechVolume = (newMinionSpeechVolume: MinionSpeechVolume) => {
    setMinionSpeechVolume(newMinionSpeechVolume);
  }
  return (
    <div id="page-body">
      {/* <p>tempo: {time}</p> */}
      <div className="hat-minion-container">
        <div className="grid-container">
          <div className="item1"><HatMinion stressed={minionBehavior.stress} /></div>
          <div className="personagem"><Minion minionBehavior={minionBehavior} /></div>
        </div>
      </div>
      <div className="banana-container">
        <PushButtonFreezing minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
      </div>
      <div className="temperature-container">
        <Temperature height={120} value={celsius} />
        <span className="tooltipTemperatureHumidity">{celsius}°C</span>
      </div>
      <div className="button-minion-container">
        <PushButtonBanana minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
      </div>
      <div className="humidity-container">
        <Humidity tips={['seco', 'médio', 'úmido']} height={100} value={humidity} />
        <span className="tooltipTemperatureHumidity">{humidity}%</span>
      </div>

      <div className="input-container">
        <div className="grid-container">
          <div className="item1">
            <InputMinion minionTalk={minionTalk} callbackFromParent={changeTalk}/>
            <SpeechMinion/>            
            <VolumeSlider minionSpeechVolume={minionSpeechVolume} callbackFromParent={changeSpeechVolume}/>
          </div>
          <div className="item2"><SwitchButtonMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} /></div>
          <div className="item3"><PushButtonMinion /></div>
          <div className="item4"><PushButtonListening minionBehavior={minionBehavior} callbackFromParent={changeBehavior} /></div>
          <div className="item5"><SwitchButtonHatMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} /></div>
        </div>
      </div>

      <div className="garagem-ipiranga">
        <img className="logo-garagem" alt="logo da garagem" src={logo_garagem} />
        <img id="logo" alt="logo da ipiranga" src={logo_ipiranga} />
      </div>
      <div className="garagem-ipiranga">
        <span className="input-container">Versão: {packageInfo.version}</span>
      </div>
    </div>

  );
}

export default App;
