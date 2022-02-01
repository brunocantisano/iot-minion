import React, { useState } from 'react';
import axios from 'axios';
import './styles.scss';

import {MinionBehavior} from "../../models/MinionBehavior";

interface SwitchButtonMinionProps {
  minionBehavior: MinionBehavior;
  callbackFromParent: Function;
}

const PushButtonFreezing: React.FC<SwitchButtonMinionProps> = (props: SwitchButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL===undefined?'':process.env.REACT_APP_URL + '/shake';

  const [estado] = useState(true);

  async function handleClick() {
    props.minionBehavior.freezing = !props.minionBehavior.freezing;
    const newMinionBehavior: MinionBehavior = {...props.minionBehavior};
    props.callbackFromParent(newMinionBehavior);
    console.log('👉 Resultado:', props.minionBehavior.freezing);
    try {
      const response = await axios.put(rota,
        {
          "status": estado ? 1 : 0
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
  function ShakingBody() {
    return <img id="freezing" alt="" className='shake minion-shake' onClick={handleClick} />;
  }
  function StopShakingBody() {
    return <img id="freezing" alt="" className='minion-normal' onClick={handleClick} />;
  }

  function DrawBody() {
    if (props.minionBehavior.freezing) {
      return <ShakingBody />;
    }
    return <StopShakingBody />;
  }
  return (
    <div className="freezing">
      <DrawBody />
    </div>
  )
}

export default PushButtonFreezing;