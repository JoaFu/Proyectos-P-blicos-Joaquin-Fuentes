/*
 * MCU3_config.h
 * Mapa de registros I2C, estados y parámetros del MCU3 (Carrusel)
 */
#ifndef MCU3_CONFIG_H_
#define MCU3_CONFIG_H_
#include <avr/io.h>
// Dirección I2C propia (7 bits)
#define MCU3_I2C_ADDR           0x11
// Índices del mapa de registros expuesto al Master
#define REG_PESO_L              0
#define REG_PESO_H              1
#define REG_CAMBIO_PESO         2
#define REG_ALERTA_MUESTRA      3
#define REG_POSICION_ACTUAL     4
#define REG_ESTADO_CARRUSEL     5
#define REG_CMD                 6
#define REG_CMD_PARAM           7
#define NUM_REGISTROS_MCU3      8
// Estados del carrusel
#define CARRUSEL_LISTO          0
#define CARRUSEL_MOVIENDO       1
#define CARRUSEL_HOMING         2
#define CARRUSEL_ERROR          3
// Comandos válidos en REG_CMD
#define CMD_NINGUNO              0
#define CMD_IR_POSICION           1
#define CMD_LIMPIAR_ALERTA       2
#define CMD_HOME                 3
// Parámetros del carrusel (28BYJ-48: ~4096 medios pasos por vuelta del eje de salida, reductor ~64:1; verificar contra datasheet real)
#define NUM_POSICIONES           6
#define PASOS_POR_POSICION       683
#define PASOS_HOMING_MAX         (PASOS_POR_POSICION * NUM_POSICIONES * 2)
// Sensor de home (activo en bajo)
#define HOME_DDR                 DDRD
#define HOME_PIN_REG              PIND
#define HOME_BIT                  PD7
// Detección de cambio de peso
#define UMBRAL_CAMBIO_PESO_G      5.0f
#define MUESTRAS_CONFIRMACION     3
#define FACTOR_CALIBRACION_HX711  2280.0f  // Ajustar tras calibración real
#define MUESTRAS_TARA             10
#endif /* MCU3_CONFIG_H_ */