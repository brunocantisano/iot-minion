import React, {useEffect, useState} from 'react';
import axios from 'axios';
import './styles.scss';
import {MinionBehavior} from "../../models/MinionBehavior";

export interface MinionProps {
  minionBehavior: MinionBehavior;
}

const Minion: React.FC<MinionProps> = (props: MinionProps) => {
  let rota = process.env.REACT_APP_URL;

  const [minionBehavior, setMinionBehavior] = useState<MinionBehavior>(props.minionBehavior);

  async function getEyes() {
    try {
      let eyes = await axios.get(rota + '/sensors?type=eye',
      {
        headers: {
          'Accept': 'application/json',
          'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
        }
      });
      props.minionBehavior.wakeUp = !Boolean(eyes.status);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  async function getBody() {
    try {
      let body = await axios.get(rota + '/sensors?type=blink',
      {
        headers: {
          'Accept': 'application/json',
          'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
        }
      });
      props.minionBehavior.hungry = !Boolean(body.status);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  async function getHat() {
    try {
      let hat = await axios.get(rota + '/sensors?type=hat',
      {
        headers: {
          'Accept': 'application/json',
          'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
        }
      });
      props.minionBehavior.stress = !Boolean(hat.status);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  async function getShake() {
    try {
      let hat = await axios.get(rota + '/sensors?type=shake',
      {
        headers: {
          'Accept': 'application/json',
          'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
        }
      });
      props.minionBehavior.freezing = !Boolean(hat.status);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  useEffect(() => {    
    setMinionBehavior(props.minionBehavior);
  }, [props.minionBehavior]);

  function WakeUp() {
    return (
      <span className="red_iris"></span>
    );
  }
  function Sleep() {
    return (
      <span className="black_iris"></span>
    );
  }
  function DrawEyes() {
    if (!props.minionBehavior.wakeUp) {
        return <Sleep />;
    }
    return <WakeUp />;
  }
  async function handleLoad() {
    await getEyes();
    await getBody();
    await getHat();
    await getShake();
  }
  return (
      <div onLoad={handleLoad} className={minionBehavior.hungry ? "jerry jerry_hungry" : minionBehavior.freezing ? "jerry jerry_freezing" : "jerry"}>
      <div className="lights">
        <span className="white_light"></span>
        <span className="dark_light"></span>
      </div>
      <div className="jerry_hair">
        <ul>
          <li className="h1"></li>
          <li className="h2"></li>
          <li className="h3"></li>
          <li className="h4"></li>
          <li className="h5"></li>
          <li className="h6"></li>
          <li className="h7"></li>
          <li className="h8"></li>
          <li className="h9"></li>
          <li className="h10"></li>
          <li className="h11"></li>
          <li className="h12"></li>
        </ul>
      </div>
      <div className="eyes1">
        <div className="eye_animate"></div>
        <div className="glasses"></div>
        <div className="white_part">
          <div className="brown_eye">
            <DrawEyes />
          </div>
        </div>
      </div>
      <div className="eyes2">
        <div className="eye_animate"></div>
        <div className="glasses"></div>
        <div className="white_part">
          <div className="brown_eye">
            <DrawEyes />
          </div>
        </div>
      </div>
      <div className="jerry_hand">
        <div className="jerry_lh"></div>
        <div className="animated_lh">
          <span className="gloves_lh"></span>
          <span className="gloves_lh2"></span>
        </div>
        <div className="jerry_rh"></div>
        <span className="gloves_rh"></span>
      </div>
      <div className="black_tie">
      <span className="right_tie">
        <div className="top_tie"></div>
        <div className="down_tie"></div>
      </span>
        <span className="left_tie">
        <div className="top_tie"></div>
        <div className="down_tie"></div>
      </span>
      </div>
      <div className={minionBehavior.hungry ? "jerry_smile_hungry" : "jerry_smile"}>
        <span className="teeth1"></span>
        <span className="teeth2"></span>
      </div>
      <div className="curves">
        <span className="jerry_curve1"></span>
        <span className="jerry_curve1 jerry_left_curve"></span>
        <span className="jerry_curve2"></span>
      </div>
      <div className="clothes">
        <div className="main_jerry"></div>
        <div className="right_shirt jerry_right_shirt"></div>
        <div className="right_shirt jerry_left_shirt"></div>
        <div className="jerry_bottom"></div>
      </div>
      <div className="pocket">
        <div className="logo"></div>
        <span className="lines"></span>
      </div>
      <div className="legs">
        <div className="jerry_shoes"><span className="jerry_small_shoes"></span></div>
        <div className="jerry_shoes jerry_left_shoes"><span className="jerry_small_shoes"></span></div>
      </div>
    </div>
  )
}

export default Minion;