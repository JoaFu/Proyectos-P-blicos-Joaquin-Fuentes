/*
 * ============================================================
 *  Servo_AVR.c - Control de servomotor por PWM hardware (Timer1)
 * ============================================================
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "Servo_AVR.h"
#include <avr/io.h>

#define SERVO_DDR   DDRB
#define SERVO_BIT   PB1   // OC1A -> pin D9 en Arduino Nano

// Prescaler = 8, F_CPU = 16MHz -> 1 tick = 0.5us -> 2 ticks = 1us
#define SERVO_TICKS_PER_US   2UL
#define SERVO_PERIOD_US      20000UL                              // 20ms -> 50Hz
#define SERVO_TOP            (SERVO_PERIOD_US * SERVO_TICKS_PER_US - 1) // ICR1 = 39999

// Rango tipico de servos de hobby (SG90, MG90S, etc.). Ajustar si el
// servo real requiere otros limites para llegar a 0/180 grados.
#define SERVO_PULSE_MIN_US   500U   // ~0 grados
#define SERVO_PULSE_MAX_US   2500U  // ~180 grados

void Servo_Init(void)
{
    SERVO_DDR |= (1 << SERVO_BIT);

    // Fast PWM, TOP = ICR1 (modo 14 de WGM13:0), OC1A no invertido
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // prescaler = 8

    ICR1  = SERVO_TOP;
    OCR1A = SERVO_PULSE_MIN_US * SERVO_TICKS_PER_US; // posicion inicial segura
}

void Servo_SetPulseUs(uint16_t pulso_us)
{
    if (pulso_us < SERVO_PULSE_MIN_US) pulso_us = SERVO_PULSE_MIN_US;
    if (pulso_us > SERVO_PULSE_MAX_US) pulso_us = SERVO_PULSE_MAX_US;

    OCR1A = (uint16_t)((uint32_t)pulso_us * SERVO_TICKS_PER_US);
}

void Servo_SetAngle(uint8_t angulo)
{
    if (angulo > 180) angulo = 180;

    uint32_t rango  = SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US;
    uint16_t pulso  = SERVO_PULSE_MIN_US + (uint16_t)((rango * angulo) / 180UL);

    Servo_SetPulseUs(pulso);
}