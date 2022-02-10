// See https://github.com/dialogflow/dialogflow-fulfillment-nodejs
// for Dialogflow fulfillment library docs, samples, and to report issues
'use strict';

const functions = require('firebase-functions');
const { WebhookClient } = require('dialogflow-fulfillment');
const { Card, Suggestion } = require('dialogflow-fulfillment');
const AIO_USERNAME = "<AIO_USERNAME>";
const AIO_KEY = "<AIO_KEY>";
const FIREBASE_TOKEN = "<FIREBASE_TOKEN>";
const FIREBASE_STORAGE_BUCKET_ID = "<FIREBASE_STORAGE_BUCKET_ID>";
const axios = require('axios');

process.env.DEBUG = 'dialogflow:debug'; // enables lib debugging statements

exports.dialogflowFirebaseFulfillment = functions.https.onRequest((request, response) => {
    const agent = new WebhookClient({ request, response });
    console.log('Dialogflow Request headers: ' + JSON.stringify(request.headers));
    console.log('Dialogflow Request body: ' + JSON.stringify(request.body));

    function welcome() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Hello.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        agent.add(`<speak>
                <audio src="${audioSource}">
                </audio>
              </speak>`);
    }

    function laugh() {
        const tipo = agent.parameters.tipo;
        const audioSource1 = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Ahahaha.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const audioSource2 = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Hehehe.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        if (tipo == 1) {
            agent.add(`<speak><audio src="${audioSource1}"></audio></speak>`);
        } else {
            agent.add(`<speak><audio src="${audioSource2}"></audio></speak>`);
        }
    }

    function wakeup() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/What.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/eyes/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'ON' } })
            .then((result) => {
                // handle success
                // console.log(result.data.last_value);
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function sleep() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Go_to_Sleep.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/eyes/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'OFF' } })
            .then((result) => {
                // handle success
                // console.log(result.data.last_value);
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function stress() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Restless.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/hat/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'ON' } })
            .then((result) => {
                // handle success       
                // console.log(result.data.last_value);
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function relax() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Hmhm.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/hat/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'OFF' } })
            .then((result) => {
                // handle success
                // console.log(result.data.last_value);
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
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
                // console.log(result.data.last_value);
                const jsonObject = JSON.parse(result.data.last_value);
                agent.add(`<speak rate="1.55" pitch="20" volume="0">A temperatura na garagem é: ${jsonObject.celsius}° celsius ou ${jsonObject.fahrenheit}° fahrenheit e com umidade: ${jsonObject.humidity}%</speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não sei qual o valor da temperatura e da umidade</speak>`);
            });
    }

    function hungry() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Bababa_Banana.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/blink/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'ON' } })
            .then((result) => {
                // handle success
                // console.log(result.data.last_value);  
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function lunch() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Hmhm.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        const ADAFRUIT_URL = `https://io.adafruit.com/api/v2/${AIO_USERNAME}/feeds/blink/data?x-aio-key=${AIO_KEY}`;
        return axios.post(ADAFRUIT_URL, { datum: { value: 'OFF' } })
            .then((result) => {
                // handle success
                // console.log(result.data.last_value);
                agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
            })
            .catch((error) => {
                agent.add(`<speak rate="1.55" pitch="20" volume="0">Desculpe, mas não entendi o seu comando</speak>`);
            });
    }

    function rock() {
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Minions_Van_Halen.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
    }

    function talk() {
        const message = agent.parameters.message;
        agent.add(`<speak rate="1.55" pitch="20" volume="0">${message}</speak>`);
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
        const audioSource = `https://firebasestorage.googleapis.com/v0/b/${FIREBASE_STORAGE_BUCKET_ID}.appspot.com/o/Look_at_You_Sound.mp3?alt=media&amp;token=${FIREBASE_TOKEN}`;
        agent.add(`<speak><audio src="${audioSource}"></audio></speak>`);
        agent.add(`<speak rate="1.55" pitch="20" volume="0">Eu não entendi. Você pode repetir?</speak>`);
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
    intentMap.set('Hungry', hungry);
    intentMap.set('Lunch', lunch);
    intentMap.set('Rock', rock);
    intentMap.set('Talk', talk);
    intentMap.set('ListApplications', listApplications);
    intentMap.set('Default Fallback Intent', fallback);
    agent.handleRequest(intentMap);
});