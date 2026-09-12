//==================================================
// TEST PLACA A - BANCO DE PRUEBAS DE COMPONENTES
//==================================================
// Placa: Keyestudio ESP32 Plus STEAMakers
//
// Sube este sketch, abre el Monitor Serie a 9600
// y escribe la letra/numero de la prueba que quieras.
// NO mueve los motores: la Placa B no se toca aqui.
//
// PINES (los mismos que el codigo final):
//   Servo cabeza ..... GPIO 14
//   HC-SR04 TRIG ..... GPIO 17
//   HC-SR04 ECHO ..... GPIO 25
//   DHT11 ............ GPIO 27
//   OLED I2C ......... SDA 21 / SCL 22 (por defecto)
//   UART2 TX (D2) .... GPIO 26  -> va a D2 de Placa B
//   UART2 RX (D5) .... GPIO 16  <- viene de D5 de Placa B
//==================================================

#include <ESP32Servo.h>
#include <DFRobot_DHT11.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>
#include <FS.h>

#define UART2_TX 26
#define UART2_RX 16
HardwareSerial ComBoard(2);

#define PIN_SERVO 14
#define PIN_TRIG  17
#define PIN_ECHO  25
#define PIN_DHT   27

Servo servo1;
DFRobot_DHT11 DHT;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool oledOK = false;

//==================================================
void menu() {
  Serial.println();
  Serial.println("=========== TEST PLACA A ===========");
  Serial.println(" 1 = SERVO    (barrido 0-90-180)");
  Serial.println(" 2 = SONAR    (20 medidas seguidas)");
  Serial.println(" 3 = DHT11    (temperatura y humedad)");
  Serial.println(" 4 = I2C      (buscar direcciones)");
  Serial.println(" 5 = OLED     (dibujar en pantalla)");
  Serial.println(" 6 = SPIFFS   (escribir y leer fichero)");
  Serial.println(" 7 = UART     (enviar ordenes a Placa B)");
  Serial.println(" 8 = ECO UART (necesita TEST_PLACA_B)");
  Serial.println(" 9 = TODO     (1 a 6 en secuencia)");
  Serial.println(" m = volver a mostrar este menu");
  Serial.println("====================================");
  Serial.print("> ");
}

//==================================================
// 1 - SERVO
//==================================================
void testServo() {
  Serial.println("\n--- SERVO (GPIO 14) ---");
  Serial.println("MIRA la cabeza del robot mientras se mueve.");

  int angulos[] = {90, 0, 45, 90, 135, 180, 90};
  for (int i = 0; i < 7; i++) {
    Serial.print("  -> ");
    Serial.print(angulos[i]);
    Serial.println(" grados");
    servo1.write(angulos[i]);
    delay(900);
  }

  Serial.println("RESULTADO:");
  Serial.println("  Se mueve a todas las posiciones .... SERVO OK");
  Serial.println("  Tiembla o zumba sin moverse ........ FALTA CORRIENTE");
  Serial.println("     (alimenta el servo aparte, NO del 5V de la placa,");
  Serial.println("      y une los GND de las dos fuentes)");
  Serial.println("  No hace absolutamente nada ......... CABLE O SERVO ROTO");
}

//==================================================
// 2 - SONAR HC-SR04
//==================================================
float leerDistancia() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long t = pulseIn(PIN_ECHO, HIGH, 25000UL);
  if (t == 0) return -1.0;
  return t / 29.15 / 2.0;
}

void testSonar() {
  Serial.println("\n--- SONAR HC-SR04 (TRIG 17 / ECHO 25) ---");
  Serial.println("Pon la mano delante y acercala/alejala.");

  int validas = 0;
  float minim = 9999, maxim = 0;

  for (int i = 1; i <= 20; i++) {
    float d = leerDistancia();
    Serial.print("  medida ");
    Serial.print(i);
    Serial.print(": ");

    if (d < 0) {
      Serial.println("sin eco (nada delante, o ECHO mal conectado)");
    } else {
      Serial.print(d);
      Serial.println(" cm");
      validas++;
      if (d < minim) minim = d;
      if (d > maxim) maxim = d;
    }
    delay(300);
  }

  Serial.println("RESULTADO:");
  Serial.print("  medidas validas: ");
  Serial.print(validas);
  Serial.println(" de 20");

  if (validas == 0) {
    Serial.println("  -> SONAR KO: revisa TRIG(17), ECHO(25), VCC 5V y GND");
  } else if (maxim - minim < 1.0) {
    Serial.println("  -> SOSPECHOSO: el valor no cambia, mira si el sensor");
    Serial.println("     apunta al chasis del robot");
  } else {
    Serial.print("  -> SONAR OK  (min ");
    Serial.print(minim);
    Serial.print(" cm / max ");
    Serial.print(maxim);
    Serial.println(" cm)");
  }
}

//==================================================
// 3 - DHT11
//==================================================
void testDHT() {
  Serial.println("\n--- DHT11 (GPIO 27) ---");
  for (int i = 1; i <= 3; i++) {
    DHT.read(PIN_DHT);
    Serial.print("  lectura ");
    Serial.print(i);
    Serial.print(": Temp = ");
    Serial.print(DHT.temperature);
    Serial.print(" C   Hum = ");
    Serial.print(DHT.humidity);
    Serial.println(" %");
    delay(1500);
  }
  Serial.println("RESULTADO:");
  Serial.println("  Valores logicos (15-30 C, 30-70 %) ... DHT11 OK");
  Serial.println("  Sale 0 o valores absurdos ............ MAL CONECTADO");
}

//==================================================
// 4 - ESCANER I2C
//==================================================
void testI2C() {
  Serial.println("\n--- ESCANER I2C (SDA 21 / SCL 22) ---");
  int encontrados = 0;

  for (byte dir = 1; dir < 127; dir++) {
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) {
      Serial.print("  dispositivo en 0x");
      if (dir < 16) Serial.print("0");
      Serial.print(dir, HEX);
      if (dir == 0x3C || dir == 0x3D) Serial.print("   <- esta es la OLED");
      Serial.println();
      encontrados++;
    }
  }

  Serial.println("RESULTADO:");
  if (encontrados == 0) {
    Serial.println("  -> NADA EN EL BUS: revisa SDA(21), SCL(22), VCC y GND");
  } else {
    Serial.print("  -> ");
    Serial.print(encontrados);
    Serial.println(" dispositivo(s). Si ves 0x3C, la OLED responde.");
  }
}

//==================================================
// 5 - OLED
//==================================================
void testOLED() {
  Serial.println("\n--- OLED SSD1306 ---");

  if (!oledOK) {
    oledOK = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  }

  if (!oledOK) {
    Serial.println("  -> OLED KO: no responde en 0x3C. Haz antes la prueba 4.");
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEST OLED");
  display.setCursor(0, 20);
  display.println("Si lees esto,");
  display.setCursor(0, 34);
  display.println("la pantalla va.");
  display.drawRect(0, 50, 127, 13, SSD1306_WHITE);
  display.display();

  Serial.println("  -> Mira la pantalla: debe poner 'TEST OLED'");
  Serial.println("     y dibujar un rectangulo abajo.");
}

//==================================================
// 6 - SPIFFS
//==================================================
void testSPIFFS() {
  Serial.println("\n--- SPIFFS ---");

  if (!SPIFFS.begin(true)) {
    Serial.println("  -> SPIFFS KO: no se pudo montar");
    return;
  }

  File f = SPIFFS.open("/test.txt", FILE_WRITE);
  if (!f) {
    Serial.println("  -> KO: no se pudo escribir");
    return;
  }
  f.println("prueba de escritura");
  f.close();

  f = SPIFFS.open("/test.txt", FILE_READ);
  if (!f) {
    Serial.println("  -> KO: no se pudo leer");
    return;
  }
  Serial.print("  leido del fichero: ");
  Serial.println(f.readStringUntil('\n'));
  f.close();

  Serial.println("  -> SPIFFS OK");
  Serial.print("  espacio usado: ");
  Serial.print(SPIFFS.usedBytes());
  Serial.print(" de ");
  Serial.println(SPIFFS.totalBytes());
}

//==================================================
// 7 - UART: mandar ordenes reales a la Placa B
//==================================================
void testUART() {
  Serial.println("\n--- UART -> PLACA B ---");
  Serial.println("Voy a enviar las 5 ordenes. Mira los LEDs de la Placa B.");
  Serial.println("LEVANTA LAS RUEDAS DEL SUELO: los motores se van a mover.");
  delay(3000);

  char ordenes[] = {'P', 'F', 'P', 'S'};
  const char* nombres[] = {
    "P = pausa   (LED verde, motores parados)",
    "F = avanza  (LED verde, motores adelante)",
    "P = pausa   (LED verde, motores parados)",
    "S = stop    (LED rojo,  motores parados)"
  };

  for (int i = 0; i < 4; i++) {
    Serial.print("  enviando '");
    Serial.print(ordenes[i]);
    Serial.print("' -> ");
    Serial.println(nombres[i]);
    ComBoard.write(ordenes[i]);
    delay(2500);
  }

  Serial.println("  enviando 'R' -> gira derecha (LED amarillo, 5s)");
  ComBoard.write('R');

  unsigned long inicio = millis();
  bool recibida = false;
  while (millis() - inicio < 9000) {
    if (ComBoard.available() > 0 && ComBoard.read() == 'D') {
      recibida = true;
      break;
    }
    delay(20);
  }

  ComBoard.write('S');

  Serial.println("RESULTADO:");
  if (recibida) {
    Serial.print("  -> UART OK en los dos sentidos. Confirmacion 'D' en ");
    Serial.print(millis() - inicio);
    Serial.println(" ms");
  } else {
    Serial.println("  -> NO llego la 'D' de vuelta.");
    Serial.println("     Si los LEDs SI cambiaron: falla el cable D5 (A) <- D5 (B)");
    Serial.println("     Si los LEDs NO cambiaron: falla el cable D2 (A) -> D2 (B)");
    Serial.println("     Y comprueba siempre el GND comun entre placas.");
  }
}

//==================================================
// 8 - ECO UART (con TEST_PLACA_B cargada en la otra)
//==================================================
void testEco() {
  Serial.println("\n--- ECO UART (la Placa B debe tener TEST_PLACA_B) ---");

  while (ComBoard.available()) ComBoard.read();

  int ok = 0;
  char prueba[] = {'1', '2', '3', '4', '5'};

  for (int i = 0; i < 5; i++) {
    ComBoard.write(prueba[i]);
    Serial.print("  envio '");
    Serial.print(prueba[i]);
    Serial.print("' ... ");

    unsigned long inicio = millis();
    bool visto = false;
    while (millis() - inicio < 1000) {
      if (ComBoard.available() > 0) {
        char c = ComBoard.read();
        if (c == prueba[i]) { visto = true; break; }
      }
    }

    if (visto) { Serial.println("vuelve OK"); ok++; }
    else       { Serial.println("NO vuelve"); }
    delay(200);
  }

  Serial.println("RESULTADO:");
  Serial.print("  ");
  Serial.print(ok);
  Serial.println(" de 5 ecos correctos");
  if (ok == 5)      Serial.println("  -> CABLES D2, D5 y GND PERFECTOS");
  else if (ok == 0) Serial.println("  -> No hay comunicacion: revisa los 3 cables");
  else              Serial.println("  -> Comunicacion intermitente: cable flojo o mal crimpado");
}

//==================================================
void setup() {
  Serial.begin(9600);
  delay(500);

  Serial.println("\n\n### BANCO DE PRUEBAS - PLACA A ###");

  ComBoard.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo1.setPeriodHertz(50);
  servo1.attach(PIN_SERVO, 500, 2400);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  Wire.begin();
  oledOK = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.println(oledOK ? "OLED detectada al arrancar" : "OLED NO detectada al arrancar");

  menu();
}

void loop() {
  if (!Serial.available()) return;

  char op = Serial.read();
  if (op == '\n' || op == '\r' || op == ' ') return;

  switch (op) {
    case '1': testServo();  break;
    case '2': testSonar();  break;
    case '3': testDHT();    break;
    case '4': testI2C();    break;
    case '5': testOLED();   break;
    case '6': testSPIFFS(); break;
    case '7': testUART();   break;
    case '8': testEco();    break;
    case '9':
      testServo(); testSonar(); testDHT();
      testI2C();   testOLED();  testSPIFFS();
      Serial.println("\n### SECUENCIA COMPLETA TERMINADA ###");
      break;
    case 'm': break;
    default:
      Serial.print("Opcion desconocida: ");
      Serial.println(op);
  }
  menu();
}
