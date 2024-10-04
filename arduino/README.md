# IOT-Minion

[![build-arduino](https://github.com/brunocantisano/iot-minion/actions/workflows/arduino.yml/badge.svg?branch=master)](https://github.com/brunocantisano/iot-minion/actions/workflows/arduino.yml)
![versão](https://img.shields.io/github/v/release/brunocantisano/iot-minion)
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE)

![working](../others/imgs/Working.gif)

Robô caseiro feito na época da [quarentena](https://bigdata-covid19.icict.fiocruz.br/) com uma garrafa de sabonete líquido com o tema dos minions. [API](https://www.youtube.com/watch?v=OVvTv9Hy91Q&feature=emb_rel_pause) feita em REST e não SOAP. 😁

![3 Laughing](../others/imgs/3laughing.gif)

[SOAP 🆚 REST](https://www.infoq.com/br/articles/rest-soap-when-to-use-each/)

A aplicação consiste em três desenvolvimentos: 

- Backend feito na linguagem `C++` para rodar na placa [ESP32](https://pt.wikipedia.org/wiki/ESP32). Seu código-fonte encontra-se dentro da pasta `iot-minion`
- Frontend feito com a metodologia de desenvolvimento de software chamada de [PWA](https://garagem.ipiranga.io/nativo-hibrido-ou-pwa), usando a linguagem `React`. Seu código-fonte encontra-se na pasta raíz do projeto.
- Interfaces de Conversação feita na linguagem `javascript` para ser utilizada no [Dialog Flow](https://en.wikipedia.org/wiki/Dialogflow) da Google. Encontra-se dentro da pasta `arduino/dialogFlow`

## Visão geral do projeto

<a href="https://brunocantisano.github.io/minion/index.html" target="_blank"><img src="../others/imgs/book.png" /></a>

## Pre-requisitos

### Instalar placas que serão usadas

Adicionar no campo `URLs Adicionais para Gerenciadores de Placas` as linhas abaixo, **separadas por vírgulas**:

* http://arduino.esp8266.com/stable/package_esp8266com_index.json
* https://dl.espressif.com/dl/package_esp32_index.json


![Preferências](../others/imgs/preferencias.png)

#### Instalar as placas

![ESP8266](../others/imgs/placa-esp8266.png)
![ESP32](../others/imgs/placa-esp32.png)

#### Referências
- [Preparando o ambiente com arduino IDE para ESP32](https://blog.eletrogate.com/conhecendo-o-esp32-usando-arduino-ide-2/)
- [Preparando o ambiente com arduino IDE para ESP8266](https://blog.smartkits.com.br/esp8266-como-programar-o-nodemcu-atraves-da-arduino-ide/)

#### Instalar as bibliotecas

* ArduinoWebsockets
* Adafruit FONA Library
* Adafruit MQTT Library
* Adafruit SleepyDog Library
* Adafruit Unified Sensor
* ArduinoJson
* AsyncElegantOTA
* DHT sensor library
* PubSubClient
* WiFi101
* LittleFS_esp32
* Preferences
* ESPAsyncWebServer
* AsyncTCP
* ESP32-audioI2S
* DHT sensor library
* ChatGPTuino

#### Instalar as bibliotecas .zip (fazer download do código e importar no arduino IDE)

* https://github.com/me-no-dev/ESPAsyncWebServer.git
* https://github.com/me-no-dev/AsyncTCP.git
* https://github.com/schreibfaul1/ESP32-audioI2S.git

### Plugins (Pre-requisito do ESP8266/ESP32 para decodificar Exception Stack Trace e para gravar arquivos do webserver na pasta `data` - LittleFS filesystem uploader)

- [Arduino ESP32 LittleFS filesystem uploader](https://github.com/lorol/arduino-esp32littlefs-plugin)

> **O código está usando o LittleFS para ler e escrever no storage, que serve para utilizar o filesystem com melhor performance e aproveitamento do espaço físico.**
 
- [Arduino ESP8266/ESP32 Exception Stack Trace Decoder](https://github.com/me-no-dev/EspExceptionDecoder)
 
* Pre-requisito do arduino IDE: 

```
sudo apt install python3-serial -y
```

* Criar diretório se não existir

```
mkdir -p ~/Arduino/tools/
```

*  Mover o arquivo jar para dentro da pasta

```
mv ~/Downloads/EspExceptionDecoder-*.zip ~/Arduino/tools
```

* Descompactar

```
unzip EspExceptionDecoder-*.zip
```

* Remover arquivo

```
rm -rf EspExceptionDecoder-*.zip
```

* Instalar dependências

```
sudo apt install libncurses5 libpython2.7 -y
```

#### Referência para o LittleFS filesystem uploader

* Criar diretório se não existir

```
mkdir -p ~/Arduino/tools/ESP32LittleFS/tool/
```

*  Mover o arquivo jar para dentro da pasta

```
mv ~/Downloads/esp32littlefs.jar ~/Arduino/tools/ESP32LittleFS/tool/esp32littlefs.jar
```

### Compilação

1. No menu `Ferramentas`, escolha a opção `Upload Speed: "115200"`

2. No menu `Ferramentas`, escolha a opção `Partition Scheme: "Minimal SPIFFS (1.9MB APP With OTA/190KB SPIFFS)"` (**o código supera o tamanho padrão de 1.2MB para o APP**)

- **Não se esqueça de alterar as variáveis abaixo, que aparecem nos códigos do arduino (`credentials.h`) e dialogflow, para as suas chaves:**

| Variáveis                      | Serviço                        |
|--------------------------------|--------------------------------|
| <AIO_USERNAME>                 | Adafruit                       |
| <AIO_KEY>                      | Adafruit                       |
| <FIREBASE_API_KEY>             | Firebase API Key               |
| <API_MINION_TOKEN>             | Base64 Basic Auth              |
| <API_VERSION>                  | Versão da API                  |

3. Clique no botão de compilação ![compilar](../others/imgs/compilar.png)

> Se por acaso a compilação do arduino aparecer a mensagem: `"exec: "python": executable file not found in $PATH`, faça essa instalação abaixo:

```
sudo apt install python-is-python3
```

## Extras

- Firebase para armazenamento dos áudios utilizados neste projeto
- `sonar-project.properties` para configurar o projeto a ser analisado pelo sonar-scanner.

## Apresentação

<a href="https://brunocantisano.github.io/apresentacao/index.html" target="_blank"><img src="../others/imgs/blah.gif" /></a>

## Minimizando o Html e o Json para o Swagger

[Minifier](https://www.willpeavy.com/tools/minifier/)
 
### Adafruit☁️

<a href="https://brunocantisano.github.io/minion/index.html" target="_blank"><img src="../others/imgs/book.png" /></a>

## Gerando certificado auto assinado

[Procedimento](https://www.ibm.com/docs/pt-br/api-connect/5.0.x?topic=profiles-generating-self-signed-certificate-using-openssl)
Para gerar um certificado SSL autoassinado usando o OpenSSL, conclua as etapas a seguir:

1) Anote o Nome comum (CN) do Certificado SSL. O CN é o nome completo do sistema que usa o certificado. Se você estiver usando DNS dinâmico, seu CN deverá ter um curinga, por exemplo: *.api.com. Caso contrário, use o nome do host ou o endereço IP configurado no Cluster de gateway (por exemplo: `minion.local`).

2) Execute o comando OpenSSL a seguir para gerar sua chave privada e seu certificado público. Responda às perguntas e insira o Nome comum quando solicitado.

```sh
openssl genrsa -out key.pem 1024
```

3) Revise o certificado criado:

```sh
openssl req -x509 -out cert.pem -key key.pem -new -sha256 -subj /CN=minion.local/O=acme/C=BR/emailAddress=bruno.cantisano@gmail.com -addext "keyUsage=digitalSignature,keyEncipherment" -addext extendedKeyUsage=serverAuth
```

4) Combine sua chave e o certificado em um pacote configurável PKCS#12 (P12):

```sh 
openssl pkcs12 -inkey key.pem -in cert.pem -export -out certificate.p12
```

5) Valide seu arquivo P2.

```sh
openssl pkcs12 -in certificate.p12 -noout -info
```

6) Gerar arquivo `crt` a partir do arquivo `pem`

```sh
openssl x509 -outform der -in cert.pem -out cert.crt
```

## APP 📱

```sh
npm run dev
```
## Referencias da placa ESP32

![Pinout](../others/imgs/ESP32_PINOUT.png)

 📣 No código arduino foram utilizadas as seguintes portas

| Porta       | Sensor       |
|-------------|--------------|
| 13          | Chapéu       |
| 14          | Olhos        |
| 15          | Pisca        |
| 22          | Treme        |
| 33          | Temperatura  |
| 5           | Cartão SD    |
| 18          | Cartão SD    |
| 19          | Cartão SD    |
| 23          | Cartão SD    |
| 25          | I2S - Audio  |
| 26          | I2S - Audio  |
| 27          | I2S - Audio  |

### Web Server

 * [Web Server](http://minion.local)
 * [Swagger](http://minion.local/swaggerUI)
 * [Swagger API](http://minion.local/swagger.json)
 
<a href="https://brunocantisano.github.io/minion/index.html#page/22#" target="_blank"><img src="../others/imgs/webserver.png" /></a>

### Upload de firmware

![firmware e filesystem](../others/imgs/firmware_filesystem.png)

![firmware e filesystem](../others/imgs/Elegant-OTA.png)

## Tecnologias 💡

- [x] [Git](https://pt.wikipedia.org/wiki/Git)
- [x] [REST](https://pt.wikipedia.org/wiki/REST)
- [x] [MQTT-Adafruit](https://io.adafruit.com/api/docs/#adafruit-io-http-api)
- [x] [IOT-Arduino ESP32](https://pt.wikipedia.org/wiki/ESP32)
- [x] [React](https://pt.wikipedia.org/wiki/React_(JavaScript))
- [x] [Swagger](https://swagger.io/)
- [x] [Firebase](https://en.wikipedia.org/wiki/Firebase)
- [x] [Jenkins](https://www.jenkins.io/)
- [x] [Postman](https://www.postman.com/)
- [x] [QR code](https://www.qrcode-monkey.com/)
- [x] [SonarQube](https://docs.sonarqube.org/latest/setup/get-started-2-minutes/)
- [x] [OTA para atualização do ESP32 pelo wifi](https://www.filipeflop.com/blog/atualizacao-de-software-ota-over-the-air-no-esp32/)
- [x] [ElegantOTA-update de firmware e filesystem](https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/)

## Google Home e Google Assistente

**Comandos de voz do google assistente** 👂

  * `Ok Google, falar com o minion` 📣
  * `Acordar` 🌄
  * `Durma` 🌛
  * `Estressar` 😠
  * `Relaxar` 😆
  * `Como está o clima?`, `Qual a temperatura?` ⛅
  * `Qual a umidade?` ⛅
  * `Repita garagem` 🚗
  * `Sorria 1`, `Sorria 2` 😃
  * `Ola` 👋
  * `Café da manhã`, `Lanche`, `Janta`, `Almoço`, `Fome` 🍌
  * `Você já comeu`, `Você já jantou`, `Você já almoçou`, `Você já lanchou` 😊
  * `Rock`, `Música`, `Toca uma música aí` 🎸 🤘
  * `Lista de aplicações` 📜

[![eruption minions](https://res.cloudinary.com/marcomontalbano/image/upload/v1594316625/video_to_markdown/images/youtube--5OQWZ3kCnpA-c05b58ac6eb4c4700831b2b3070cd403.jpg)](https://www.youtube.com/watch?v=5OQWZ3kCnpA "eruption minions")

## Modelos 3D 👻

[![Modelos 3D](../others/imgs/3dmodels.png)](https://brunocantisano.github.io/minion/index.html)


<table>
    <tr>
        <td align="center"><a href="https://free3d.com/3d-model/two-minions-ready-for-rigging-8605.html"><img src="https://preview.free3d.com/img/2014/10/1714721518392444385/sc9p1dne.jpg" width="100px;" alt="" /><br /><sub><b>Minion</b></sub></a><br />
        </td>
        <td align="center"><a href="https://sketchfab.com/3d-models/dr-nefario-dab-dance-af530ffc10e94117a78363f7a3b204f6"><img src="https://static.wikia.nocookie.net/villains/images/f/fb/DrNefario.png/revision/latest/scale-to-width-down/206?cb=20210303191340" width="100px;" alt="" /><br /><sub><b>Dr Nefário</b></sub></a><br />
        </td>
        <td align="center"><a href="https://free3d.com/3d-model/ricken-backer-4003---bass-888215.html"><img src="https://images.free3d.com/imgd/l96/5ac7fb2326be8b18328b4567/2893-ricken-backer-4003---bass.png" width="100px;" alt="" /><br /><sub><b>Ricken Backer Bass</b></sub></a><br />
        </td>
        <td align="center"><a href="https://free3d.com/3d-model/classic-flying-v-54812.html"><img src="https://images.free3d.com/imgd/l65871-classic-flying-v-54812.jpg" width="100px;" alt="" /><br /><sub><b>Classic Flying V</b></sub></a><br />
        </td>
        <td align="center"><a href="https://free3d.com/3d-model/drum-set-99664.html"><img src="https://images.free3d.com/imgd/l52925-drum-set-99664.jpg" width="100px;" alt="" /><br /><sub><b>Drum Set</b></sub></a><br />
        </td>
        <td align="center"><a href="https://free3d.com/3d-model/iphonex-113534.html"><img src="https://images.free3d.com/imgd/l45/5a8085c926be8b954b8b4567/8356-iphonex.png" width="100px;" alt="" /><br /><sub><b>Iphonex</b></sub></a><br />
        </td>
    </tr>
</table>

## Links utilizados no projeto 🔗

- [Push Button](https://cssdeck.com/labs/animated-push-button)
- [Banana Button](https://www.codeseek.co/preview/weKryW)
- [Minion](https://cssdeck.com/labs/minions-css)
- [Switch Button](https://fribly.com/2015/11/28/css-minion-switch-button/)
- [React Charts](https://reactjsexample.com/a-react-environment-charts/)
- [Github Emoticons](https://gist.github.com/rxaviers/7360908)
- [API Rest - Antonio Mancuso](https://mancusoa74.blogspot.com/2018/02/simple-http-rest-server-on-esp8266.html)
- [Temperatura e umidade - DHT11](https://github.com/amiroffme/esp8266-dht11-webserver)
- [Audio files - Minion](https://www.soundboard.com/sb/minions "Minion_Audio_Files")
- [Firebase - Storage](https://firebase.google.com/docs/storage "Subir arquivos no Storage")
- [Conexão Wifi selecionada pelo Bluetooth](https://robotzero.one/esp32-wi-fi-connection-bluetooth/)
- [Tocando audio pelo Esp32](https://circuitdigest.com/microcontroller-projects/esp32-based-audio-player)
- [Pipeline no github action para projetos arduino](https://medium.com/swlh/how-to-create-an-automated-build-pipeline-for-your-arduino-project-1df9826f2a5e)
- [Async Web Server Upload de arquivos](https://github.com/smford/esp32-asyncwebserver-fileupload-example/blob/master/example-01/example-01.ino)
- [Async get params](https://techtutorialsx.com/2017/12/17/esp32-arduino-http-server-getting-query-parameters/)
- [Evitando apertar botão de reset para upload de código](https://randomnerdtutorials.com/solved-failed-to-connect-to-esp32-timed-out-waiting-for-packet-header/)
- [Criptografia](https://portal.vidadesilicio.com.br/seguranca-de-dados-com-aes/)
- [Bluetooth Google Assistant](https://support.google.com/assistant/answer/9281916#zippy=%2Cuse-your-voice)
- [Explicação sobre umidade do ar](https://www.cgesp.org/v3/umidade-relativa-do-ar.jsp
)
- [HTTPS failure with low free heap memory](https://github.com/espressif/arduino-esp32/issues/2175)
- [How to use the PSRAM with ArduinoJson](https://arduinojson.org/v6/how-to/use-external-ram-on-esp32/)
- [Página HTML+CSS para exibição de imagens](https://codepen.io/diemoritat/pen/LKROYZ)
- [Base64 encode](https://www.dfrobot.com/blog-1210.html)
- [Base64 decode](https://www.dfrobot.com/blog-1199.html)
- [Load Wav File](https://www.xtronical.com/i2s-ep3/)
- [Converte binário para hexadecimal](http://tomeko.net/online_tools/file_to_hex.php?lang=en)
- [Audio para Google Assistente](https://github.com/pschatzmann/ESP32-A2DP)
- [Criando threads no ESP32](https://techtutorialsx.com/2017/05/06/esp32-arduino-creating-a-task/)
- [Visualizar imagens como livro](http://www.turnjs.com/)
- [ESP32 com métricas para prometheus](https://github.com/douglaszuqueto/esp32-prometheus)
- [Write binary file to SPIFFS](https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloworld_read_write_text_file/file.ino)
- [Write binary file to LittleFS](https://iotespresso.com/esp32-captive-portal-fetching-html-using-littlefs/)
- [LittleFS esp32 issues](https://wellys.com/posts/esp32_issues/)
- [ESP32 HTTPS web server](https://techtutorialsx.com/2019/04/07/esp32-https-web-server/)
- [I2S MP3 Player](https://www.fernandok.com/2020/02/mp3-player-com-esp32-e-i2s.html)
- [Google Actions](https://codelabs.developers.google.com/codelabs/actions-1/#0)

## Vídeos de referência 🎥

[![Site DC motor](https://i3.ytimg.com/vi/ml366LJiwnk/maxresdefault.jpg)](https://www.youtube.com/watch?v=ml366LJiwnk&feature=emb_imp_woyt "Site DC motor")

[![Utilizando Obsidian para fazer apresentações como código](https://i3.ytimg.com/vi/LtBK_iNcVEQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=LtBK_iNcVEQ "Utilizando Obsidian para fazer apresentações como código")

## Peças

<table>
    <tr>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-2042867002-micro-motor-dc-3v-vibraco-vibracall-5-pecas-celular-pager-_JM?matt_tool=40343894&matt_word=&matt_source=google&matt_campaign_id=14303413655&matt_ad_group_id=125984293117&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539354956680&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=507180778&matt_product_id=MLB2042867002&matt_product_partition_id=1404886571418&matt_target_id=aud-315891067179:pla-1404886571418&gclid=EAIaIQobChMIzOug37jt9AIVxdzICh0bSwGnEAQYAiABEgL8KvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_808293-MLB47732501155_102021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Motor vibra call</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-1353051721-20-led-flash-5mm-rgb-alto-brilho-lento-2-pin-cor-automatica-_JM?matt_tool=56291529&matt_word=&matt_source=google&matt_campaign_id=14303413604&matt_ad_group_id=125984287157&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539354956218&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=164968240&matt_product_id=MLB1353051721&matt_product_partition_id=1404886571258&matt_target_id=aud-1457490208988:pla-1404886571258&gclid=CjwKCAiArOqOBhBmEiwAsgeLmYz9lBnG_pNvFYrp1jvWBnjUr7PzNy1FyH0LaA6pvpBPgAPm6Tf7DRoCJHUQAvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_962744-MLB44915060024_022021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Led Rgb Cor Automática</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-1800879468-led-vermelho-difuso-5mm-50-unidades-eletrnica-arduino-_JM?matt_tool=56291529&matt_word=&matt_source=google&matt_campaign_id=14303413604&matt_ad_group_id=125984287157&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539354956218&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=310832918&matt_product_id=MLB1800879468&matt_product_partition_id=1435016894331&matt_target_id=pla-1435016894331&gclid=CjwKCAiArOqOBhBmEiwAsgeLmbnvvMu-Yn9KMhgQkwQGaMeUJbqecAT_ruqnkHSVPk6Nbm1HMeirRhoC1EsQAvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_836520-MLB44941848166_022021-O.webp" width="100px;" alt="" /><br /><sub><b>2-Led Vermelho</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-1735283882-40-jumpers-macho-fmea-20cm-cabo-fios-protoboard-jumper-_JM?matt_tool=56291529&matt_word=&matt_source=google&matt_campaign_id=14303413604&matt_ad_group_id=125984287157&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539354956218&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=280311926&matt_product_id=MLB1735283882&matt_product_partition_id=1404886571258&matt_target_id=aud-615548715344:pla-1404886571258&gclid=CjwKCAiArOqOBhBmEiwAsgeLmegdNyfSJLVYgYEmbQR87zuYKnnv_xfrjt4CkfrT8n_qvDAVwoB_MhoCU4cQAvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_766177-MLB44261620026_122020-O.webp" width="100px;" alt="" /><br /><sub><b>Jumpers Macho Fêmea 20cm</b></sub></a><br />
        </td>
    </tr>
    <tr>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-1558103893-cabo-carregador-micro-usb-reforcado-amazon-kindle-paperwhite-_JM#searchVariation=65550515777&position=8&search_layout=stack&type=pad&tracking_id=72b4bc71-d5d4-44a1-923a-e238b35bb2b1&is_advertising=true&ad_domain=VQCATCORE_LST&ad_position=8&ad_click_id=MWRiZjFkNWItMzhkYy00Mjg4LTlmMzItOGM5MmQwOTUwZDU2"><img src="https://http2.mlstatic.com/D_NQ_NP_968405-MLB46586324827_072021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Cabo Carregador Micro Usb</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-2012169907-carregador-tomada-plug-adaptador-fonte-usb-5v-20a-bivolt-_JM#position=9&search_layout=stack&type=pad&tracking_id=b5581a28-4201-43aa-aae7-51d7b4fc0720&is_advertising=true&ad_domain=VQCATCORE_LST&ad_position=9&ad_click_id=NzY5ZWQxY2EtMTkxYy00ZjQ3LTliZjgtMzkxMmFhOTQ1ZjU3"><img src="https://http2.mlstatic.com/D_NQ_NP_746337-MLB47387323442_092021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Carregador Usb 5v 2A</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-1992594677-motor-dc-pdvd-c-clump-_JM#position=1&search_layout=stack&type=item&tracking_id=4c89883c-e7af-4b0e-b7ad-7df39b293cd2"><img src="https://http2.mlstatic.com/D_NQ_NP_845648-MLB47249057175_082021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Motor Dc</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-2043197044-esp32-doit-devkit-com-esp32-wroom-32d-e-certif-anatel-_JM#position=3&search_layout=grid&type=item&tracking_id=2c3c3ec4-ef59-470a-995f-79bef8f8936f"><img src="https://http2.mlstatic.com/D_NQ_NP_880203-MLB47737034800_102021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Esp32</b></sub></a><br />
        </td>        
    </tr>
    <tr>
        <td align="center"><a href="https://www.mercadolivre.com.br/google-home-mini-com-asistente-virtual-google-assistant-charcoal-110v220v/p/MLB15541915?pdp_filters=category:MLB278167#searchVariation=MLB15541915&position=2&search_layout=grid&type=product&tracking_id=fd2918d9-8ccd-4ffd-881c-e407e1b52d0d"><img src="https://http2.mlstatic.com/D_NQ_NP_868365-MLA45255589079_032021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Google Home Mini</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-750803825-dht11-modulo-sensor-temp-umidade-arduino-esp8266-_JM?matt_tool=40343894&matt_word=&matt_source=google&matt_campaign_id=14303413655&matt_ad_group_id=125984293117&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539354956680&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=325976021&matt_product_id=MLB750803825&matt_product_partition_id=1404886571418&matt_target_id=aud-1457490208988:pla-1404886571418&gclid=CjwKCAiArOqOBhBmEiwAsgeLmamL3b46VP1gsx7S2CZZ2odSHwNrvpDnzL7aIVHiAol9rn9POd-mxBoCIXEQAvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_972488-MLB44509250054_012021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Dht11 Modulo Sensor Temp Umidade</b></sub></a><br />
        </td>  
        <td align="center"><a href="https://www.vespoliprofumi.com/it/linea-bambini-walt-disney/4524-minions-bagnoschiuma-3d-per-bambini-500-ml-con-dispenser-663350066289.html"><img src="https://m.media-amazon.com/images/I/61dhQkcKGhL._AC_SY550_.jpg" width="100px;" alt="" /><br /><sub><b>1-Recipiente de sabão líquido dos minions</b></sub></a><br />
        </td>
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-2001780570-mini-chapeu-de-palha-boneca-dog-para-decoraco-12cm-loja-_JM?matt_tool=58936140&matt_word=&matt_source=google&matt_campaign_id=14300471974&matt_ad_group_id=127611133362&matt_match_type=&matt_network=g&matt_device=c&matt_creative=539425454119&matt_keyword=&matt_ad_position=&matt_ad_type=pla&matt_merchant_id=485086757&matt_product_id=MLB2001780570&matt_product_partition_id=1397999322603&matt_target_id=pla-1397999322603&gclid=CjwKCAiArOqOBhBmEiwAsgeLmQKhPbiOyBsXihZ57dU2UC5CT7eD2qIJbhmy8v5kflBRG1RbKav-_xoCs9sQAvD_BwE"><img src="https://http2.mlstatic.com/D_NQ_NP_895777-MLB47273532723_082021-O.webp" width="100px;" alt="" /><br /><sub><b>1-Chapéu de palha</b></sub></a><br />
        </td>
    </tr>
    <tr>        
        <td align="center"><a href="https://produto.mercadolivre.com.br/MLB-2130299507-brinquedo-lanca-gira-helice-pirocoptero-com-luz-crianca-_JM#position=3&search_layout=grid&type=item&tracking_id=505acd45-b47c-4456-83e0-84137f18b30d"><img src="https://http2.mlstatic.com/D_NQ_NP_867104-MLB49155003648_022022-O.webp" width="100px;" alt="" /><br /><sub><b>1-Hélice de pirulito pirocoptero</b></sub></a><br />
        </td>
        <td align="center"><a href="https://www.filipeflop.com/produto/modulo-cartao-micro-sd/"><img src="https://www.filipeflop.com/wp-content/uploads/2017/07/SKU122168a.jpg" width="100px;" alt="" /><br /><sub><b>1-Módulo Cartão Micro SD</b></sub></a><br />
        </td>
        <td align="center"><a href="https://shopee.com.br/sou%E2%98%AC-CJMCU-1334-DAC-Module-UDA1334A-Stereo-Decoder-Board-I2S-Output-Interface-Sound-Frequency-Decoding-Module-for-3.3V-to-5V-i.290382738.11917876698"><img src="https://tse2.mm.bing.net/th?id=OIP.3tM_B4FqtTu4CucxvSPF9QHaHa&pid=Api&P=0&h=180" width="100px;" alt="" /><br /><sub><b>1-UDA1334A</b></sub></a><br />
        </td>
    </tr>
</table>

## Bugs 🐛

- Em caso de encontrar algum bug, abra um pull request

![Sick](https://cdn180.picsart.com/231018569019212.png?to=crop&type=webp&r=264x480&q=85)
![Doctor](../others/imgs/minion-doctor.png)

## Lembrem-se:

**Devemos:**
* Lavar bem as mãos 💦👏🙌
* Evitar aglomerações
    ![Aglomeracao](../others/imgs/aglomeracao.gif)
* Usar máscara 😷

A pandemia vai passar!

**💉 VACINA SIM 💉**

