// CryptoHandler.h
#ifndef CRYPTO_HANDLER_H
#define CRYPTO_HANDLER_H

#ifdef debug
#undef debug
#endif

#include <AESLib.h>
#include "Config.h"

#define MAX_AES_BUFFER 16

#ifndef AES_KEY_HEX
#define AES_KEY_HEX "000102030405060708090A0B0C0D0E0F"  // fallback
#endif

#ifndef AES_IV_HEX
#define AES_IV_HEX "A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"   // fallback
#endif

// Instância AES (pode ser compartilhada)
extern AESLib aesLib;

class CryptoHandler {
public:
    CryptoHandler();

    // Criptografa uma String e retorna a saída como hexadecimal
    const char* encrypt(String plain);

    // Descriptografa uma String em hexadecimal e retorna o texto original
    const char* decrypt(String hexEncrypted, int length);
    // Função auxiliar para converter hex em bytes
    bool hexStringToBytes(const String& hex, unsigned char* bytes, int& length);

private:
    // Buffers internos
    char encrypted[MAX_BUFFER];
    char decrypted[MAX_BUFFER];

    // Chave e IV em byte[] (definido dinamicamente a partir do HEX do pipeline)
    byte aes_key[MAX_AES_BUFFER];
    byte aes_iv[MAX_AES_BUFFER];
    byte aes_iv_original[MAX_AES_BUFFER];

    // Função de inicialização
    void initAES();
};

#endif
