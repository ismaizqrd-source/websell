#!/usr/bin/env bash
# ==================================================
# probar.sh - compilar, subir y leer el log del robot
# ==================================================
# Necesita arduino-cli en el ordenador que tiene el
# robot conectado por USB.  Uso:
#
#   ./probar.sh deps                 instala core ESP32 y librerias
#   ./probar.sh puertos              lista las placas conectadas
#   ./probar.sh subir TEST_PLACA_A /dev/ttyUSB0
#   ./probar.sh log /dev/ttyUSB0 60  guarda 60 s de Monitor Serie
#   ./probar.sh todo TEST_PLACA_A /dev/ttyUSB0
# ==================================================
set -euo pipefail

FQBN="${FQBN:-esp32:esp32:esp32}"   # exporta FQBN=... si tu placa es otra
BAUD=9600
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "ERROR: no encuentro arduino-cli."
  echo "Instalalo desde https://arduino.github.io/arduino-cli/latest/installation/"
  exit 1
fi

deps() {
  echo ">> Anadiendo el indice de placas ESP32..."
  arduino-cli config add board_manager.additional_urls \
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json || true
  arduino-cli core update-index
  echo ">> Instalando el core ESP32 (tarda un rato la primera vez)..."
  arduino-cli core install esp32:esp32
  echo ">> Instalando librerias..."
  arduino-cli lib install "ESP32Servo"
  arduino-cli lib install "DFRobot_DHT11"
  arduino-cli lib install "Adafruit GFX Library"
  arduino-cli lib install "Adafruit SSD1306"
  echo ">> Listo."
}

puertos() {
  echo ">> Placas detectadas:"
  arduino-cli board list
  echo
  echo "Si no aparece ninguna: prueba otro cable USB (muchos son solo de carga)"
  echo "y comprueba los drivers CP210x o CH340 de la placa."
}

compilar() {
  local sketch="$1"
  echo ">> Compilando $sketch para $FQBN ..."
  arduino-cli compile --fqbn "$FQBN" "$DIR/$sketch"
  echo ">> Compila sin errores."
}

subir() {
  local sketch="$1" puerto="$2"
  compilar "$sketch"
  echo ">> Subiendo a $puerto ..."
  arduino-cli upload -p "$puerto" --fqbn "$FQBN" "$DIR/$sketch"
  echo ">> Subido."
}

log() {
  local puerto="$1" segundos="${2:-60}"
  local fichero="$DIR/log_$(date +%H%M%S).txt"
  echo ">> Escuchando $puerto a $BAUD durante $segundos s -> $fichero"
  echo ">> (escribe aqui el numero de la prueba que quieras lanzar)"
  timeout "$segundos" arduino-cli monitor -p "$puerto" -c "baudrate=$BAUD" \
    | tee "$fichero" || true
  echo
  echo ">> Guardado en $fichero"
  echo ">> Pega ese fichero en el chat y te digo que falla."
}

case "${1:-}" in
  deps)     deps ;;
  puertos)  puertos ;;
  compilar) compilar "${2:?falta el sketch}" ;;
  subir)    subir "${2:?falta el sketch}" "${3:?falta el puerto}" ;;
  log)      log "${2:?falta el puerto}" "${3:-60}" ;;
  todo)     subir "${2:?falta el sketch}" "${3:?falta el puerto}"; sleep 2; log "$3" 120 ;;
  *)
    sed -n '2,16p' "${BASH_SOURCE[0]}"
    exit 1 ;;
esac
