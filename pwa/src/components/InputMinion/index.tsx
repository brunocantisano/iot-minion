import React from 'react';
import axios from 'axios';
import { TextField, IconButton } from '@material-ui/core';
import SendIcon from '@material-ui/icons/Send';
import './styles.scss';

import {MinionTalk} from "../../models/MinionTalk";

interface InputButtonMinionProps {
  minionTalk: MinionTalk;
  callbackFromParent: Function;
}

const InputMinion: React.FC<InputButtonMinionProps> = (props: InputButtonMinionProps) => {
  let rota: string = process.env.REACT_APP_URL + '/talk';
  
  async function handleClick() {
    const newMinionTalk: MinionTalk = {...props.minionTalk};
    props.callbackFromParent(newMinionTalk);
    console.log('👉 Resultado:', props.minionTalk.message);
    try {
      const response = await axios.post(rota,
        {
          "mensagem": props.minionTalk.message
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
  async function updateInputValue(evt: any){
    //console.log("updateInputValue: "+evt.target.value);
    props.minionTalk.message=evt.target.value;
  }
  return (
    <div id="input-minion">
      <form noValidate autoComplete="off">
        <TextField onChange={updateInputValue} className="phrase" id="standard-basic" label="Insira a sua frase" />
        <IconButton onClick={handleClick} aria-label="send">
          <SendIcon></SendIcon>
        </IconButton>
      </form>
    </div>
  )
}

export default InputMinion;