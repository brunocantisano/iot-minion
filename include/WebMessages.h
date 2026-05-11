const char WRONG_CLIMATE[] PROGMEM = "Erro desconhecido ao buscar temperatura e umidade";
const char PLAYED[] PROGMEM = "Arquivo foi colocado para tocar.";
const char URL_PLAYED[] PROGMEM = "A url foi colocada para tocar.";
const char API_TOKEN_CHAT_GPT_ERROR[] PROGMEM = "Erro ao utilizar  chave de api do chat GPT.";
const char URL_CHAT_GPT[] PROGMEM = "Erro ao analisar a resposta JSON do ChatGPT";
const char ERRO_CONEXAO[] PROGMEM = "Falha na conexão";
const char NOT_PLAYED[] PROGMEM = "Não foi possível tocar o áudio.";
const char NOT_LOADED_AUDIO[] PROGMEM = "Não foi possível carregar a biblioteca de áudio.";
const char WRONG_AUTHORIZATION[] PROGMEM = "Authorization token errado";
const char NOT_AUTHORIZED_EXTENTIONS[] PROGMEM = "Extensão de arquivo inválida para upload";
const char SDCARD_PHOTO_WRITTEN[] PROGMEM = "Imagem salva no cartão SD com sucesso";
const char WRONG_STATUS[] PROGMEM = "Erro ao atualizar o status";
const char EXISTING_ITEM[] PROGMEM = "Item já existente na lista";
const char REMOVED_ITEM[] PROGMEM = "Item removido da lista";
const char REMOVED_FILE[] PROGMEM = "Arquivo removido";
const char UPLOADED_FILE[] PROGMEM = "Arquivo criado com sucesso";
const char NOT_FOUND_ITEM[] PROGMEM = "Item não encontrado na lista";
const char NOT_FOUND_ROUTE[] PROGMEM = "Rota nao encontrada";
const char PARSER_ERROR[] PROGMEM = "{\"message\": \"Erro ao fazer parser do json\"}";
const char WEB_SERVER_CONFIG[] PROGMEM = "\nConfiguring Webserver ...";
const char WEB_SERVER_STARTED[] PROGMEM = "Webserver started";
const char HTML_MISSING_DATA_UPLOAD[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><title>Minion ESP8266-Garagem Digital</title>" 
                "<meta charset=\"utf-8\"><meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">" 
                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>"
                "<body><center><img src=\"http://www.imagenspng.com.br/wp-content/uploads/2015/07/minions-52-roxo.png\" width=\"128\"/> </center>"
                "<div class=\"container\">Lembre-se que para rodar a aplicação será necessário, previamente, instalar o plugin: "
                "<b><a src=\"https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/\">Install ESP8266 Filesystem Uploader in Arduino IDE\"</a></b>"
                " e utilizar o menu no Arduino IDE: <b>Ferramentas->ESP8266 Sketch Data Upload</b>"
                " para gravar o conteúdo do web server (pasta: <b>/data</b>) no <b>Storage</b>.</div></body></html>";
static const char* MSG_ARQUIVO_NAO_ENCONTRADO = "Provavelmente voce nao carregou os arquivos da pasta \"data\" (LittleFS) para o servidor!";

// HTTP status code aliases
#define HTTP_CODE_OK            HTTP_OK
#define HTTP_CODE_BAD_REQUEST   HTTP_BAD_REQUEST
#define HTTP_CODE_UNAUTHORIZED  HTTP_UNAUTHORIZED
#define HTTP_CODE_NOT_FOUND     HTTP_NOT_FOUND
#define HTTP_CODE_NO_CONTENT    HTTP_NO_CONTENT
#define HTTP_CODE_CONFLICT      HTTP_CONFLICT







