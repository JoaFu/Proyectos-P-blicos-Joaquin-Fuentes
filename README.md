# Casillero Inteligente de Laboratorio

**Proyecto 1 — Electrónica Digital 2**

> ⚠️ **Documento en construcción.** Este README describe el diseño actual del proyecto y puede sufrir cambios conforme avance el desarrollo.

---

## Tabla de Contenidos

- [Descripción](#descripción)
- [Objetivos](#objetivos)
- [Arquitectura General](#arquitectura-general)
- [Sensores](#sensores)
- [Actuadores](#actuadores)
- [Funcionamiento General](#funcionamiento-general)
- [Comunicación](#comunicación)
- [Estado del Proyecto](#estado-del-proyecto)
- [Integrantes](#integrantes)

---

## Descripción

Este proyecto consiste en el diseño e implementación de un **Casillero Inteligente de Laboratorio**, un sistema distribuido basado en una red de sensores y múltiples microcontroladores que permite controlar el acceso a un compartimiento de almacenamiento y supervisar sus condiciones ambientales.

El sistema busca simular el funcionamiento de un casillero utilizado para almacenar materiales sensibles, integrando monitoreo de humedad, control de acceso automatizado y ventilación inteligente.

---

## Objetivos

- Implementar una red de sensores utilizando tres microcontroladores ATmega328P.
- Integrar un ESP32 para enviar información en tiempo real hacia Adafruit IO.
- Aplicar comunicación mediante I2C y UART.
- Implementar una máquina de estados para controlar el funcionamiento del sistema.
- Diseñar un proceso automatizado con múltiples sensores y actuadores.

---

## Arquitectura General

El sistema estará conformado por:

| Componente | Cantidad | Rol |
|---|---|---|
| ATmega328P | 3 | Maestro + 2 periféricos |
| ESP32 | 1 | Puente WiFi hacia Adafruit IO |
| Pantalla LCD 16x2 | 1 | Visualización de estado (MCU Maestro) |
| Comunicación I2C | — | Entre los 3 microcontroladores |
| Comunicación UART | — | Maestro → ESP32 |
| Adafruit IO | — | Monitoreo remoto en tiempo real |

---

## Sensores

El proyecto utilizará tres sensores distintos, con al menos uno mediante I2C:

| Sensor | Interfaz | Función |
|---|---|---|
| **BME280** | I2C | Medición de temperatura y humedad interna |
| **HC-SR04** | Digital | Detección de presencia para control de acceso |
| **Celda de carga + HX711** | Digital | Monitoreo continuo del peso almacenado |

---

## Actuadores

Se utilizarán los tres tipos de motores requeridos por el proyecto:

| Actuador | Función |
|---|---|
| **Servo motor** | Control del pestillo del casillero |
| **Motor Stepper** | Regulación gradual de una compuerta de ventilación |
| **Motor DC** | Sistema extractor de aire |

---

## Funcionamiento General

El sistema operará mediante dos procesos principales.

### Control de Acceso

1. Al detectar presencia sostenida durante un tiempo determinado, el sistema desbloquea el pestillo mediante el servomotor.
2. Durante la interacción, se supervisa continuamente el estado de la puerta y el peso almacenado, para detectar cambios en el contenido.
3. Una vez finalizada la interacción y confirmada la posición de la puerta, el pestillo vuelve a bloquearse automáticamente.

### Control Ambiental

1. El sistema monitorea continuamente la humedad y temperatura internas mediante el sensor BME280.
2. Si la humedad supera determinados umbrales, la compuerta de ventilación se abre gradualmente mediante el motor paso a paso.
3. Si la ventilación pasiva no es suficiente, se activa el extractor mediante el motor DC.

---

## Comunicación

| Protocolo | Uso |
|---|---|
| **I2C** | Intercambio de información entre los microcontroladores ATmega328P |
| **UART** | Envío de datos del MCU Maestro hacia el ESP32 |
| **WiFi / Adafruit IO** | Publicación en tiempo real de las variables del sistema |

---

## Estado del Proyecto

 Actualmente en etapa de **diseño y definición de la arquitectura general**.

Próximas fases:

- [ ] Diseño de la máquina de estados
- [ ] Diseño mecánico del casillero
- [ ] Definición del hardware
- [ ] Desarrollo del software embebido
- [ ] Integración con Adafruit IO

---

## Integrantes

- Joaquín Fuentes
- Luis Fernando Arriaza

---

<p align="center"><i>Proyecto desarrollado para el curso Electrónica Digital 2</i></p>
