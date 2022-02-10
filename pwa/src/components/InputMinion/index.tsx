import React from 'react';
import axios from 'axios';
import { TextField, IconButton } from '@material-ui/core';
import SendIcon from '@material-ui/icons/Send';
import './styles.scss';

const InputMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL===undefined?'':process.env.REACT_APP_URL + '/talk';
  
  async function talk(message: string){
    try {
      const response = await axios.post(rota, 
        { 
          mensagem: message
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
  async function handleClick(){
    await talk("");
  }
  return (
    <div id="input-minion">
      <form noValidate autoComplete="off">
        <TextField className="phrase" id="standard-basic" label="Insira a sua frase" />
        <IconButton onClick={handleClick} aria-label="send">
          <SendIcon></SendIcon>
        </IconButton>
      </form>
    </div>
  )
}

export default InputMinion;