@echo off
setlocal ENABLEDELAYEDEXPANSION

:: ===========================================================
:: CONFIGURACOES GERAIS
:: ===========================================================
set CHIP=esp32
set DATA_DIR=.\data
set SPEED=921600
set MAX_RETRIES=3
set IMAGE=.\littlefs.bin

:: Offset padrao caso NAO seja possivel ler da partitions.csv
set ESP32_FS_OFFSET=0x290000

:: Opcional: aponte manualmente sua partitions.csv aqui (se quiser forcar)
:: set PARTITIONS_CSV=.\partitions.csv

echo.
echo === UPLOAD LittleFS (ESP32) ===

:: ===========================================================
:: DETECTAR PORTA COM (CH340/CP210x)
:: ===========================================================
for /f "tokens=1,2 delims=," %%A in ('
  powershell -NoProfile -Command ^
  "Get-WmiObject Win32_SerialPort | Where-Object { $_.Name -like '*CP210*' -or $_.Name -like '*CH340*' } | ForEach-Object { $_.DeviceID + ',' + $_.Name }"
') do (
  set PORT=%%A
  set PORT_NAME=%%B
)

if "%PORT%"=="" (
  echo ❌ Porta COM do conversor USB-Serial (CH340/CP210x) nao encontrada.
  echo    Conecte sua placa e tente novamente.
  pause
  exit /b 1
)
echo 🔌 Porta detectada: %PORT% (%PORT_NAME%)

:: ===========================================================
:: VALIDACOES INICIAIS
:: ===========================================================
if /I not "%CHIP%"=="esp32" (
  echo ❌ Este script foi preparado para ESP32. Ajuste a variavel CHIP se necessario.
  exit /b 1
)

if not exist "%DATA_DIR%" (
  echo ❌ Pasta %DATA_DIR% nao encontrada. Crie-a e adicione os arquivos do FS.
  exit /b 1
)

dir /b /a:-d "%DATA_DIR%" | findstr . >nul || (
  echo ❌ A pasta %DATA_DIR% esta vazia.
  exit /b 1
)

:: ===========================================================
:: LOCALIZAR MKLITTLEFS (core ESP32) OU NO PATH
:: ===========================================================
set "MKLITTLEFS="
where mklittlefs >nul 2>nul
if errorlevel 1 (
  for /f "usebackq delims=" %%P in (`
    powershell -NoProfile -Command ^
      "$base = Join-Path $env:LOCALAPPDATA 'Arduino15\\packages\\esp32\\tools\\mklittlefs';" ^
      "if (Test-Path $base) { Get-ChildItem -Path $base -Recurse -Filter mklittlefs.exe | Select-Object -First 1 -ExpandProperty FullName }"
  `) do set "MKLITTLEFS=%%P"
) else (
  for /f "delims=" %%P in ('where mklittlefs') do set "MKLITTLEFS=%%P"
)

if "%MKLITTLEFS%"=="" (
  echo ❌ mklittlefs nao encontrado.
  echo    Instale o core ESP32 no Arduino IDE/CLI ou adicione o mklittlefs ao PATH.
  exit /b 1
)
echo 🧩 mklittlefs: "%MKLITTLEFS%"

:: ===========================================================
:: DESCOBRIR partitions.csv E EXTRAIR OFFSET/SIZE DO LittleFS
:: Regras:
::  - Procura .\partitions*.csv (prioridade)
::  - Se PARTITIONS_CSV estiver setado, usa-o
::  - Busca por linha cujo name==littlefs OU subtype/type contenha 'littlefs'
::  - Aceita size em 0x..., NNNK, NNNM ou decimal
:: ===========================================================
set "FS_OFFSET_HEX="
set "FS_SIZE_DEC="

for /f "usebackq tokens=1,* delims==" %%K in (`
  powershell -NoProfile -Command ^
    "$path = $env:PARTITIONS_CSV;" ^
    "if (-not $path) {" ^
    "  $cands = Get-ChildItem -Path '.' -Filter 'partitions*.csv' -File | Select-Object -First 1;" ^
    "  if ($cands) { $path = $cands.FullName }" ^
    "}" ^
    "if ($path -and (Test-Path $path)) {" ^
    "  $lines = Get-Content $path | Where-Object { $_ -and -not $_.Trim().StartsWith('#') };" ^
    "  $rows = foreach ($ln in $lines) {" ^
    "    $c = $ln.Split(',');" ^
    "    if ($c.Count -ge 5) {" ^
    "      [pscustomobject]@{name=$c[0].Trim(); type=$c[1].Trim(); subtype=$c[2].Trim(); offset=$c[3].Trim(); size=$c[4].Trim()}" ^
    "    }" ^
    "  };" ^
    "  $hit = $rows | Where-Object { $_.name -match '^littlefs$' -or $_.subtype -match 'littlefs' -or $_.type -match 'littlefs' } | Select-Object -First 1;" ^
    "  if ($hit) {" ^
    "    function To-Bytes($s) {" ^
    "      if ($s -match '^0x[0-9a-fA-F]+$') { return [Convert]::ToInt64($s,16) }" ^
    "      elseif ($s -match '^[0-9]+[mM]$') { return [int64]($s.TrimEnd('m','M')) * 1024 * 1024 }" ^
    "      elseif ($s -match '^[0-9]+[kK]$') { return [int64]($s.TrimEnd('k','K')) * 1024 }" ^
    "      elseif ($s -match '^[0-9]+$')     { return [int64]$s }" ^
    "      else { throw 'Formato de tamanho desconhecido: ' + $s }" ^
    "    }" ^
    "    $sizeDec = To-Bytes $hit.size;" ^
    "    $offHex  = if ($hit.offset -match '^0x') { $hit.offset } else { '0x' + ([Convert]::ToString([int64]$hit.offset,16)) };" ^
    "    Write-Output ('FS_OFFSET_HEX='+$offHex);" ^
    "    Write-Output ('FS_SIZE_DEC='+$sizeDec);" ^
    "  }"
    "}"
`) do (
  if /I "%%K"=="FS_OFFSET_HEX" set "FS_OFFSET_HEX=%%L"
  if /I "%%K"=="FS_SIZE_DEC"  set "FS_SIZE_DEC=%%L"
)

if defined FS_OFFSET_HEX (
  echo 📌 partitions.csv detectada: Offset=%FS_OFFSET_HEX%  Tam(bytes)=%FS_SIZE_DEC%
) else (
  echo ⚠️  partitions.csv nao encontrada ou particao 'littlefs' nao localizada.
  echo    Usando offset padrao: %ESP32_FS_OFFSET%
  set "FS_OFFSET_HEX=%ESP32_FS_OFFSET%"
)

:: ===========================================================
:: GERAR IMAGEM LITTLEFS
::  - Se FS_SIZE_DEC foi encontrado, usa -s com esse tamanho
::  - Senao, gera sem -s (muitos builds aceitam; mas o ideal e informar)
:: ===========================================================
echo.
echo 🛠️  Gerando imagem LittleFS a partir de "%DATA_DIR%"...
if defined FS_SIZE_DEC (
  "%MKLITTLEFS%" -c "%DATA_DIR%" -b 4096 -p 256 -s %FS_SIZE_DEC% "%IMAGE%"
) else (
  "%MKLITTLEFS%" -c "%DATA_DIR%" -b 4096 -p 256 "%IMAGE%"
)

if errorlevel 1 (
  echo ❌ Falha ao gerar a imagem LittleFS.
  exit /b 1
)
if not exist "%IMAGE%" (
  echo ❌ A imagem LittleFS nao foi criada: %IMAGE%
  exit /b 1
)
for %%F in ("%IMAGE%") do set IMAGE_SIZE=%%~zF
echo ✅ Imagem criada: %IMAGE% (%IMAGE_SIZE% bytes)

:: ===========================================================
:: PREPARAR MODO BOOTLOADER + AVISO POR VOZ
:: ===========================================================
powershell -NoProfile -Command ^
  "Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('Coloque a placa em modo de gravacao. Pressione o botao Boot se necessario.');"
echo ▶️  Coloque a placa em modo de gravacao (BOOT/EN conforme sua placa).
pause

:: ===========================================================
:: UPLOAD COM TENTATIVAS (esptool.py)
:: ===========================================================
where esptool.py >nul 2>&1
if errorlevel 1 (
  echo ❌ esptool.py nao encontrado no PATH.
  echo    Instale com:  pip install esptool
  exit /b 1
)

set /a count=1
:UPLOAD_TRY
echo.
echo 🚀 Gravando LittleFS (tentativa !count! de %MAX_RETRIES%)...
esptool.py --chip esp32 --port %PORT% --baud %SPEED% write_flash %FS_OFFSET_HEX% "%IMAGE%"
if errorlevel 1 (
  echo ❌ Falha no upload do LittleFS.
  set /a count+=1
  if !count! LEQ %MAX_RETRIES% (
    echo 🔁 Tentando novamente em 2s...
    timeout /t 2 >nul
    goto UPLOAD_TRY
  ) else (
    powershell -NoProfile -Command ^
      "Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('O upload do sistema de arquivos falhou.');"
    exit /b 1
  )
)

echo ✅ Upload concluido com sucesso!
powershell -NoProfile -Command ^
  "Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('Upload concluido com sucesso.');"
pause
exit /b 0
