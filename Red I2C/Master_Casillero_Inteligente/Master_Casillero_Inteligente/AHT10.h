/*
 * AHT10.h
 * Created: 2026-08-06
 * Author: Joaquín
 * Description: Archivo de cabecera para el sensor AHT10 (I2C).
 *              Adaptado para usar dirección de 7 bits.
 */

#ifndef AHT10_H_
#define AHT10_H_

#include <stdint.h>

/* Dirección I2C de 7 bits del sensor AHT10 */
#define AHT10_ADDRESS    0x38  

/* Comandos del AHT10 */
#define AHT10_CMD_INIT   0xE1
#define AHT10_CMD_MEASURE 0xAC

// Prototipos de la librería AHT10
void AHT10_Init(void);
uint8_t AHT10_Read(float *temperature, float *humidity);

#endif /* AHT10_H_ */