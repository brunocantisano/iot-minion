import React from 'react';
import './styles.scss';

export interface HatMinionProps {
  stressed: boolean;
}

const HatMinion: React.FC<HatMinionProps> = (props: HatMinionProps) => {
  return(
    <div id="hat-minion">
      {props.stressed? <div className="hat hat-move"></div>:<div className="hat hat-not-moving"></div>}
    </div>
  )
}

export default HatMinion;

