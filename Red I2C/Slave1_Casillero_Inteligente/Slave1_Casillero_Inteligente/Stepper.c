/*
 * Stepper.c
 * Control de motor paso a paso mediante driver STEP/DIR (A4988/DRV8825)
 * ATmega328P - Timer1 en modo CTC genera los pulsos de STEP
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "Stepper.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define STEPPER_STEP_DDR   DDRD
#define STEPPER_STEP_PORT  PORTD
#define STEPPER_STEP_BIT   PD4

#define STEPPER_DIR_DDR    DDRD
#define STEPPER_DIR_PORT   PORTD
#define STEPPER_DIR_BIT    PD5

#define STEPPER_EN_DDR     DDRD
#define STEPPER_EN_PORT    PORTD
#define STEPPER_EN_BIT     PD6   // Activo en bajo (A4988/DRV8825)

// Prescaler 8 -> 1 tick = 0.5us. Valor por defecto ~1kHz de pasos (ajustar según motor).
#define STEPPER_OCR1A_DEFECTO   2000

static volatile int32_t pasos_pendientes = 0;
static volatile int8_t  direccion_actual = 1;

// Configura pines STEP/DIR/ENABLE y Timer1 en CTC (driver deshabilitado al inicio).
void Stepper_Init(void)
{
    STEPPER_STEP_DDR |= (1 << STEPPER_STEP_BIT);
    STEPPER_DIR_DDR  |= (1 << STEPPER_DIR_BIT);
    STEPPER_EN_DDR   |= (1 << STEPPER_EN_BIT);

    STEPPER_EN_PORT |= (1 << STEPPER_EN_BIT); // Driver deshabilitado

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11); // CTC, prescaler 8
    OCR1A  = STEPPER_OCR1A_DEFECTO;
    TIMSK1 = 0;
}

// Inicia un movimiento de "pasos" (signo define dirección). No bloqueante.
void Stepper_MoverPasos(int32_t pasos)
{
    if (pasos == 0) return;

    if (pasos > 0)
    {
        direccion_actual = 1;
        STEPPER_DIR_PORT |= (1 << STEPPER_DIR_BIT);
    }
    else
    {
        direccion_actual = -1;
        STEPPER_DIR_PORT &= ~(1 << STEPPER_DIR_BIT);
        pasos = -pasos;
    }

    pasos_pendientes = pasos;

    STEPPER_EN_PORT &= ~(1 << STEPPER_EN_BIT); // Habilita driver
    TIMSK1 |= (1 << OCIE1A);
}

// Retorna 1 mientras queden pasos pendientes por ejecutar.
uint8_t Stepper_EnMovimiento(void)
{
    return (pasos_pendientes > 0) ? 1 : 0;
}

// Detiene el movimiento inmediatamente y deshabilita el driver.
void Stepper_Detener(void)
{
    TIMSK1 &= ~(1 << OCIE1A);
    pasos_pendientes = 0;
    STEPPER_EN_PORT |= (1 << STEPPER_EN_BIT);
}

// Ajusta el periodo entre pasos (valores más bajos = más rápido).
void Stepper_SetVelocidad(uint16_t valor_ocr1a)
{
    OCR1A = valor_ocr1a;
}

int8_t Stepper_GetDireccion(void)
{
    return direccion_actual;
}

ISR(TIMER1_COMPA_vect)
{
    STEPPER_STEP_PORT |= (1 << STEPPER_STEP_BIT);
    __asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop");
    STEPPER_STEP_PORT &= ~(1 << STEPPER_STEP_BIT);

    if (pasos_pendientes > 0)
    {
        pasos_pendientes--;
    }

    if (pasos_pendientes == 0)
    {
        TIMSK1 &= ~(1 << OCIE1A);
        STEPPER_EN_PORT |= (1 << STEPPER_EN_BIT);
    }
}