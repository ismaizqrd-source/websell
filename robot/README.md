# Robot TR - codigo y banco de pruebas

Dos placas **Keyestudio ESP32 Plus STEAMakers** que se hablan por UART2.

| Carpeta | Que es |
|---|---|
| `PLACA_A_SENSORS/` | Codigo final: sonar, servo, DHT11, OLED, SPIFFS. Es quien decide. |
| `PLACA_B_MOTORS/` | Codigo final: motores y LEDs. Obedece a la Placa A. |
| `TEST_PLACA_A/` | Banco de pruebas de los componentes de la Placa A |
| `TEST_PLACA_B/` | Banco de pruebas de los componentes de la Placa B |

---

## Conexion entre placas

    Placa A  D2  (GPIO 26, TX) ---->  Placa B  D2  (GPIO 26, RX)
    Placa A  D5  (GPIO 16, RX) <----  Placa B  D5  (GPIO 16, TX)
    Placa A  GND --------------------  Placa B  GND     <- imprescindible

Ordenes A -> B: `F` avanza, `P` pausa, `S` stop definitivo, `L` gira izquierda, `R` gira derecha.
Respuesta B -> A: `D` giro terminado.

## Pines

**Placa A**

| Componente | Pin |
|---|---|
| Servo cabeza | GPIO 14 |
| HC-SR04 TRIG | GPIO 17 |
| HC-SR04 ECHO | GPIO 25 |
| DHT11 | GPIO 27 |
| OLED SSD1306 | I2C 0x3C (SDA 21 / SCL 22) |
| UART2 TX / RX | GPIO 26 / GPIO 16 |

**Placa B**

| Componente | Pin |
|---|---|
| LED verde / amarillo / rojo | GPIO 14 / 17 / 25 |
| Motor A: ENA / IN1 / IN2 | GPIO 18 / 19 / 23 |
| Motor B: ENB / IN3 / IN4 | GPIO 12 / 5 / 13 |
| UART2 TX / RX | GPIO 16 / GPIO 26 |

Umbral de deteccion: `LLINDAR_CM = 20.0` cm. Duracion del giro: 5000 ms.

---

## Como probar los componentes

Sube `TEST_PLACA_A` a una placa y `TEST_PLACA_B` a la otra, abre el
**Monitor Serie a 9600** y escribe el numero de la prueba. Cada prueba
dice en pantalla que tendrias que ver y que significa si no lo ves.

### Orden recomendado

Prueba cada cosa **suelta antes de montar el robot**. Si montas primero
y algo falla, no sabras si es el componente, el cable o el codigo.

1. **Placa B, opcion 1 (LEDS)** - lo mas facil, confirma que la placa
   ejecuta codigo y que sabes leer el Monitor Serie.
2. **Placa A, opcion 4 (I2C)** y **5 (OLED)** - sin partes moviles.
3. **Placa A, opcion 2 (SONAR)** - mueve la mano delante.
4. **Placa A, opcion 3 (DHT11)** y **6 (SPIFFS)**.
5. **Placa A, opcion 1 (SERVO)** - aqui aparecen los problemas de corriente.
6. **Placa B, opciones 2 y 3 (MOTORES uno a uno)** - con las ruedas al aire.
7. **Placa B, opcion 4 y 5** - los dos motores juntos y los giros.
8. **Los cables entre placas**: opcion 7 en la Placa B (modo eco) y
   opcion 8 en la Placa A. Cinco ecos correctos = los tres cables bien.
9. **Todo junto**: codigo final en las dos placas.

### Los tres fallos que mas pasan

1. **Alimentar el servo o los motores desde el pin 5V de la placa.**
   La placa se reinicia o el servo tiembla, y parece un fallo de codigo.
   Fuente aparte para los motores y el servo, y **GND comun** con la placa.
2. **Falta el GND entre las dos placas.** Los LEDs no reaccionan y salta
   el timeout de 8 s esperando la `D`. Con dos cables de datos y sin masa
   comun, el UART no funciona.
3. **Cable dupont roto por dentro.** Se ve entero y no conduce. Se detecta
   con el multimetro en modo continuidad, o sustituyendo el cable por otro.

---

## Cosas a revisar en el codigo final

Detectadas al leer los dos sketches, ninguna impide que funcione:

- **ENB esta en GPIO 12, que es un pin de strapping del ESP32.** Si el
  driver lo pone en ALTO durante el arranque, la Placa B no arranca
  (bucle de reinicio). Si pasa: arranca con el cable de ENB desconectado
  y conectalo despues, o mueve ENB a otro pin libre.
- **IN3 esta en GPIO 5**, tambien de strapping: suelta un pulso al
  arrancar y el motor puede dar un tiron en cada reset.
- **Mensaje de debug desfasado**: el `setup()` de la Placa A imprime
  "Umbral deteccion = 30cm" pero la constante real es 20 cm.
- **Comentarios de pines desfasados** en la Placa B: dice
  `MOTOR A (D13, D12, D11)` y `MOTOR B (D8, D10, D9)`, pero el codigo
  usa GPIO 18/19/23 y 12/5/13. Conviene comprobarlo contra la serigrafia.
- **El escaneo lateral usa 0 y 180 grados.** Si el sonar detecta el propio
  chasis en los extremos, prueba con 30 y 150 grados.
