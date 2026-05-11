// See https://github.com/dialogflow/dialogflow-fulfillment-nodejs
// for Dialogflow fulfillment library docs, samples, and to report issues
'use strict';
 
const functions = require('firebase-functions');
const {WebhookClient} = require('dialogflow-fulfillment');
const {Card, Suggestion} = require('dialogflow-fulfillment');
const AIO_USERNAME = "<AIO_USERNAME>";
const AIO_KEY = "<AIO_KEY>";
const axios = require('axios');

process.env.DEBUG = 'dialogflow:debug'; // enables lib debugging statements
 
exports.dialogflowFirebaseFulfillment = functions.https.onRequest((request, response) => {
  const agent = new WebhookClient({ request, response });
  console.log('Dialogflow Request headers: ' + JSON.stringify(request.headers));
  console.log('Dialogflow Request body: ' + JSON.stringify(request.body));
 
  function welcome(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Hello.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    agent.add(`<speak>
                <audio src="${audioSource}">
                </audio>
              </speak>`);
  }
  function laugh(agent) {
    const tipo = agent.parameters.tipo;
    const audioSource1 = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Ahahaha.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const audioSource2 = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Hehehe.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    if (tipo == 1) {
      agent.add(`<speak><audio src="${audioSource1}"></audio></speak>`);
    } else {
      agent.add(`<speak><audio src="${audioSource2}"></audio></speak>`);
    }
  }
  function wakeup(agent) {    
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/What.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/eyes/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'ON'}})
    .then((result) => {
      // handle success
      // console.log(result.data.last_value);
	  	agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);  
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }
  function sleep(agent) {    
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Go_to_Sleep.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/eyes/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'OFF'}})
    .then((result) => {
      // handle success
      // console.log(result.data.last_value);
	  	agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }
  function stress(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Restless.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/hat/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'ON'}})
    .then((result) => {
      // handle success       
      // console.log(result.data.last_value);
	  	agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);  
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }
  function relax(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Hmhm.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/hat/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'OFF'}})
    .then((result) => {
      // handle success
      // console.log(result.data.last_value);
	  	agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);  
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }
  function temperature(agent) {
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/temperature?x-aio-key=${AIO_KEY}`;
    return axios.get(ADAFRUIT_URL)
    .then((result) => {
    	// handle success
	    // console.log(result.data.last_value);
      	const jsonObject = JSON.parse(result.data.last_value);
	  	agent.add(`<speak rate="1.55" pitch="20" volume="0">A temperatura na garagem é: ${jsonObject.celsius}° celsius ou ${jsonObject.fahrenheit}° fahrenheit e com umidade: ${jsonObject.humidity}%</speak>`);
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não sei qual o valor da temperatura e da umidade</speak>`);
    });
  }
  function hungry(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Bababa_Banana.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/blink/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'ON'}})
    .then((result) => {
      // handle success
      // console.log(result.data.last_value);  
      agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
    })
    .catch((error) => {
      agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }
  function lunch(agent) {    
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Hmhm.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/blink/data?x-aio-key=${AIO_KEY}`;
    return axios.post(ADAFRUIT_URL, {datum: {value: 'OFF'}})
    .then((result) => {
      // handle success
      // console.log(result.data.last_value);
	  	agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);  
    });
  }  
  function rock(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Minions_Van_Halen.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
  }
  function talk(agent) {
    const message = agent.parameters.message;
    agent.add(`<speak rate="1.55" pitch="20" volume="0">${message}</speak>`);
  }
  function listApplications(agent) {
    const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/list?x-aio-key=${AIO_KEY}`;
    return axios.get(ADAFRUIT_URL)
    .then((result) => {
      // handle success       
      // console.log(result.data.last_value);
      const arrObjects = JSON.parse(result.data.last_value);
      if (arrObjects.count == 0) {
      	agent.add(`<speak rate="1.55" pitch="20" volume="0">A lista de aplicações está vazia.</speak>`);
      } else {
        agent.add(`<speak rate="1.55" pitch="20" volume="0">As aplicações que quebraram no jenkins são:</speak>`);
        for (let item of arrObjects) {
          agent.add(`<speak rate="1.55" pitch="20" volume="0">${item.name} na linguagem ${item.language} e com a descrição: ${item.description}</speak>`);	
        }
      }
    })
    .catch((error) => {
    	agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não sei como listar as aplicações que quebraram no jenkins</speak>`);  
    });
  }  
  function fallback(agent) {
    const audioSource = "https://firebasestorage.googleapis.com/v0/b/sentinela-280519.appspot.com/o/Look_at_You_Sound.mp3?alt=media&amp;token=<FIREBASE_TOKEN>";
    agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
    agent.add(`<speak rate="1.55" pitch="20" volume="0">Eu não entendi. Você pode repetir?</speak>`);
  }
  // // Uncomment and edit to make your own intent handler
  // // uncomment `intentMap.set('your intent name here', yourFunctionHandler);`
  // // below to get this function to be run when a Dialogflow intent is matched
  // function yourFunctionHandler(agent) {
  //   agent.add(`This message is from Dialogflow's Cloud Functions for Firebase editor!`);
  //   agent.add(new Card({
  //       title: `Title: this is a card title`,
  //       imageUrl: 'https://developers.google.com/actions/images/badges/XPM_BADGING_GoogleAssistant_VER.png',
  //       text: `This is the body text of a card.  You can even use line\n  breaks and emoji! 💁`,
  //       buttonText: 'This is a button',
  //       buttonUrl: 'https://assistant.google.com/'
  //     })
  //   );
  //   agent.add(new Suggestion(`Quick Reply`));
  //   agent.add(new Suggestion(`Suggestion`));
  //   agent.setContext({ name: 'weather', lifespan: 2, parameters: { city: 'Rome' }});
  // }

  // // Uncomment and edit to make your own Google Assistant intent handler
  // // uncomment `intentMap.set('your intent name here', googleAssistantHandler);`
  // // below to get this function to be run when a Dialogflow intent is matched
  // function googleAssistantHandler(agent) {
  //   let conv = agent.conv(); // Get Actions on Google library conv instance
  //   conv.ask('Hello from the Actions on Google client library!') // Use Actions on Google library
  //   agent.add(conv); // Add Actions on Google library responses to your agent's response
  // }
  // // See https://github.com/dialogflow/fulfillment-actions-library-nodejs
  // // for a complete Dialogflow fulfillment library Actions on Google client library v2 integration sample

  // Run the proper function handler based on the matched Dialogflow intent name
  let intentMap = new Map();
  intentMap.set('Default Welcome Intent', welcome);
  intentMap.set('Laugh', laugh);
  intentMap.set('WakeUp', wakeup);
  intentMap.set('Sleep', sleep);
  intentMap.set('Stress', stress);
  intentMap.set('Relax', relax);
  intentMap.set('Temperature', temperature);
  intentMap.set('Hungry', hungry);
  intentMap.set('Lunch', lunch);
  intentMap.set('Rock', rock);
  intentMap.set('Talk', talk);
  intentMap.set('ListApplications', listApplications);
  intentMap.set('Default Fallback Intent', fallback);
  agent.handleRequest(intentMap);
});
