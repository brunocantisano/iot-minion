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
import axios from 'axios';
import './App.css';
import './assets/styles/global.css';
import packageInfo from '../package.json';
import VolumeSlider from './components/VolumeSlider';
import PushButtonListening from './components/PushButtonListening';
import SpeechMinion from './components/SpeechMinion';
import { useTimer } from 'use-timer';

const logo_garagem = 'https://i.ibb.co/hv8HZwv/garagem-logo.gif';
const logo_ipiranga = 'https://i.ibb.co/jmS2bv2/ipiranga.png';

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
      <header className="app-header">
        <div className="app-header__brand">
          <span className="app-header__emoji" role="img" aria-label="minion">🍌</span>
          <h1>Minion IoT</h1>
        </div>
        <span className="app-header__version">v{packageInfo.version}</span>
      </header>

      <main className="app-main">
        <section className="card stage-card">
          <div className="stage-card__climate">
            <div className="climate-chip">
              <Temperature height={100} value={celsius} />
              <span className="climate-chip__value">{celsius}°C</span>
              <span className="climate-chip__label">Temperatura</span>
            </div>
            <div className="climate-chip">
              <Humidity tips={['seco', 'médio', 'úmido']} height={90} value={humidity} />
              <span className="climate-chip__value">{humidity}%</span>
              <span className="climate-chip__label">Umidade</span>
            </div>
          </div>

          <div className="stage-frame">
            <div className="hat-minion-container">
              <div className="grid-container">
                <div className="item1"><HatMinion stressed={minionBehavior.stress} /></div>
                <div className="personagem"><Minion minionBehavior={minionBehavior} /></div>
              </div>
            </div>
          </div>
        </section>

        <section className="card controls-card">
          <h2 className="card__title">Brincar com o Minion</h2>
          <div className="controls-grid">
            <div className="control-item">
              <PushButtonBanana minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
              <span className="control-item__label">Banana</span>
            </div>
            <div className="control-item">
              <PushButtonFreezing minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
              <span className="control-item__label">Assustar</span>
            </div>
            <div className="control-item">
              <PushButtonListening minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
              <span className="control-item__label">Rádio</span>
            </div>
            <div className="control-item">
              <PushButtonMinion />
              <span className="control-item__label">Tocar som</span>
            </div>
          </div>
          <div className="controls-switches">
            <div className="control-item">
              <SwitchButtonMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
              <span className="control-item__label">Acordar</span>
            </div>
            <div className="control-item">
              <SwitchButtonHatMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
              <span className="control-item__label">Chapéu</span>
            </div>
          </div>
        </section>

        <section className="card chat-card">
          <h2 className="card__title">Fale com o Minion</h2>
          <InputMinion minionTalk={minionTalk} callbackFromParent={changeTalk} />
          <div className="chat-card__row">
            <SpeechMinion />
            <VolumeSlider minionSpeechVolume={minionSpeechVolume} callbackFromParent={changeSpeechVolume} />
          </div>
        </section>
      </main>

      <footer className="app-footer">
        <img className="app-footer__logo" alt="logo da garagem" src={logo_garagem} />
        <img className="app-footer__logo app-footer__logo--ipiranga" alt="logo da ipiranga" src={logo_ipiranga} />
      </footer>
    </div>
  );
}

export default App;
