[![build-firebase](https://github.com/brunocantisano/iot-minion/actions/workflows/firebase.yml/badge.svg?branch=master)](https://github.com/brunocantisano/iot-minion/actions/workflows/firebase.yml)

Este projeto foi criado com [Create React App](https://github.com/facebook/create-react-app).

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
export REACT_APP_URL="http://minion.local"
export REACT_APP_API_MINION_TOKEN="YnJ1bm86bWluaW9uMTIz"
```

**Base64**: `YnJ1bm86bWluaW9uMTIz` é um exemplo de base64 criptografado no padrão `usuario`:`senha`
* usuário: `minion`
* senha: `DespicableMe`

3. Node Version: `v16.13.2`

4. Firebase tools:

```
npm install -g firebase-tools
```

## Github Actions (CI/CD)

* Para rodar o cicd do github e subir o projeto no firebase hosting, rode o comando make firebase-token e siga os passos. Após receber o token, cadastre na sessão `secrets` do seu repositório com o nome `FIREBASE_TOKEN` e adicione também a secret `REACT_APP_URL` com a url da api do arduino.

![Firebase1](../others/imgs/firebase1.png)
![Firebase2](../others/imgs/firebase2.png)
![Firebase3](../others/imgs/firebase3.png)
![Firebase4](../others/imgs/firebase4.png)
![Firebase5](../others/imgs/firebase5.png)
![Firebase6](../others/imgs/firebase6.png)

Referências: 

* [CSS switch minion](https://codepen.io/mohab-elhamzawy/pen/qOQKNB)
* [Personagem](http://cssdeck.com/labs/minions-css)
* [Termômetro](https://codepen.io/chrisgannon/pen/vjNNew/)
* [Evitar commit de chaves](https://betterprogramming.pub/how-you-can-prevent-committing-secrets-and-credentials-into-git-repositories-adffc25c2ea2)
* [Badges](https://github.com/brunocantisano/badges)
