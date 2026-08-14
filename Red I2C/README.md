# Casillero Inteligente de Laboratorio

**Proyecto 1 — Electrónica Digital 2**

Sistema distribuido basado en tres microcontroladores **ATmega328P** y un **ESP32**, desarrollado para el monitoreo y control de un casillero inteligente mediante sensores, actuadores, comunicación I2C/UART y conexión con **Adafruit IO**.

---

## Tabla de Contenidos

* [Descripción](#descripción)
* [Arquitectura General](#arquitectura-general)
* [Sensores](#sensores)
* [Actuadores](#actuadores)
* [Funcionamiento General](#funcionamiento-general)
* [Comunicación](#comunicación)
* [Control Remoto](#control-remoto)
* [Integrantes](#integrantes)

---

## Descripción

El proyecto implementa un **Casillero Inteligente de Laboratorio** capaz de supervisar sus condiciones ambientales, controlar el acceso y posicionar un carrusel interno para el almacenamiento de muestras.

La arquitectura distribuye las tareas entre tres ATmega328P:

* **MCU Maestro:** centraliza información, controla la pantalla LCD, lee el AHT10 y mantiene comunicación con el ESP32.
* **MCU2:** controla el acceso mediante HC-SR04 y servomotor, además del motor DC mediante un TB6612FNG.
* **MCU3:** controla el posicionamiento del carrusel mediante un motor Stepper.
* **ESP32:** funciona como interfaz entre el sistema embebido y Adafruit IO.

---

## Arquitectura General

| Componente       | Cantidad | Función                                    |
| ---------------- | -------: | ------------------------------------------ |
| ATmega328P       |        3 | MCU Maestro + 2 MCU periféricos            |
| ESP32            |        1 | Comunicación WiFi con Adafruit IO          |
| LCD 16x2         |        1 | Visualización local de variables y estados |
| AHT10            |        1 | Humedad y temperatura                      |
| HC-SR04          |        1 | Detección del estado de la puerta          |
| Servomotor SG90  |        1 | Pestillo de acceso                         |
| Motor DC         |        1 | Ventilación                                |
| TB6612FNG        |        1 | Driver del motor DC                        |
| Stepper 28BYJ-48 |        1 | Posicionamiento del carrusel               |
| CD4017 + 74HC157 |  1 etapa | Secuenciación y dirección del Stepper      |

---

## Sensores

### AHT10

Sensor digital conectado directamente al **MCU Maestro mediante I2C**. Proporciona las mediciones de humedad y temperatura utilizadas por la pantalla LCD, el sistema de telemetría y el control automático de ventilación.

### HC-SR04

Sensor ultrasónico conectado al **MCU2**. La distancia medida permite determinar mediante histéresis si la puerta se encuentra abierta o cerrada.

El estado de la puerta se relaciona directamente con el control del pestillo: cuando se detecta la puerta cerrada, el sistema puede ejecutar su bloqueo automático después del tiempo configurado.

---

## Actuadores

| Actuador                 | Control | Función                           |
| ------------------------ | ------- | --------------------------------- |
| **Servomotor SG90**      | MCU2    | Bloqueo y desbloqueo del pestillo |
| **Motor DC + TB6612FNG** | MCU2    | Ventilación con control PWM       |
| **Stepper 28BYJ-48**     | MCU3    | Posicionamiento del carrusel      |

### Motor DC

El motor DC funciona como sistema de ventilación. El MCU2 controla su velocidad mediante PWM y el driver **TB6612FNG**.

En modo automático, el MCU Maestro utiliza la humedad obtenida por el AHT10 para ejecutar una secuencia no bloqueante de ventilación con diferentes velocidades.

### Servomotor

El servomotor controla mecánicamente el pestillo del casillero. Puede recibir órdenes de bloqueo y desbloqueo desde el MCU Maestro y también participa en la lógica automática asociada al estado de la puerta.

### Motor Stepper

El motor **28BYJ-48** posiciona el carrusel interno. Su etapa de control utiliza un **CD4017**, un **74HC157** y transistores BJT para manejar las bobinas.

El MCU3 genera mediante Timer1 los pulsos necesarios para avanzar la secuencia y controla la dirección del movimiento. El firmware permite realizar movimientos hacia posiciones definidas y ejecutar una rutina de *homing* para establecer la posición de referencia.

---

## Funcionamiento General

### Monitoreo ambiental

1. El MCU Maestro obtiene periódicamente la humedad y temperatura mediante el AHT10.
2. Las mediciones se muestran en la pantalla LCD.
3. La humedad es evaluada por la lógica de control automático.
4. Al superar el umbral configurado, el Maestro ordena al MCU2 activar el motor DC.
5. El control del motor se realiza mediante una máquina de estados no bloqueante.
6. Las mediciones son enviadas al ESP32 mediante UART para su publicación en Adafruit IO.

### Control de acceso

1. El MCU2 mide periódicamente la distancia mediante el HC-SR04.
2. Se utiliza histéresis para determinar si la puerta está abierta o cerrada.
3. El servomotor controla el bloqueo y desbloqueo del pestillo.
4. Cuando la puerta se detecta cerrada, puede ejecutarse un bloqueo automático después del tiempo establecido.
5. Los estados de puerta y pestillo son enviados al MCU Maestro mediante I2C.

### Control del carrusel

1. El MCU Maestro envía al MCU3 la posición solicitada mediante I2C.
2. MCU3 calcula el número de pasos requerido respecto a la posición actual.
3. El movimiento del Stepper se ejecuta de forma no bloqueante.
4. La rutina de *homing* permite establecer la posición inicial del carrusel.
5. MCU3 publica mediante I2C la posición y el estado actual del mecanismo.

---

## Comunicación

El sistema utiliza tres niveles de comunicación:

| Protocolo              | Uso                                                  |
| ---------------------- | ---------------------------------------------------- |
| **I2C/TWI**            | Comunicación entre MCU Maestro, MCU2, MCU3 y AHT10   |
| **UART**               | Comunicación bidireccional entre MCU Maestro y ESP32 |
| **WiFi / Adafruit IO** | Monitoreo y control remoto                           |

### Red I2C

El ATmega328P Maestro administra el bus I2C a **100 kHz**.

Los MCU periféricos utilizan mapas de registros para intercambiar estados, parámetros y comandos con el Maestro.

| Dispositivo | Dirección |
| ----------- | --------: |
| MCU2        |    `0x10` |
| MCU3        |    `0x11` |
| AHT10       |    `0x38` |

### UART

El MCU Maestro mantiene comunicación bidireccional con el ESP32. Por este enlace se transmiten las mediciones y estados del casillero, mientras que los comandos recibidos desde Adafruit IO regresan al Maestro para ser procesados y enviados al MCU correspondiente.

---

## Control Remoto

Adafruit IO permite visualizar las variables principales del sistema y enviar comandos de control.

El sistema dispone de un **modo manual** que habilita el accionamiento remoto de los actuadores. Los comandos recibidos por el ESP32 son enviados mediante UART al MCU Maestro, que determina el MCU periférico correspondiente y retransmite la orden mediante I2C.

Entre las funciones implementadas se encuentran:

* Bloqueo y desbloqueo del pestillo.
* Control de velocidad del motor DC.
* Posicionamiento del carrusel.
* Ejecución del *homing* del carrusel.
* Activación y desactivación del modo manual.
* Monitoreo de humedad y temperatura.
* Monitoreo del estado de puerta y pestillo.

---

## Estado del Proyecto

**Proyecto finalizado.**

Se completó la integración de:

* [x] Red I2C entre los ATmega328P
* [x] Lectura I2C del AHT10
* [x] Detección de puerta mediante HC-SR04
* [x] Control del pestillo mediante servomotor
* [x] Control PWM del motor DC mediante TB6612FNG
* [x] Control del carrusel mediante Stepper
* [x] Pantalla LCD desde el MCU Maestro
* [x] Comunicación UART Maestro–ESP32
* [x] Conexión WiFi y Adafruit IO
* [x] Modo manual para control remoto
* [x] Control automático de ventilación
* [x] Integración y funcionamiento simultáneo del sistema

---

## Integrantes

* **Joaquín Enrique Fuentes Rodríguez**
* **Luis Fernando Arriaza Castillo**

---

<p align="center">
  <i>Proyecto desarrollado para el curso Electrónica Digital 2.</i>
</p>
