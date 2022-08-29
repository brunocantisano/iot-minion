// See https://github.com/dialogflow/dialogflow-fulfillment-nodejs
// for Dialogflow fulfillment library docs, samples, and to report issues
'use strict';

const functions = require('firebase-functions');
const { WebhookClient } = require('dialogflow-fulfillment');
const { Card, Suggestion } = require('dialogflow-fulfillment');
const AIO_USERNAME = "<AIO_USERNAME>";
const AIO_KEY = "<AIO_KEY>";
const axios = require('axios');

process.env.DEBUG = 'dialogflow:debug'; // enables lib debugging statements

exports.dialogflowFirebaseFulfillment = functions.https.onRequest((request, response) => {
    const agent = new WebhookClient({ request, response });
    console.log('Dialogflow Request headers: ' + JSON.stringify(request.headers));
    console.log('Dialogflow Request body: ' + JSON.stringify(request.body));

    function welcome() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/play/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'Hello.mp3' } })
            .then((result) => {
                // handle success
                console.log("welcome");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function laugh() {
        const tipo = agent.parameters.tipo;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/play/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'Hehehe.mp3' } })
            .then((result) => {
                // handle success
                console.log("laugh");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function wakeup() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'What.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`eye/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'ON' } });
                console.log("wakeup");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function sleep() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Go_to_Sleep.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`eye/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'OFF' } });
                console.log("sleep");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function stress() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Restless.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`hat/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'ON' } });
                console.log("stress");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function relax() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Hmhm.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`hat/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'OFF' } });
                console.log("relax");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });            
    }

    function temperature() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/temperature?x-aio-key=${AIO_KEY}`;
        return axios.get(ADAFRUIT_URL)
            .then((result) => {
                // handle success
                const celsius = JSON.parse(result.data.last_value);
                agent.add(`<speak rate="1.55" pitch="20" volume="0">A temperatura na garagem é: ${celsius}° celsius</speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não sei qual o valor da temperatura</speak>`);
            });
    }

    function humidity() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/temperature?x-aio-key=${AIO_KEY}`;
        return axios.get(ADAFRUIT_URL)
            .then((result) => {
                // handle success
                const humidity = JSON.parse(result.data.last_value);
                agent.add(`<speak rate="1.55" pitch="20" volume="0">A umidade na garagem é: ${humidity}%</speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não sei qual o valor da umidade</speak>`);
            });
    }

    function hungry() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Bababa_Banana.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`blink/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'ON' } });
                console.log("hungry");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function lunch() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Hmhm.mp3' } })
            .then((result) => {
                // handle success
                axios.post(ADAFRUIT_URL+`blink/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'OFF' } });
                console.log("lunch");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function rock() {
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`play/data?x-aio-key=${AIO_KEY}`, { datum: { value: 'Minions_Van_Halen.mp3' } })
            .then((result) => {
                // handle success
                console.log("rock");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function talk() {
        const message = agent.parameters.message;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/`;
        return axios.post(ADAFRUIT_URL+`talk/data?x-aio-key=${AIO_KEY}`, { datum: { message: `${message}` } })
            .then((result) => {
                // handle success
                console.log("talk");
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });        
    }

    function listApplications() {
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

    function fallback() {
        agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
    }

    // Run the proper function handler based on the matched Dialogflow intent name
    let intentMap = new Map();
    intentMap.set('Default Welcome Intent', welcome);
    intentMap.set('Laugh', laugh);
    intentMap.set('WakeUp', wakeup);
    intentMap.set('Sleep', sleep);
    intentMap.set('Stress', stress);
    intentMap.set('Relax', relax);
    intentMap.set('Temperature', temperature);
    intentMap.set('Humidity', humidity);    
    intentMap.set('Hungry', hungry);
    intentMap.set('Lunch', lunch);
    intentMap.set('Rock', rock);
    intentMap.set('Talk', talk);
    intentMap.set('ListApplications', listApplications);
    intentMap.set('Default Fallback Intent', fallback);
    agent.handleRequest(intentMap);
});