/*
 * ============================================================
 *  Servo_AVR.h - Control de servomotor por PWM hardware (Timer1)
 *  ATmega328P - Salida OC1A = PB1 (D9 en Arduino Nano)
 *
 *  Fast PWM, TOP = ICR1, prescaler = 8 -> periodo de 20ms (50Hz)
 * ============================================================
 */
#ifndef SERVO_AVR_H
#define SERVO_AVR_H

#include <stdint.h>

/* Configura Timer1 en Fast PWM (modo 14, TOP=ICR1) a 50Hz y PB1 como salida. */
void Servo_Init(void);

/* Fija el ancho de pulso directamente en microsegundos (limitado a
 * [SERVO_PULSE_MIN_US, SERVO_PULSE_MAX_US] definidos en Servo_AVR.c). */
void Servo_SetPulseUs(uint16_t pulso_us);

/* Fija la posicion en grados (0-180), mapeado linealmente a ancho de pulso. */
void Servo_SetAngle(uint8_t angulo);

#endif /* SERVO_AVR_H */