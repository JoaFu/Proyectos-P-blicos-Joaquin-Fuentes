/*
 * Stepper.h
 * Control de motor paso a paso mediante driver STEP/DIR (A4988/DRV8825)
 * ATmega328P - Timer1 genera los pulsos de STEP (no bloqueante)
 */
#ifndef STEPPER_H_
#define STEPPER_H_

#include <stdint.h>

void Stepper_Init(void);                       // Configura pines y Timer1 (driver deshabilitado)
void Stepper_MoverPasos(int32_t pasos);         // Mueve N pasos (signo = dirección), no bloqueante
uint8_t Stepper_EnMovimiento(void);             // 1 si aún quedan pasos pendientes
void Stepper_Detener(void);                     // Detiene el movimiento y deshabilita el driver
void Stepper_SetVelocidad(uint16_t valor_ocr1a);// Ajusta la velocidad (periodo entre pasos)
int8_t Stepper_GetDireccion(void);              // Última dirección usada (1 o -1)

#endif /* STEPPER_H_ */