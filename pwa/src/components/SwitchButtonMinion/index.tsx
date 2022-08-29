import React, {useState} from 'react';
import axios from 'axios';
import './styles.scss';

import {MinionBehavior} from "../../models/MinionBehavior";

interface SwitchButtonMinionProps {
  minionBehavior: MinionBehavior;
  callbackFromParent: Function;
}

const SwitchButtonMinion: React.FC<SwitchButtonMinionProps> = (props: SwitchButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL + '/sensor?type=eye';

  const [estado] = useState(true);

  async function handleClick() {
    props.minionBehavior.wakeUp = !props.minionBehavior.wakeUp;
    const newMinionBehavior: MinionBehavior = {...props.minionBehavior};
    props.callbackFromParent(newMinionBehavior);
    console.log('👉 Resultado:', props.minionBehavior.wakeUp);
    try {
      const response = await axios.put(rota, 
        {
          "status": estado ? 1 : 0
        },
        {
          headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic ' + process.env.REACT_APP_API_MINION_TOKEN
          }
        });
      console.log('👉 Returned data:', response);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }

  return (
    <div className="SwitchButtonMinion">
      <div className="minion-switcher reverse">
        <input type="checkbox" className="check" onClick={handleClick}/>
        <div className="btn">
          <div className="eye">
            <div className="eye-back"></div>
            <div className="eye-inner">
              <div className="eye-brown"></div>
            </div>
            <div className="eye-cover"></div>
          </div>
        </div>
      </div>
    </div>
  )
}

export default SwitchButtonMinion;