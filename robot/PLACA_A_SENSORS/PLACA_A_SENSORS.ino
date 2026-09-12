//==================================================
// PLACA A - SENSORES, SERVO, OLED, SPIFFS  (v5 - con debug)
//==================================================
// Placa: Keyestudio ESP32 Plus STEAMakers
//
// CONEXION ENTRE PLACAS (etiquetas DIGITAL de tu placa):
//   Placa A D2 -----> Placa B D2
//   Placa A D5 <----- Placa B D5
//   Placa A GND ------ Placa B GND
//
// Comandos enviados A -> B:
//   'F' = avanza              -> LED verde, motores adelante
//   'P' = pausa (escaneando)  -> LED verde, motores parados
//   'S' = stop definitivo     -> LED rojo,  motores parados
//   'L' = gira izquierda      -> LED amarillo, 5s, luego F
//   'R' = gira derecha        -> LED amarillo, 5s, luego F
//
// Comandos recibidos B -> A:
//   'D' = Done, el giro ha terminado
//==================================================

#include <ESP32Servo.h>
#include <DFRobot_DHT11.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <SPIFFS.h>
#include <FS.h>

//==================================================
// DEBUG
//==================================================
#define DEBUG 1  // pon a 0 para silenciar el Monitor Serie

void dbg(const char* tipo, String msg) {
#if DEBUG
  Serial.print("[A][t=");
  Serial.print(millis());
  Serial.print("ms][");
  Serial.print(tipo);
  Serial.print("] ");
  Serial.println(msg);
#endif
}

//==================================================
// UART2 - COMUNICACION CON PLACA B
//==================================================
#define UART2_TX 26  // etiqueta D2 en tu placa
#define UART2_RX 16  // etiqueta D5 en tu placa
HardwareSerial ComBoard(2); // UART2

//==================================================
// DURACION DEL GIRO Y TIMEOUT DE SEGURIDAD
//==================================================
const unsigned long TURN_DURATION_MS = 5000;
const unsigned long TURN_TIMEOUT_MS = TURN_DURATION_MS + 3000;

//==================================================
// SERVO (cabeza / sonar)
//==================================================
Servo servo1;

//==================================================
// SENSOR ULTRASONIDOS HC-SR04
//==================================================
int trig = 17;
int echo = 25;

float temps = 0;
float dist = 0;

const float SIN_OBJECTE = 999.0;
const float LLINDAR_CM = 20.0;

//==================================================
// DHT11
//==================================================
DFRobot_DHT11 DHT;
#define DHT11_PIN 27

//==================================================
// OLED SSD1306
//==================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

bool oledOK = false;
bool bloquejat = false;

unsigned long ultimGuardat = 0;
const unsigned long INTERVAL_GUARDAT_MS = 5000;

unsigned long numVolta = 0;

//==================================================
// MEDIR DISTANCIA (con timeout)
//==================================================
float llegirDistancia() {

  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  temps = pulseIn(echo, HIGH, 25000UL);

  if (temps == 0) {
    dbg("SENSOR", "Sonar: sin eco (timeout) -> sin objeto");
    return SIN_OBJECTE;
  }

  float d = temps / 29.15 / 2.0;
  return d;
}

//==================================================
// MOSTRAR DATOS PRINCIPALES (frontal + temp/hum)
//==================================================
void mostrarOLED(float distancia) {

  if (!oledOK) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ROBOT ESP32");

  display.setCursor(0, 18);
  display.print("Temp: ");
  display.print(DHT.temperature);
  display.println(" C");

  display.setCursor(0, 34);
  display.print("Hum: ");
  display.print(DHT.humidity);
  display.println(" %");

  display.setCursor(0, 50);
  display.print("Dist frontal: ");
  display.print(distancia);
  display.println("cm");

  display.display();
}

void mostrarEscaneig(const char* etiqueta, float distancia) {

  if (!oledOK) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Escanejant...");

  display.setCursor(0, 24);
  display.println(etiqueta);

  display.setCursor(0, 44);
  if (distancia >= SIN_OBJECTE) {
    display.println("Sense objecte");
  } else {
    display.print(distancia);
    display.println(" cm");
  }

  display.display();
}

void mostrarBloquejat() {

  if (!oledOK) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ROBOT ESP32");
  display.setCursor(0, 24);
  display.println("BLOQUEJAT!");
  display.setCursor(0, 40);
  display.println("Reinicia per seguir");
  display.display();
}

//==================================================
// GUARDAR DATOS EN SPIFFS (cada 5 s)
//==================================================
void guardarDades() {

  if (millis() - ultimGuardat < INTERVAL_GUARDAT_MS) return;
  ultimGuardat = millis();

  File fitxer = SPIFFS.open("/dades.txt", FILE_APPEND);

  if (fitxer) {
    fitxer.print("Temperatura: ");
    fitxer.print(DHT.temperature);
    fitxer.print(" C   ");
    fitxer.print("Humitat: ");
    fitxer.print(DHT.humidity);
    fitxer.println(" %");
    fitxer.close();
    dbg("SPIFFS", "Datos guardados en /dades.txt");
  } else {
    dbg("SPIFFS", "ERROR: no se pudo abrir /dades.txt");
  }
}

//==================================================
// ENVIAR ORDEN A LA PLACA B
//==================================================
void enviarOrden(char orden) {
  ComBoard.write(orden);
  String m = "TX -> Placa B: '";
  m += orden;
  m += "'";
  dbg("UART", m);
}

//==================================================
// ESPERAR CONFIRMACION 'D' DE LA PLACA B TRAS UN GIRO
//==================================================
void esperarFiGir() {

  unsigned long inici = millis();
  bool rebut = false;

  dbg("SYNC", "Esperando confirmacion 'D' de Placa B...");

  while (millis() - inici < TURN_TIMEOUT_MS) {

    if (ComBoard.available() > 0) {
      char c = ComBoard.read();
      if (c == 'D') {
        rebut = true;
        break;
      } else {
        String m = "Byte inesperado mientras esperaba 'D': '";
        m += c;
        m += "'";
        dbg("UART", m);
      }
    }
    delay(20);
  }

  unsigned long transcurrido = millis() - inici;

  if (rebut) {
    String m = "Gir completat, confirmacio rebuda en ";
    m += transcurrido;
    m += "ms";
    dbg("SYNC", m);
  } else {
    String m = "AVISO: timeout esperando 'D' (";
    m += transcurrido;
    m += "ms). Compruebalos cables D5<->D5 y GND. Continuo igualment.";
    dbg("SYNC", m);
  }
}

//==================================================
// SETUP
//==================================================
void setup() {

  Serial.begin(9600);
  delay(300);
  dbg("BOOT", "Placa A arrancando...");

  ComBoard.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  dbg("BOOT", "UART2 iniciado (RX=16/D5, TX=26/D2)");

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo1.setPeriodHertz(50);
  servo1.attach(14, 500, 2400);
  dbg("BOOT", "Servo cabeza inicializado (pin 14)");

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  Wire.begin();

  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    oledOK = true;
    dbg("BOOT", "OLED OK");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Sistema iniciat");
    display.display();
  } else {
    dbg("BOOT", "OLED FALLO - seguimos sin pantalla (no se cuelga el robot)");
  }

  if (!SPIFFS.begin(true)) {
    dbg("BOOT", "SPIFFS FALLO");
  } else {
    dbg("BOOT", "SPIFFS OK");
  }

  dbg("BOOT", "Setup completo. Umbral deteccion = 30cm. Giro = 5000ms.");

  delay(2000);
}

//==================================================
// LOOP
//==================================================
void loop() {

  numVolta++;

  //------------------------------------------------
  // ESTADO BLOQUEADO definitivo
  //------------------------------------------------
  if (bloquejat) {
    enviarOrden('S');
    dbg("STATE", "BLOQUEJAT permanente - reinicia la placa para salir");
    delay(1000);
    return;
  }

  dbg("LOOP", "----- vuelta #" + String(numVolta) + " -----");

  servo1.write(90);
  delay(300);

  DHT.read(DHT11_PIN);

  dist = llegirDistancia();

  String m = "Distancia frontal: ";
  m += dist;
  m += " cm (umbral ";
  m += LLINDAR_CM;
  m += "cm)";
  dbg("SENSOR", m);

  dbg("SENSOR", "Temp=" + String(DHT.temperature) + "C  Hum=" + String(DHT.humidity) + "%");

  mostrarOLED(dist);
  guardarDades();

  if (dist < LLINDAR_CM) {

    dbg("DECISION", "Objeto delante -> pausa y escaneo lateral");
    enviarOrden('P');

    // --- Mirar a la DERECHA ---
    servo1.write(0);
    delay(900);

    dist = llegirDistancia();
    dbg("SENSOR", "Distancia derecha: " + String(dist) + " cm");
    mostrarEscaneig("Dreta", dist);
    delay(400);

    if (dist < LLINDAR_CM) {

      dbg("DECISION", "Derecha ocupada -> escaneo izquierda");

      servo1.write(180);
      delay(900);

      dist = llegirDistancia();
      dbg("SENSOR", "Distancia izquierda: " + String(dist) + " cm");
      mostrarEscaneig("Esquerra", dist);
      delay(400);

      if (dist < LLINDAR_CM) {

        servo1.write(90);
        delay(300);

        dbg("DECISION", "Objeto en las 3 direcciones -> BLOQUEO DEFINITIVO");

        enviarOrden('S');
        mostrarBloquejat();
        bloquejat = true;
      }
      else {

        servo1.write(90);
        delay(300);

        dbg("DECISION", "Izquierda libre -> gira izquierda");
        enviarOrden('L');

        esperarFiGir();
      }
    }
    else {

      servo1.write(90);
      delay(300);

      dbg("DECISION", "Derecha libre -> gira derecha");
      enviarOrden('R');

      esperarFiGir();
    }
  }
  else {

    dbg("DECISION", "Via libre -> avanzar");
    enviarOrden('F');
  }

  delay(500);
}

