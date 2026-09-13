//==================================================
// TEST PLACA B - BANCO DE PRUEBAS DE COMPONENTES
//==================================================
// Placa: Keyestudio ESP32 Plus STEAMakers
//
// Sube este sketch, abre el Monitor Serie a 9600
// y escribe la letra/numero de la prueba que quieras.
//
// !!! LEVANTA EL ROBOT DEL SUELO ANTES DE PROBAR MOTORES !!!
// Pon un libro debajo para que las ruedas giren al aire.
//
// PINES (los mismos que el codigo final):
//   LED verde ........ GPIO 14
//   LED amarillo ..... GPIO 17
//   LED rojo ......... GPIO 25
//   Motor A: ENA 18 / IN1 19 / IN2 23
//   Motor B: ENB 12 / IN3 5  / IN4 13
//   UART2 TX (D5) .... GPIO 16  -> va a D5 de Placa A
//   UART2 RX (D2) .... GPIO 26  <- viene de D2 de Placa A
//
// AVISO IMPORTANTE SOBRE ENB = GPIO12:
//   GPIO12 es un pin de "strapping" del ESP32. Si el driver de
//   motores lo pone en ALTO mientras la placa arranca, la placa
//   NO ARRANCA (se queda en bucle de reinicio). Si te pasa:
//   desconecta el cable de ENB, enciende, y conectalo despues.
//==================================================

#define UART2_TX 16
#define UART2_RX 26
HardwareSerial ComBoard(2);

int verd    = 14;
int groc    = 17;
int vermell = 25;

int ENA = 18, IN1 = 19, IN2 = 23;
int ENB = 12, IN3 = 5,  IN4 = 13;

//==================================================
void menu() {
  Serial.println();
  Serial.println("=========== TEST PLACA B ===========");
  Serial.println(" 1 = LEDS       (verde, amarillo, rojo)");
  Serial.println(" 2 = MOTOR A    (adelante y atras)");
  Serial.println(" 3 = MOTOR B    (adelante y atras)");
  Serial.println(" 4 = LOS DOS    (adelante juntos)");
  Serial.println(" 5 = GIROS      (izquierda y derecha)");
  Serial.println(" 6 = UART       (escuchar a la Placa A)");
  Serial.println(" 7 = ECO        (devolver lo que llegue)");
  Serial.println(" 8 = CALIBRAR   (ajustar los grados del giro)");
  Serial.println(" 0 = PARAR TODO");
  Serial.println(" m = volver a mostrar este menu");
  Serial.println("====================================");
  Serial.print("> ");
}

//==================================================
void motoresParados() {
  digitalWrite(ENA, LOW); digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(ENB, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void ledsApagados() {
  digitalWrite(verd, LOW);
  digitalWrite(groc, LOW);
  digitalWrite(vermell, LOW);
}

//==================================================
// 1 - LEDS
//==================================================
void testLeds() {
  Serial.println("\n--- LEDS ---");
  Serial.println("Mira los 3 LEDs, se encienden de uno en uno.");

  int pines[]        = {verd, groc, vermell};
  const char* nombre[] = {"VERDE (GPIO 14)", "AMARILLO (GPIO 17)", "ROJO (GPIO 25)"};

  for (int i = 0; i < 3; i++) {
    ledsApagados();
    Serial.print("  encendiendo ");
    Serial.println(nombre[i]);
    digitalWrite(pines[i], HIGH);
    delay(1500);
  }

  ledsApagados();
  Serial.println("  parpadeo final de los tres...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(verd, HIGH); digitalWrite(groc, HIGH); digitalWrite(vermell, HIGH);
    delay(300);
    ledsApagados();
    delay(300);
  }

  Serial.println("RESULTADO:");
  Serial.println("  Los 3 se encienden .......... LEDS OK");
  Serial.println("  Uno no se enciende .......... LED al reves, sin resistencia,");
  Serial.println("                                cable suelto, o LED fundido");
}

//==================================================
// 2 y 3 - MOTORES POR SEPARADO
//==================================================
void probarMotor(int EN, int INa, int INb, const char* nombre) {
  Serial.print("\n--- ");
  Serial.print(nombre);
  Serial.println(" ---");
  Serial.println("RUEDAS AL AIRE. Empiezo en 3 segundos...");
  delay(3000);

  Serial.println("  ADELANTE (2 s)");
  digitalWrite(EN, HIGH);
  digitalWrite(INa, HIGH);
  digitalWrite(INb, LOW);
  delay(2000);

  motoresParados();
  Serial.println("  parado (1 s)");
  delay(1000);

  Serial.println("  ATRAS (2 s)");
  digitalWrite(EN, HIGH);
  digitalWrite(INa, LOW);
  digitalWrite(INb, HIGH);
  delay(2000);

  motoresParados();

  Serial.println("RESULTADO:");
  Serial.println("  Gira en los dos sentidos ...... MOTOR OK");
  Serial.println("  Gira solo en un sentido ....... falla IN1 o IN2 (cable)");
  Serial.println("  No gira pero zumba ............ pilas flojas o motor agarrotado");
  Serial.println("  No hace nada .................. revisa EN, el driver y la pila");
  Serial.println("  Gira al reves de lo esperado .. intercambia los 2 cables del motor");
}

//==================================================
// 4 - LOS DOS ADELANTE
//==================================================
void testAmbos() {
  Serial.println("\n--- LOS DOS MOTORES ADELANTE ---");
  Serial.println("RUEDAS AL AIRE. Empiezo en 3 segundos...");
  delay(3000);

  digitalWrite(ENA, HIGH); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(ENB, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  digitalWrite(verd, HIGH);

  Serial.println("  girando 3 s...");
  delay(3000);

  motoresParados();
  ledsApagados();

  Serial.println("RESULTADO:");
  Serial.println("  Las 2 ruedas giran HACIA ADELANTE ....... OK");
  Serial.println("  Una gira al reves que la otra ........... intercambia los");
  Serial.println("     dos cables de ESE motor en el driver");
  Serial.println("  Una gira mucho mas despacio ............. pilas justas o");
  Serial.println("     motor con mas rozamiento");
}

//==================================================
// 5 - GIROS
//==================================================
void testGiros() {
  Serial.println("\n--- GIROS ---");
  Serial.println("RUEDAS AL AIRE. Empiezo en 3 segundos...");
  delay(3000);

  digitalWrite(groc, HIGH);

  Serial.println("  IZQUIERDA (3 s): rueda A atras, rueda B adelante");
  digitalWrite(ENA, HIGH); digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(ENB, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  delay(3000);

  motoresParados();
  delay(1000);

  Serial.println("  DERECHA (3 s): rueda A adelante, rueda B atras");
  digitalWrite(ENA, HIGH); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(ENB, HIGH); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  delay(3000);

  motoresParados();
  ledsApagados();

  Serial.println("RESULTADO:");
  Serial.println("  Las ruedas giran en sentidos OPUESTOS ... OK");
  Serial.println("  Las dos giran igual ..................... tienes IN3/IN4");
  Serial.println("     cruzados respecto a IN1/IN2");
}

//==================================================
// 6 - ESCUCHAR A LA PLACA A
//==================================================
void testUART() {
  Serial.println("\n--- ESCUCHANDO A LA PLACA A (20 s) ---");
  Serial.println("Enciende la Placa A con su codigo final.");
  Serial.println("Aqui deben ir apareciendo las ordenes que manda.");

  while (ComBoard.available()) ComBoard.read();

  unsigned long inicio = millis();
  int recibidos = 0;

  while (millis() - inicio < 20000) {
    if (ComBoard.available() > 0) {
      char c = ComBoard.read();
      recibidos++;
      Serial.print("  [");
      Serial.print(millis() - inicio);
      Serial.print(" ms] llega: '");
      Serial.print(c);
      Serial.print("'");

      switch (c) {
        case 'F': Serial.println("  = avanza");        break;
        case 'P': Serial.println("  = pausa/escanea"); break;
        case 'S': Serial.println("  = stop");          break;
        case 'L': Serial.println("  = gira izquierda");break;
        case 'R': Serial.println("  = gira derecha");  break;
        default:  Serial.println("  = byte raro (velocidad distinta o ruido)");
      }
    }
    delay(10);
  }

  Serial.println("RESULTADO:");
  Serial.print("  bytes recibidos: ");
  Serial.println(recibidos);
  if (recibidos == 0) {
    Serial.println("  -> NO llega nada. Revisa:");
    Serial.println("     - cable D2 de Placa A a D2 de Placa B");
    Serial.println("     - GND comun entre las dos placas");
    Serial.println("     - que la Placa A este encendida y ejecutando su codigo");
  } else {
    Serial.println("  -> Comunicacion A -> B OK");
  }
}

//==================================================
// 7 - ECO (pareja de la opcion 8 de TEST_PLACA_A)
//==================================================
void testEco() {
  Serial.println("\n--- MODO ECO (30 s) ---");
  Serial.println("Devuelvo cada byte que llegue de la Placa A.");
  Serial.println("Lanza la opcion 8 en TEST_PLACA_A.");

  unsigned long inicio = millis();
  int n = 0;

  while (millis() - inicio < 30000) {
    if (ComBoard.available() > 0) {
      char c = ComBoard.read();
      ComBoard.write(c);
      n++;
      Serial.print("  eco de '");
      Serial.print(c);
      Serial.println("'");
    }
    delay(5);
  }

  Serial.print("RESULTADO: ");
  Serial.print(n);
  Serial.println(" bytes devueltos");
}

//==================================================
// 8 - CALIBRAR EL GIRO
//==================================================
// El giro se mide en TIEMPO, no en grados. Cuantos grados
// salen de X milisegundos depende de las pilas, del suelo
// y del agarre de las ruedas. Esta prueba te da el numero.
//==================================================
long leerNumero(const char* pregunta, long minimo, long maximo) {
  Serial.print(pregunta);
  Serial.setTimeout(60000);
  long v = Serial.parseInt();
  Serial.setTimeout(1000);
  while (Serial.available()) Serial.read();
  Serial.println(v);
  if (v < minimo || v > maximo) {
    Serial.print("  valor fuera de rango (");
    Serial.print(minimo); Serial.print(" a "); Serial.print(maximo);
    Serial.println("), cancelo");
    return -1;
  }
  return v;
}

void calibrarGiro() {
  Serial.println("\n--- CALIBRAR EL GIRO ---");
  Serial.println("IMPORTANTE: el robot va EN EL SUELO, no con las ruedas al aire.");
  Serial.println("Las pilas deben estar como las vas a usar el dia de la");
  Serial.println("presentacion: si las cambias, hay que recalibrar.");
  Serial.println();
  Serial.println("Marca con cinta por donde mira el robot ahora.");

  long ms = leerNumero("Milisegundos de giro a probar (empieza por 5000): ", 200, 15000);
  if (ms < 0) return;

  Serial.println("Girando a la DERECHA en 3 segundos...");
  delay(3000);

  digitalWrite(groc, HIGH);
  digitalWrite(ENA, HIGH); digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(ENB, HIGH); digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
  delay(ms);
  motoresParados();
  ledsApagados();

  Serial.println("Hecho. Mide con un transportador cuanto ha girado.");
  long graus = leerNumero("Grados que ha girado de verdad: ", 5, 720);
  if (graus < 0) return;

  long sugerido = (ms * 90L) / graus;

  Serial.println("RESULTADO:");
  Serial.print("  "); Serial.print(ms); Serial.print(" ms  ->  ");
  Serial.print(graus); Serial.println(" grados");
  Serial.print("  Para 90 grados necesitas unos ");
  Serial.print(sugerido);
  Serial.println(" ms");
  Serial.println();
  Serial.println("  Cambia TURN_DURATION_MS a ese valor en los DOS sketches:");
  Serial.println("    PLACA_B_MOTORS  -> es el que mueve los motores");
  Serial.println("    PLACA_A_SENSORS -> su timeout debe seguir siendo mayor");
  Serial.println();
  Serial.println("  Repite la prueba 2 o 3 veces: si sale un numero muy");
  Serial.println("  distinto cada vez, el problema no es el tiempo, son las");
  Serial.println("  ruedas patinando o las pilas justas.");
}

//==================================================
void setup() {
  Serial.begin(9600);
  delay(500);

  Serial.println("\n\n### BANCO DE PRUEBAS - PLACA B ###");
  Serial.println("Si lees esto, la placa arranca bien (GPIO12 no la bloquea).");

  ComBoard.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(verd, OUTPUT); pinMode(groc, OUTPUT); pinMode(vermell, OUTPUT);

  motoresParados();
  ledsApagados();

  menu();
}

void loop() {
  if (!Serial.available()) return;

  char op = Serial.read();
  if (op == '\n' || op == '\r' || op == ' ') return;

  switch (op) {
    case '1': testLeds(); break;
    case '2': probarMotor(ENA, IN1, IN2, "MOTOR A (ENA 18 / IN1 19 / IN2 23)"); break;
    case '3': probarMotor(ENB, IN3, IN4, "MOTOR B (ENB 12 / IN3 5 / IN4 13)");  break;
    case '4': testAmbos(); break;
    case '5': testGiros(); break;
    case '6': testUART();  break;
    case '7': testEco();   break;
    case '8': calibrarGiro(); break;
    case '0':
      motoresParados();
      ledsApagados();
      Serial.println("\nTODO PARADO.");
      break;
    case 'm': break;
    default:
      Serial.print("Opcion desconocida: ");
      Serial.println(op);
  }
  menu();
}
