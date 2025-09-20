@echo off
setlocal ENABLEDELAYEDEXPANSION

:: ===== CONFIG (ESP32) =====
:: FQBN (board genérico ESP32)
set FQBN=esp32:esp32:esp32

:: Diretório do sketch (raiz do projeto)
set SKETCH_DIR=.

:: Tamanho do LittleFS (ajuste conforme sua partitions.csv)
:: Exemplos comuns: 0x100000 (1MB), 0x120000 (~1.125MB), 0x1E0000 (~1.875MB)
set LFS_SIZE_HEX=0x120000

:: Parâmetros do sistema de arquivos
set LFS_PAGE=256
set LFS_BLOCK=4096

echo.
echo === Compilando (ESP32) ===
arduino-cli core list | findstr /i "esp32:esp32" >nul
if errorlevel 1 (
  echo Instalando core esp32...
  arduino-cli core update-index
  arduino-cli core install esp32:esp32 || (echo ❌ Falha ao instalar core ESP32. & exit /b 1)
)

echo.
echo > compile.log
arduino-cli compile --build-path ./meu_build --fqbn %FQBN% %SKETCH_DIR% --warnings all --verbose 1>> compile.log 2>&1
if errorlevel 1 (
  echo ❌ Erro na compilacao. Veja compile.log
  exit /b 1
)
echo ✅ Sketch compilado com sucesso.

:: ===== LittleFS (gerar imagem) =====
:: Procurar mklittlefs no PATH
where mklittlefs >nul 2>nul
if errorlevel 1 (
  :: Se não está no PATH, tentar localizar na instalação Arduino15 (core ESP32)
  set "MKLFS_EXE="
  for /f "usebackq delims=" %%P in (`
    powershell -NoProfile -Command ^
      "$base = Join-Path $env:LOCALAPPDATA 'Arduino15\packages\esp32\tools\mklittlefs';" ^
      "if (Test-Path $base) { Get-ChildItem -Path $base -Recurse -Filter mklittlefs.exe | Select-Object -First 1 -ExpandProperty FullName }"
  `) do (
    set "MKLFS_EXE=%%P"
  )
  if "%MKLFS_EXE%"=="" (
    echo ❌ mklittlefs nao encontrado.
    echo    Dica: o core esp32 costuma instalar em:
    echo    %%LOCALAPPDATA%%\Arduino15\packages\esp32\tools\mklittlefs\*\mklittlefs.exe
    echo    Adicione ao PATH ou ajuste o script para chamar por caminho absoluto.
    exit /b 1
  )
) else (
  for /f "delims=" %%P in ('where mklittlefs') do set "MKLFS_EXE=%%P"
)

echo 🧩 mklittlefs: "%MKLFS_EXE%"

:: Verificar pasta data/ (na raiz do projeto)
if not exist "data\" (
  echo ❌ A pasta .\data nao existe. Crie-a e adicione os arquivos do FS.
  exit /b 1
)
dir /b /a:-d "data" | findstr . >nul || (
  echo ❌ A pasta .\data esta vazia.
  exit /b 1
)

echo.
echo === Gerando imagem LittleFS (ESP32) ===
:: Para ESP32, o mklittlefs aceita -s; use o valor que bate com sua particao
"%MKLFS_EXE%" -c "data" -p %LFS_PAGE% -b %LFS_BLOCK% -s %LFS_SIZE_HEX% "meu_build\data\.littlefs.bin"
if errorlevel 1 (
  echo ❌ Falha ao gerar meu_build\data\.littlefs.bin
  exit /b 1
)

if not exist "meu_build\data\.littlefs.bin" (
  echo ❌ A imagem nao foi criada (meu_build\data\.littlefs.bin)
  exit /b 1
)

for %%F in ("meu_build\data\.littlefs.bin") do set IMAGE_SIZE=%%~zF
echo ✅ LittleFS OK: meu_build\data\.littlefs.bin  (tamanho: %IMAGE_SIZE% bytes)

echo.
echo 💡 Pronto! (compile.log contem detalhes)
exit /b 0
