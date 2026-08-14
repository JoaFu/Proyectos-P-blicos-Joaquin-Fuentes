/*
 * Stepper.c
 * Control de motor paso a paso 28BYJ-48 mediante CD4017 (secuenciador) + 74HC157 (direccion)
 * ATmega328P - Timer1 en modo CTC genera los pulsos de CLK (el CD4017 arma la secuencia,
 * el 74HC157 elige el sentido, y una etapa de 4 transistores maneja las bobinas)
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include "Stepper.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define STEPPER_CLK_DDR    DDRD
#define STEPPER_CLK_PORT   PORTD
#define STEPPER_CLK_BIT    PD4   // -> CLOCK del CD4017

#define STEPPER_DIR_DDR    DDRD
#define STEPPER_DIR_PORT   PORTD
#define STEPPER_DIR_BIT    PD5   // -> SELECT del 74HC157

// Prescaler 64 -> 1 tick = 4us. Valor por defecto ~2ms entre pasos (ajustar según motor).
#define STEPPER_OCR1A_DEFECTO   500

static volatile int32_t pasos_pendientes = 0;
static volatile int8_t  direccion_actual = 1;

// Configura pines CLK/DIR y Timer1 en CTC (sin pulsos al inicio)
void Stepper_Init(void)
{
    STEPPER_CLK_DDR |= (1 << STEPPER_CLK_BIT);
    STEPPER_DIR_DDR |= (1 << STEPPER_DIR_BIT);

    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, prescaler 64
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
    TIMSK1 |= (1 << OCIE1A);
}

// Retorna 1 mientras queden pasos pendientes por ejecutar.
uint8_t Stepper_EnMovimiento(void)
{
    return (pasos_pendientes > 0) ? 1 : 0;
}

// Detiene el envío de pulsos. Sin 5to transistor no hay forma de desenergizar,
// el CD4017 queda parqueado en la última fase (motor con retención).
void Stepper_Detener(void)
{
    TIMSK1 &= ~(1 << OCIE1A);
    pasos_pendientes = 0;
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
    STEPPER_CLK_PORT |= (1 << STEPPER_CLK_BIT);
    __asm__ __volatile__("nop\n\tnop\n\tnop\n\tnop");
    STEPPER_CLK_PORT &= ~(1 << STEPPER_CLK_BIT);

    if (pasos_pendientes > 0)
    {
        pasos_pendientes--;
    }

    if (pasos_pendientes == 0)
    {
        TIMSK1 &= ~(1 << OCIE1A);
    }
}