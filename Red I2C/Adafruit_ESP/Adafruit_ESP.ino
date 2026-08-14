// Adafruit IO Publish & Subscribe Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit Industries!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Definición de UART2 *******************************/
#define RX2_PIN 16  // Cambia estos pines si estás usando otros en tu hardware
#define TX2_PIN 17

/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
int count = 0;

// Track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds.
#define IO_LOOP_DELAY 15000
unsigned long lastUpdate = 0;
uint8_t Humedad = 0;
uint16_t Masa = 0;
uint8_t Cerradura = 0;
uint8_t Puerta = 0;
uint8_t DC = 0;
uint8_t Servo = 0;
uint8_t Stepper = 0;
uint8_t Modo_manual = 0; // Almacena el estado actual del modo manual

// set up the feeds
AdafruitIO_Feed *canalHumedad = io.feed("Humedad");
AdafruitIO_Feed *canalMasa = io.feed("Masa");
AdafruitIO_Feed *canalPuerta = io.feed("Puerta");
AdafruitIO_Feed *canalModo_manual = io.feed("Modo_manual");
AdafruitIO_Feed *canalDC = io.feed("DC");
AdafruitIO_Feed *canalServo = io.feed("Servo");
AdafruitIO_Feed *canalStepper = io.feed("Stepper");
AdafruitIO_Feed *canalCerradura = io.feed("Cerradura");

void setup() {

  // Start the USB serial connection for debugging
  Serial.begin(115200);

  // Start UART2 connection (Baudrate, Config, RX pin, TX pin)
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  // wait for serial monitor to open
  while (!Serial)
    ;

  Serial.print("Connecting to Adafruit IO");

  // connect to ://adafruit.com
  io.connect();

  // set up a message handler for the a feed.
  canalModo_manual->onMessage(handleModo_manual);
  canalDC->onMessage(handleDC);
  canalStepper->onMessage(handleStepper);
  canalServo->onMessage(handleServo);

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  canalModo_manual->get();
  canalStepper->get();
  canalServo->get();
  canalDC->get();
}

void loop() {

  // io.run(); is required for all sketches.
  io.run();

  // Subrutina para revisar y procesar datos entrantes desde UART2
  leerUart2();

  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    // save count to the feeds on Adafruit IO
    Serial.print("sending -> ");
    Serial.println(Humedad);
    canalHumedad->save(Humedad);

    Serial.print("sending -> ");
    Serial.println(Masa);
    canalMasa->save(Masa);

    Serial.print("sending -> ");
    Serial.println(Puerta);
    canalPuerta->save(Puerta);

    Serial.print("sending -> ");
    Serial.println(Cerradura);
    canalCerradura->save(Cerradura);

    // after publishing, store the current time
    lastUpdate = millis();
  }
}

// Subrutina para recibir y actualizar datos por UART2
void leerUart2() {
  if (Serial2.available() > 0) {
    // Lee la cadena hasta encontrar un salto de línea (\n) y remueve espacios vacíos
    String comando = Serial2.readStringUntil('\n');
    comando.trim(); 

    if (comando.length() == 0) return;

    Serial.print("UART2 Recibido: ");
    Serial.println(comando);

    // Procesar datos de Cerradura
    if (comando == "Lock_Off") {
      Cerradura = 1;
      Serial.println("Estado Cerradura actualizado: Abierto");
    } 
    else if (comando == "Lock_On") {
      Cerradura = 0;
      Serial.println("Estado Cerradura actualizado: Cerrado");
    }
    // Procesar datos de Puerta
    else if (comando == "Gate_Op") {
      Puerta = 1;
      Serial.println("Estado Puerta actualizado: 1 (Abierta)");
    } 
    else if (comando == "Gate_Cl") {
      Puerta = 0;
      Serial.println("Estado Puerta actualizado: 0 (Cerrada)");
    }
    // Procesar datos de Humedad (Ejemplo: H45)
    else if (comando.startsWith("H")) {
      String valorStr = comando.substring(1); // Extrae el texto después de la 'H'
      int valorHum = valorStr.toInt();
      if (valorHum >= 0 && valorHum <= 100) {
        Humedad = valorHum;
        Serial.print("Humedad actualizada: ");
        Serial.println(Humedad);
      }
    }
    // Procesar datos de Masa (Ejemplo: M1250)
    else if (comando.startsWith("M")) {
      String valorStr = comando.substring(1); // Extrae el texto después de la 'M'
      int valorMasa = valorStr.toInt();
      if (valorMasa >= 0 && valorMasa <= 5000) {
        Masa = valorMasa;
        Serial.print("Masa actualizada: ");
        Serial.println(Masa);
      }
    }
  }
}

// Control del Modo Manual
void handleModo_manual(AdafruitIO_Data *data) {
  int valor = data->toInt();
  Modo_manual = valor; // Actualiza el estado global

  Serial.print("Modo Manual recibido: ");
  Serial.println(valor);

  if (valor == 1) {
    Serial2.println("Manual_On");
  } else {
    Serial2.println("Manual_Off");
  }
}

// Control del canal DC
void handleDC(AdafruitIO_Data *data) {
  String valorStr = data->value();
  
  Serial.print("DC recibido: ");
  Serial.println(valorStr);

  // Solo transmite si el modo manual está activo
  if (Modo_manual == 1) {
    // UNIFICADO: Concatenación en un solo String para un único envío UART
    String cadenaDC = "DC" + valorStr + "";
    Serial2.println(cadenaDC);
  }
}

// Control del motor Stepper
void handleStepper(AdafruitIO_Data *data) {
  int valor = data->toInt();

  Serial.print("Stepper recibido: ");
  Serial.println(valor);

  // Solo transmite si el modo manual está activo
  if (Modo_manual == 1) {
    switch(valor) {
      case 0:
        Serial2.println("Stepper_Der(0)");
        break;
      case 1:
        Serial2.println("Stepper_Der(1)");
        break;
      case 2:
        Serial2.println("Stepper_Der(2)");
        break;
      case 3:
        Serial2.println("Stepper_Der(3)");
        break;
      case 4:
        Serial2.println("Stepper_Der(4)");
        break;
      case 5:
        Serial2.println("Stepper_Der(5)");
        break;
    }
  }
}

// Control del Servo (Cerradura/Pestillo)
void handleServo(AdafruitIO_Data *data) {
  int valor = data->toInt();

  Serial.print("Servo recibido: ");
  Serial.println(valor);

  // CORRECCIÓN: Los comandos de cerradura deben ejecutarse siempre,
  // sin importar si el modo manual está activo o no.
  // Se eliminó la condición "if (Modo_manual == 1)" que lo impedía.
  if (valor == 0) {
    Serial2.println("Servo_close");
  } else if (valor == 1) {
    Serial2.println("Servo_open");
  }
}