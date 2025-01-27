import React from 'react';
import axios from 'axios';
import './styles.scss';

import {MinionBehavior} from "../../models/MinionBehavior";

interface SwitchButtonMinionProps {
  minionBehavior: MinionBehavior;
  callbackFromParent: Function;
}

const SwitchButtonHatMinion: React.FC<SwitchButtonMinionProps> = (props: SwitchButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL ? process.env.REACT_APP_URL + '/sensor?type=hat':'';
  async function handleClick() {
    try {
      if(rota !== '') {
        props.minionBehavior.stress = !props.minionBehavior.stress;
        const newMinionBehavior: MinionBehavior = {...props.minionBehavior};
        props.callbackFromParent(newMinionBehavior);
        console.log('👉 Resultado:', props.minionBehavior.stress);
        const response = await axios.put(rota,
          {
            "status": props.minionBehavior.stress ? 1 : 0
          },
          {
            headers: {
              'Content-Type': 'application/json',
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
    <div className="SwitchButtonHatMinion">
      <div className="wrapper">
        <input type="checkbox" name="checkboxHat" className="switch" onClick={handleClick} />
      </div>
    </div>
  )
}

export default SwitchButtonHatMinion;