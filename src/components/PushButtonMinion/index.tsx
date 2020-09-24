import React from 'react';
import axios from 'axios';
import './styles.scss'

const PushButtonMinion: React.FC = () => {
  let rota: string = process.env.REACT_APP_URL === undefined ? '' : process.env.REACT_APP_URL + '/laugh';
  async function handleClick() {
    try {
      const response = await axios.put(rota,
        {
          "midia": 1
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
  return (
    <div className="button-minion">
      <button className="push--skeuo">
        <div className="minion">
          <div className="minion-body">
            <div className="dungarees-pocket" onClick={handleClick} >
              <div className="logo">
                <span></span>
              </div>
            </div>
          </div>
        </div>
      </button>
      
    </div>
  )
}

export default PushButtonMinion;