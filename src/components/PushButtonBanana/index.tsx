import React, { useState } from 'react';
import axios from 'axios';
import './styles.scss';

import {MinionBehavior} from "../../models/MinionBehavior";

interface SwitchButtonMinionProps {
  minionBehavior: MinionBehavior;
  callbackFromParent: Function;
}

const PushButtonBanana: React.FC<SwitchButtonMinionProps> = (props: SwitchButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL===undefined?'':process.env.REACT_APP_URL + '/blink';

  const [estado] = useState(true);

  async function handleClick() {
    props.minionBehavior.hungry = !props.minionBehavior.hungry;
    const newMinionBehavior: MinionBehavior = {...props.minionBehavior};
    props.callbackFromParent(newMinionBehavior);
    console.log('👉 Resultado:', props.minionBehavior.hungry);
    try {
      const response = await axios.put(rota,
        {
          "status": estado ? 1 : 0
        },
        {
          headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
          }
        });
      console.log('👉 Returned data:', response);
    } catch (e) {
      console.log(`😱 Axios request failed: ${e}`);
    }
  }
  function ShakingBanana() {
    return <img id="banana" alt="clique para fome" className='shake' onClick={handleClick} src="http://res.cloudinary.com/dpbh2bgsn/image/upload/v1496906677/banana_b1jf9l.gif" />;
  }
  function StopShakingBanana() {
    return <img id="banana" alt="clique para saciar a fome" onClick={handleClick} src="http://res.cloudinary.com/dpbh2bgsn/image/upload/v1496906677/banana_b1jf9l.gif" />;
  }
  function DrawBanana() {
    if (props.minionBehavior.hungry) {
      return <ShakingBanana />;
    }
    return <StopShakingBanana />;
  }
  return (
    <div className="banana">
      <DrawBanana />
    </div>
  )
}

export default PushButtonBanana;