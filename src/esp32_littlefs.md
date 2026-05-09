# ESP32 LittleFS Flash Error — Correção

## Erro

```
A fatal error occurred: File .pio\build\upesy_wroom\littlefs.bin (length 1572864)
at offset 6750208 will not fit in 4194304 bytes of flash.
Change the --flash_size argument, or flashing address.
*** [uploadfs] Error 2
```

## Causa Raiz

A tabela de partições está configurada para um flash maior do que os 4MB disponíveis no chip. O arquivo LittleFS (1,5MB) no offset `0x670000` (6.750.208 bytes) ultrapassa o limite de 4MB (4.194.304 bytes).

---

## Soluções

### Opção 1: Alterar o esquema de partições (mais comum)

No `platformio.ini`, defina um esquema compatível com 4MB de flash:

```ini
[env:upesy_wroom]
platform = espressif32
board = upesy_wroom
framework = arduino
board_build.filesystem = littlefs
board_build.partitions = min_spiffs.csv   ; ou "default", "huge_app", "no_ota"
```

Esquemas integrados para flash de 4MB:

| Esquema          | App          | SPIFFS/LittleFS |
|------------------|--------------|-----------------|
| `default.csv`    | 1,25MB×2 OTA | 1MB             |
| `min_spiffs.csv` | 1,9MB×2 OTA  | 190KB           |
| `huge_app.csv`   | 3MB (sem OTA)| 1MB             |
| `no_ota.csv`     | 2MB (sem OTA)| 2MB             |

> Para um filesystem grande, **`no_ota.csv`** oferece o maior espaço para LittleFS.

---

### Opção 2: Tabela de partições personalizada

Crie o arquivo `partitions.csv` na raiz do projeto:

```csv
# Name,   Type, SubType, Offset,  Size,    Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xE000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x1C0000,
spiffs,   data, spiffs,  0x1D0000,0x230000,
```

E no `platformio.ini`:

```ini
board_build.partitions = partitions.csv
```

---

### Opção 3: Reduzir o tamanho da imagem do filesystem

A pasta `/data` contém **18 arquivos** — verifique o tamanho dos maiores (`.webp`, `.jpg`, `.png`):

```bash
# Verificar tamanhos antes de enviar
ls -lh data/
```

Considere:
- Comprimir imagens
- Servir arquivos grandes externamente (CDN)
- Compactar HTML/CSS/JS com Gzip

---

## Recomendação Rápida

Se não precisar de OTA, adicione ao `platformio.ini`:

```ini
board_build.partitions = no_ota.csv
```

Isso fornece uma partição LittleFS de 2MB, suficiente para a imagem de 1,5MB.
