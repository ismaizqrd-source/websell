//==================================================
// PLACA B - MOTORES Y LEDS  (v5 - con debug)
//==================================================
// Placa: Keyestudio ESP32 Plus STEAMakers
//
// CONEXION ENTRE PLACAS (etiquetas DIGITAL de tu placa):
//   Placa B D2 <----- Placa A D2
//   Placa B D5 -----> Placa A D5
//   Placa B GND ------ Placa A GND
//
// Comandos recibidos A -> B:
//   'F' = avanza              -> LED verde, motores adelante
//   'P' = pausa (escaneando)  -> LED verde, motores parados
//   'S' = stop definitivo     -> LED rojo,  motores parados
//   'L' = gira izquierda      -> LED amarillo, 5s, luego F
//   'R' = gira derecha        -> LED amarillo, 5s, luego F
//
// Comandos enviados B -> A:
//   'D' = Done, el giro acaba de terminar
//==================================================

//==================================================
// DEBUG
//==================================================
#define DEBUG 1  // pon a 0 para silenciar el Monitor Serie

void dbg(const char* tipo, String msg) {
#if DEBUG
  Serial.print("[B][t=");
  Serial.print(millis());
  Serial.print("ms][");
  Serial.print(tipo);
  Serial.print("] ");
  Serial.println(msg);
#endif
}

//==================================================
// UART2 - COMUNICACION CON PLACA A
//==================================================
#define UART2_TX 16  // etiqueta D5 en tu placa
#define UART2_RX 26  // etiqueta D2 en tu placa
HardwareSerial ComBoard(2); // UART2

const unsigned long TURN_DURATION_MS = 5000;

//==================================================
// LEDS
//==================================================
int verd = 14;
int groc = 17;
int vermell = 25;

//==================================================
// MOTOR A (D13, D12, D11)
//==================================================
int ENA = 18;
int IN1 = 19;
int IN2 = 23;

//==================================================
// MOTOR B (D8, D10, D9)
//==================================================
int ENB = 12;
int IN3 = 5;
int IN4 = 13;

char ordenActual = 'S';
char ordenAnterior = 0;

//==================================================
// LEDS - helpers (con log de cambio de color)
//==================================================
String colorActual = "";

void fijarColor(String nom) {
  if (colorActual != nom) {
    dbg("LED", "-> " + nom);
    colorActual = nom;
  }
}

void ledVerd() {
  digitalWrite(vermell, LOW);
  digitalWrite(groc, LOW);
  digitalWrite(verd, HIGH);
  fijarColor("VERDE");
}

void ledGroc() {
  digitalWrite(vermell, LOW);
  digitalWrite(groc, HIGH);
  digitalWrite(verd, LOW);
  fijarColor("AMARILLO");
}

void ledVermell() {
  digitalWrite(vermell, HIGH);
  digitalWrite(groc, LOW);
  digitalWrite(verd, LOW);
  fijarColor("ROJO");
}

//==================================================
// MOTORES - helper para pararlos
//==================================================
void motorsParats() {
  digitalWrite(ENA, LOW);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(ENB, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

//==================================================
// FUNCIONES DE ESTADO
//==================================================

void endevant() {

  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(ENB, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledVerd();
}

void pausaEscaneig() {
  motorsParats();
  ledVerd();
}

void para() {
  motorsParats();
  ledVermell();
}

void buidarBufferUART() {
  int descartats = 0;
  while (ComBoard.available() > 0) {
    ComBoard.read();
    descartats++;
  }
  if (descartats > 0) {
    dbg("UART", "Buffer vaciado: " + String(descartats) + " byte(s) descartado(s)");
  }
}

void esquerra() {

  dbg("MOTOR", "Girando IZQUIERDA (5000ms)");

  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(ENB, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledGroc();

  unsigned long inici = millis();
  delay(TURN_DURATION_MS);
  unsigned long realDuration = millis() - inici;

  dbg("MOTOR", "Giro izquierda terminado, duracion real: " + String(realDuration) + "ms");

  buidarBufferUART();

  ComBoard.write('D');
  dbg("UART", "TX -> Placa A: 'D' (confirmacion fin de giro)");

  endevant();
  ordenActual = 'F';
}

void dreta() {

  dbg("MOTOR", "Girando DERECHA (5000ms)");

  digitalWrite(ENA, HIGH);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(ENB, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledGroc();

  unsigned long inici = millis();
  delay(TURN_DURATION_MS);
  unsigned long realDuration = millis() - inici;

  dbg("MOTOR", "Giro derecha terminado, duracion real: " + String(realDuration) + "ms");

  buidarBufferUART();

  ComBoard.write('D');
  dbg("UART", "TX -> Placa A: 'D' (confirmacion fin de giro)");

  endevant();
  ordenActual = 'F';
}

//==================================================
// SETUP
//==================================================
void setup() {

  Serial.begin(9600);
  delay(300);
  dbg("BOOT", "Placa B arrancando...");

  ComBoard.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  dbg("BOOT", "UART2 iniciado (RX=26/D2, TX=16/D5)");

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(verd, OUTPUT);
  pinMode(groc, OUTPUT);
  pinMode(vermell, OUTPUT);

  para();

  dbg("BOOT", "Setup completo. Esperando ordenes de Placa A...");
}

//==================================================
// LOOP
//==================================================
void loop() {

  if (ComBoard.available() > 0) {

    char recibido = ComBoard.read();

    if (recibido == 'F' || recibido == 'S' || recibido == 'P' ||
        recibido == 'L' || recibido == 'R') {

      ordenActual = recibido;

      if (ordenActual != ordenAnterior) {
        dbg("UART", "RX <- Placa A: '" + String(ordenActual) + "'");
        ordenAnterior = ordenActual;
      }
    } else {
      dbg("UART", "Byte desconocido recibido: '" + String(recibido) + "' (ignorado)");
    }
  }

  switch (ordenActual) {

    case 'F':
      endevant();
      break;

    case 'P':
      pausaEscaneig();
      break;

    case 'S':
      para();
      break;

    case 'L':
      esquerra();
      break;

    case 'R':
      dreta();
      break;
  }

  delay(50);
}

