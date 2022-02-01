[![build-firebase](https://github.com/brunocantisano/iot-minion/actions/workflows/firebase.yml/badge.svg?branch=master)](https://github.com/brunocantisano/iot-minion/actions/workflows/firebase.yml)

![Stars](https://img.shields.io/github/stars/brunocantisano/iot-minion.svg)
![Forks](https://img.shields.io/github/forks/brunocantisano/iot-minion.svg)
![Watchers](https://img.shields.io/github/watchers/brunocantisano/iot-minion.svg)
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)

Este projeto foi criado com [Create React App](https://github.com/facebook/create-react-app).

## Projeto no [Arduino](arduino/README.md)

## Pré Requisitos

1. Make

```bash
sudo apt install -y build-essential
```

2. Git secrets (para evitar de subir chaves de serviços dentro do código em repositórios públicos)

```bash
git clone https://github.com/awslabs/git-secrets.git && \
    cd git-secrets/ && \
    make install
```

* Para instalar checagens específicas de AWS e GCP:

```
git secrets --register-aws
git secrets --register-gcp
```

3. Environment Variables
```
export REACT_APP_URL="https://minion.local:3000"
export REACT_APP_API_MINION_TOKEN="bWluaW9uOkRlc3BpY2FibGVNZQ=="
```

**Base64**: `bWluaW9uOkRlc3BpY2FibGVNZQ==` é um exemplo de base64 criptografado no padrão `usuario`:`senha`
* usuário: `minion`
* senha: `DespicableMe`

3. Node Version: `v16.13.2`

4. Firebase tools:

```
npm install -g firebase-tools
```

## Github Actions (CI/CD)

* Para rodar o cicd do github e subir o projeto no firebase hosting, rode o comando make firebase-token e siga os passos. Após receber o token, cadastre na sessão `secrets` do seu repositório com o nome `FIREBASE_TOKEN` e adicione também a secret `REACT_APP_URL` com a url da api do arduino.

![Firebase1](assets/firebase1.png)
![Firebase2](assets/firebase2.png)
![Firebase3](assets/firebase3.png)
![Firebase4](assets/firebase4.png)
![Firebase5](assets/firebase5.png)
![Firebase6](assets/firebase6.png)

Referências: 

* [CSS switch minion](https://codepen.io/mohab-elhamzawy/pen/qOQKNB)
* [Personagem](http://cssdeck.com/labs/minions-css)
* [Termômetro](https://codepen.io/chrisgannon/pen/vjNNew/)
* [Evitar commit de chaves](https://betterprogramming.pub/how-you-can-prevent-committing-secrets-and-credentials-into-git-repositories-adffc25c2ea2)
* [Badges](https://github.com/brunocantisano/badges)

[![iot-minion - Garagem Digital - Ipiranga](https://res.cloudinary.com/marcomontalbano/image/upload/v1600966110/video_to_markdown/images/youtube--pUK80vrXUAI-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://youtu.be/pUK80vrXUAI "iot-minion - Garagem Digital - Ipiranga")

* [Garagem Digital](https://www.garagemdigital.io/sentinela-da-garagem-digital/)

## Desenvolvedores/Contribuintes :octocat:

<table>
    <tr>
        <td align="center"><a href="https://github.io/brunocantisano"><img style="border-radius: 50%;" src="https://avatars2.githubusercontent.com/u/11641388?s=400&u=0ba16a79456c2f250e7579cb388fa18c5c2d7d65&v=4" width="100px;" alt="" /><br /><sub><b>Bruno Cantisano</b></sub></a><br /><a href="https://github.com/brunocantisano" title="Bruno Cantisano">:metal:</a></td>
        <td align="center"><a href="https://github.io/mauriciofragajr"><img style="border-radius: 50%;" src="https://avatars1.githubusercontent.com/u/22761689?s=460&u=806503605676192b5d0c363e4490e13d8127ed64&v=4" width="100px;" alt="" /><br /><sub><b>Mauricio Fraga Jr</b></sub></a><br /><a href="https://github.io/mauriciofragajr/" title="MAuricio Fraga">:sunglasses:</a></td>
        <td align="center"><a href="https://github.io/aferreirasv"><img style="border-radius: 50%;" src="https://avatars2.githubusercontent.com/u/43724436?s=460&v=4" width="100px;" alt="" /><br /><sub><b>Alan Ferreira</b></sub></a><br /><a href="https://github.com/aferreirasv" title="Alan Ferreira">:v:</a></td>
        <td align="center"><a href="https://github.io/diogoalexandria"><img style="border-radius: 50%;" src="https://avatars3.githubusercontent.com/u/42212100?s=460&v=4" width="100px;" alt="" /><br /><sub><b>Diogo Agra Alexandria</b></sub></a><br /><a href="https://github.com/diogoalexandria" title="Diogo Alexandria">:innocent:</a></td>
        <td align="center"><a href="https://github.io/AlvaroBeckerig"><img style="border-radius: 50%;" src="https://avatars0.githubusercontent.com/u/12736693?s=460&u=cdff2624a327a43e2765112a54e966a06eac6d79&v=4" width="100px;" alt="" /><br /><sub><b>Alvaro H. Beckerig</b></sub></a><br /><a href="https://github.com/AlvaroBeckerig" title="Álvaro Beckerig">:trollface:</a></td>
    </tr>
</table>
