#include <ArduinoBLE.h>
//Control de movimiento mediante controles DC + Sensor Ultrasónico
//Joaquín Fuentes
// 12/02/2026
/*------------------------------------------------------------------*/
// Motor A
const int IN1 = 4;
const int IN2 = 5;
const int ENA = 6;

// Motor B
const int IN3 = 7;
const int IN4 = 8;
const int ENB = 9;

// Ultrasónico
const int trigPin = 10;
const int echoPin = 11;
const int ledPin = 12;

long duracion;
int distancia;

int velocidad = 200;

// BLE
BLEService motorService("180A");
BLEStringCharacteristic comandoChar("2A57", BLEWrite, 20);

void setup() {

  // Motores
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  // BLE
  BLE.begin();
  BLE.setLocalName("RobotR4");
  BLE.setAdvertisedService(motorService);
  motorService.addCharacteristic(comandoChar);
  BLE.addService(motorService);
  comandoChar.writeValue("S");
  BLE.advertise();
}

void loop() {

  // -------- BLE --------
  BLEDevice central = BLE.central();

  if (central) {

    while (central.connected()) {

      if (comandoChar.written()) {

        String comando = comandoChar.value();
        comando.trim();

        if (comando == "F") adelante();
        if (comando == "B") atras();
        if (comando == "L") izquierda();
        if (comando == "R") derecha();
        if (comando == "S") detener();
      }

      medirDistancia();  // Medimos mientras está conectado
    }

    detener();
  }

  medirDistancia();  // También medimos aunque no esté conectado
}  

// -------- SENSOR --------

void medirDistancia() {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracion = pulseIn(echoPin, HIGH, 20000);  // timeout 20ms
  distancia = duracion * 0.034 / 2;

  Serial.println(distancia);

  if (distancia > 0 && distancia < 20) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(100);
}

// -------- MOTORES --------

void adelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void atras() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void izquierda() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void derecha() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void detener() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
