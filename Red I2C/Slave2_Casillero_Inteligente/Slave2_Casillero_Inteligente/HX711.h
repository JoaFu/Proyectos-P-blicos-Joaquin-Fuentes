/*
 * HX711.h
 * Driver de celda de carga (amplificador HX711) para ATmega328P
 * Canal A, ganancia 128. Lectura por bit-banging (DT/SCK).
 */
#ifndef HX711_H_
#define HX711_H_

#include <stdint.h>

void HX711_Init(void);                 // Configura pines DT/SCK
uint8_t HX711_IsReady(void);           // 1 si hay un dato nuevo disponible
int32_t HX711_ReadRaw(void);           // Lectura cruda de 24 bits (bloqueante)
void HX711_Tare(uint8_t muestras);     // Promedia N lecturas y fija el offset
void HX711_SetScale(float factor);     // Fija el factor de calibración
float HX711_GetWeight(void);           // Retorna peso ya escalado y con tara aplicada

#endif /* HX711_H_ */