import React, { useEffect, useState } from 'react';
import Minion from './components/Minion';
import PushButtonMinion from './components/PushButtonMinion';
import InputMinion from './components/InputMinion';
import SwitchButtonMinion from './components/SwitchButtonMinion';
import SwitchButtonHatMinion from './components/SwitchButtonHatMinion';
import PushButtonBanana from './components/PushButtonBanana';
import HatMinion from './components/HatMinion';
import { MinionBehavior } from "./models/MinionBehavior";
import { Climate } from "./models/Climate";
import { Temperature, Humidity } from 'react-environment-chart';
import logo_garagem from './assets/garagem-logo.gif';
import logo_ipiranga from './assets/ipiranga.png';
import axios from 'axios';
import './App.css';
import './assets/styles/global.css';

function App() {
  const [minionBehavior, setMinionBehavior] = useState<MinionBehavior>({ hungry: false, stress: false, wakeUp: false });
  const [climate, setClimate] = useState<Climate>({ celsius: 25, fahrenheit: 75, humidity: 80 });

  let rota: string = process.env.REACT_APP_URL + '/temperature';

  async function getTemperature() {
    try {
      let data: Climate = await axios.get(rota);
      climate.celsius = data.celsius;
      climate.fahrenheit = data.fahrenheit;
      climate.humidity = data.humidity;
      setClimate(climate);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }
  useEffect(() => {
    // console.log("clima foi alterado");
    setClimate(climate);
  }, [climate]);

  useEffect(() => {
    console.log("Iniciando componente");
    setMinionBehavior({ hungry: false, stress: false, wakeUp: false });
  }, []);

  const changeBehavior = (newMinionBehavior: MinionBehavior) => {
    setMinionBehavior(newMinionBehavior);
  }
  async function handleLoad() {
    await getTemperature();
  }
  return (
    <div id="page-body" onLoad={handleLoad} >
      <div className="hat-minion-container">
        <HatMinion stressed={minionBehavior.stress} />
      </div>
      <div className="minion-container">
        <Minion minionBehavior={minionBehavior} />
      </div>
      <div className="input-container">
        <InputMinion />
      </div>
      <div className="switch-minion-container">
        <SwitchButtonMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
      </div>
      <div className="switch-hat-container">
        <SwitchButtonHatMinion minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
      </div>
      <div className="banana-container">
        <PushButtonBanana minionBehavior={minionBehavior} callbackFromParent={changeBehavior} />
      </div>
      <div className="button-minion-container">
        <PushButtonMinion />
      </div>
      <div className="temperature-container">
        <Temperature height={120} value={climate.celsius} />
        <span className="tooltipTemperatureHumidity">{climate.celsius}°C</span>
      </div>
      <div className="humidity-container">
        <Humidity tips={['seco', 'médio', 'úmido']} height={100} value={climate.humidity} />
        <span className="tooltipTemperatureHumidity">{climate.humidity}%</span>
      </div>
      <div className="garagem-ipiranga">
        <img className="logo-garagem" alt="logo da garagem" src={logo_garagem} />
        <img id="logo" alt="logo da ipiranga" src={logo_ipiranga} />
      </div>
    </div>

  );
}

export default App;
