/*
 * Master_config.h
 * Direcciones I2C y mapas de registros de los esclavos (deben coincidir
 * exactamente con MCU2_config.h y MCU3_config.h de cada esclavo).
 */
#ifndef MASTER_CONFIG_H_
#define MASTER_CONFIG_H_

// Direcciones I2C de los esclavos
#define MCU2_I2C_ADDR    0x10
#define MCU3_I2C_ADDR    0x11

/* ---------------- Mapa de registros MCU2 (Acceso) ---------------- */
#define MCU2_REG_DISTANCIA_CM     0
#define MCU2_REG_ESTADO_PUERTA    1
#define MCU2_REG_ESTADO_PESTILLO  2
#define MCU2_REG_ERROR_SENSOR     3
#define MCU2_REG_CMD              4
#define MCU2_NUM_REGISTROS        5

#define MCU2_PUERTA_CERRADA         0
#define MCU2_PUERTA_ABIERTA         1
#define MCU2_PESTILLO_BLOQUEADO     0
#define MCU2_PESTILLO_DESBLOQUEADO  1
#define MCU2_CMD_NINGUNO            0
#define MCU2_CMD_DESBLOQUEAR        1

/* ---------------- Mapa de registros MCU3 (Carrusel) ---------------- */
#define MCU3_REG_PESO_L            0
#define MCU3_REG_PESO_H            1
#define MCU3_REG_CAMBIO_PESO       2
#define MCU3_REG_ALERTA_MUESTRA    3
#define MCU3_REG_POSICION_ACTUAL   4
#define MCU3_REG_ESTADO_CARRUSEL   5
#define MCU3_REG_CMD               6
#define MCU3_REG_CMD_PARAM         7
#define MCU3_NUM_REGISTROS         8

#define MCU3_CARRUSEL_LISTO       0
#define MCU3_CARRUSEL_MOVIENDO    1
#define MCU3_CARRUSEL_HOMING      2
#define MCU3_CARRUSEL_ERROR       3

#define MCU3_CMD_NINGUNO           0
#define MCU3_CMD_IR_POSICION       1
#define MCU3_CMD_LIMPIAR_ALERTA    2
#define MCU3_CMD_HOME              3

#define MCU3_NUM_POSICIONES        6

// Periodo de lectura periódica de ambos esclavos (ms)
#define PERIODO_LECTURA_MS         500


#define MCU2_CMD_BLOQUEAR        2

// --- Motor DC de MCU2 (driver TB6612FNG) ---
#define MCU2_REG_CMD_DC          5
#define MCU2_REG_VEL_DC          6

#define MCU2_CMD_DC_NINGUNO      0
#define MCU2_CMD_DC_ADELANTE     1
#define MCU2_CMD_DC_ATRAS        2
#define MCU2_CMD_DC_DETENER      3
#define MCU2_CMD_DC_FRENO        4

/* ---------------- Control Automático del Motor DC ---------------- */
// Umbral de humedad para activar el modo automático (0-100%)
#define UMBRAL_HUMEDAD_AUTO_DC      70

// Tiempos de cada etapa del ciclo automático (en milisegundos)
#define AUTO_DC_ETAPA1_MS           5000    // 5 segundos al 100%
#define AUTO_DC_ETAPA2_MS           3000    // 3 segundos al 50%
#define AUTO_DC_ETAPA3_MS           2000    // 2 segundos al 20%

#endif /* MASTER_CONFIG_H_ */