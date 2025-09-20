// CryptoHandler.cpp
#include "CryptoHandler.h"

AESLib aesLib;

CryptoHandler::CryptoHandler() {
    initAES();
}

void CryptoHandler::initAES() {
    auto parseHex = [](const char* hex, byte* buffer, size_t maxLen) {
        for (size_t i = 0; i < maxLen && hex[i * 2] && hex[i * 2 + 1]; i++) {
            char byteStr[3] = { hex[i * 2], hex[i * 2 + 1], 0 };
            buffer[i] = (byte)strtoul(byteStr, nullptr, 16);
        }
    };

    parseHex(AES_KEY_HEX, aes_key, MAX_AES_BUFFER);
    parseHex(AES_IV_HEX, aes_iv, MAX_AES_BUFFER);
    memcpy(aes_iv_original, aes_iv, MAX_AES_BUFFER); 
}

const char* CryptoHandler::encrypt(String plain) {
    Serial.println("=== Criptografando com AES-128-CBC ===");

    memcpy(aes_iv, aes_iv_original, MAX_AES_BUFFER);

    static char hexOutput[MAX_BUFFER * 2 + 1]; // 2 caracteres por byte + '\0'
    memset(hexOutput, 0x00, sizeof(hexOutput));

    byte encryptedBuffer[MAX_BUFFER];
    memset(encryptedBuffer, 0x00, MAX_BUFFER);
    
    int cipherLen = aesLib.encrypt(
        (const byte*) plain.c_str(),
        plain.length(),
        encryptedBuffer,
        aes_key,
        sizeof(aes_key),
        aes_iv
    );

    if (cipherLen <= 0 || cipherLen > MAX_BUFFER) {
        Serial.println("Erro na criptografia");
        return "";
    }

    for (int i = 0; i < cipherLen; ++i) {
        sprintf(&hexOutput[i * 2], "%02x", encryptedBuffer[i]);
    }

    return hexOutput;
}

bool CryptoHandler::hexStringToBytes(const String& hex, unsigned char* bytes, int& length) {
    int hexLen = hex.length();
    if (hexLen % 2 != 0) return false;  // Tamanho inválido

    length = hexLen / 2;

    for (int i = 0; i < length; ++i) {
        char high = hex.charAt(2 * i);
        char low  = hex.charAt(2 * i + 1);

        // Verifica se ambos os caracteres são HEX válidos
        if (!isxdigit(high) || !isxdigit(low)) {
            return false;
        }

        // Converte cada caractere HEX em valor decimal
        auto hexCharToInt = [](char c) -> uint8_t {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return 0; // Nunca alcançado se validado antes
        };

        bytes[i] = (hexCharToInt(high) << 4) | hexCharToInt(low);
    }

    return true;
}

const char* CryptoHandler::decrypt(String hexEncrypted, int length) {
    Serial.println("=== Descriptografando AES-128-CBC ===");

    memcpy(aes_iv, aes_iv_original, MAX_AES_BUFFER);

    byte encryptedBuffer[MAX_BUFFER];
    memset(encryptedBuffer, 0x00, MAX_BUFFER);
    int len;
    if (!hexStringToBytes(hexEncrypted, encryptedBuffer, len)) {
        Serial.println("Hex inválido!");
        return "";
    }

    static char decryptedText[MAX_BUFFER];
    memset(decryptedText, 0x00, MAX_BUFFER);

    int plainLen = aesLib.decrypt(
        encryptedBuffer,
        len,
        (byte*)decryptedText,
        aes_key,
        sizeof(aes_key),
        aes_iv
    );

    if (plainLen <= 0 || plainLen >= MAX_BUFFER) {
        Serial.println("Erro na descriptografia ou tamanho inválido");
        return "";
    }
    
    decryptedText[length] = '\0'; // Garante terminação
    return decryptedText;
}
