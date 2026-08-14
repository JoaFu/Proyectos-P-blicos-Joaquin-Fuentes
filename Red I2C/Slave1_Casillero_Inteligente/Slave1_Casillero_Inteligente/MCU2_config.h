/*
 * MCU2_config.h
 * Mapa de registros I2C, estados y parámetros del MCU2 (Acceso)
 */
#ifndef MCU2_CONFIG_H_
#define MCU2_CONFIG_H_
// Dirección I2C propia (7 bits)
#define MCU2_I2C_ADDR          0x10
// Índices del mapa de registros expuesto al Master
#define REG_DISTANCIA_CM       0
#define REG_ESTADO_PUERTA      1
#define REG_ESTADO_PESTILLO    2
#define REG_ERROR_SENSOR       3
#define REG_CMD                4
// --- Motor DC (nuevo) ---
#define REG_CMD_DC              5
#define REG_VEL_DC               6
#define NUM_REGISTROS_MCU2     7
// Estados de la puerta
#define PUERTA_CERRADA         0
#define PUERTA_ABIERTA         1
// Estados del pestillo
#define PESTILLO_BLOQUEADO     0
#define PESTILLO_DESBLOQUEADO  1
// Comandos válidos en REG_CMD
#define CMD_NINGUNO             0
#define CMD_DESBLOQUEAR         1
#define CMD_BLOQUEAR             2
// Comandos válidos en REG_CMD_DC (motor DC, driver TB6612FNG)
#define CMD_DC_NINGUNO           0
#define CMD_DC_ADELANTE          1
#define CMD_DC_ATRAS             2
#define CMD_DC_DETENER           3
#define CMD_DC_FRENO             4
// Umbrales de distancia con histéresis (cm)
#define UMBRAL_PUERTA_CERRADA_CM   11
#define UMBRAL_PUERTA_ABIERTA_CM   13
// Ángulos del servo (pestillo)
#define ANGULO_BLOQUEADO        0
#define ANGULO_DESBLOQUEADO     90
// Temporización (ms)
#define PERIODO_MEDICION_MS     100
#define TIEMPO_CIERRE_MS        1000
#endif /* MCU2_CONFIG_H_ */