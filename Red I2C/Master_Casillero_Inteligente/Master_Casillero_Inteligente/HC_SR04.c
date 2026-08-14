// HC_SR04.c
// Driver de bajo nivel para HC-SR04 en ATmega328P
// Timer2 (8 bits) + overflow_count para extender el rango de medicion.
// Timer1 queda libre para otros usos (ej. servo).

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "HC_SR04.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TRIG_DDR      DDRD
#define TRIG_PORT     PORTD
#define TRIG_BIT      PD2

#define ECHO_DDR      DDRD
#define ECHO_PORT     PORTD
#define ECHO_PIN_REG  PIND
#define ECHO_BIT      PD3

// Prescaler 256 -> 1 tick = 16 us (con F_CPU = 16 MHz)
#define TIMER2_PRESCALER_256  ((1 << CS22) | (1 << CS21))
#define TICKS_TO_US(ticks)    ((float)(ticks) * 16.0f)

// Timeout ~30 ms -> 30000us / 16us = 1875 ticks
#define TIMEOUT_TICKS 1875UL

// Contador de overflows de Timer2, extiende TCNT2 (8 bits) a ~24 bits
static volatile uint16_t overflow_count = 0;

ISR(TIMER2_OVF_vect)
{
	overflow_count++;
}

// Lectura atomica del contador extendido (overflow_count:TCNT2)
static uint32_t Timer2_GetTicks(void)
{
	uint8_t sreg = SREG;
	cli();
	uint32_t ticks = ((uint32_t)overflow_count << 8) | TCNT2;
	SREG = sreg;
	return ticks;
}

static void HC_SR04_TriggerPulse(void)
{
	TRIG_PORT &= ~(1 << TRIG_BIT);
	_delay_us(2);
	TRIG_PORT |= (1 << TRIG_BIT);
	_delay_us(10);
	TRIG_PORT &= ~(1 << TRIG_BIT);
}

void HC_SR04_Init(void)
{
	TRIG_DDR |= (1 << TRIG_BIT);
	TRIG_PORT &= ~(1 << TRIG_BIT);

	ECHO_DDR &= ~(1 << ECHO_BIT);
	ECHO_PORT &= ~(1 << ECHO_BIT);

	// Timer2 modo normal, corre libre todo el tiempo
	TCCR2A = 0x00;
	TCCR2B = TIMER2_PRESCALER_256;
	TCNT2 = 0;
	overflow_count = 0;

	TIMSK2 |= (1 << TOIE2); // interrupcion de overflow
}

float HC_SR04_ReadDistance(void)
{
	float distancia_cm = -1.0f;

	HC_SR04_TriggerPulse();

	uint32_t inicio = Timer2_GetTicks();

	// Esperar flanco de subida
	while (!(ECHO_PIN_REG & (1 << ECHO_BIT)))
	{
		if ((Timer2_GetTicks() - inicio) >= TIMEOUT_TICKS)
		{
			return distancia_cm;
		}
	}

	uint32_t inicio_pulso = Timer2_GetTicks();

	// Esperar flanco de bajada
	while (ECHO_PIN_REG & (1 << ECHO_BIT))
	{
		if ((Timer2_GetTicks() - inicio_pulso) >= TIMEOUT_TICKS)
		{
			return distancia_cm;
		}
	}

	uint32_t fin_pulso = Timer2_GetTicks();
	uint32_t pulso_ticks = fin_pulso - inicio_pulso;

	float tiempo_us = TICKS_TO_US(pulso_ticks);
	distancia_cm = tiempo_us / 58.0f;

	return distancia_cm;
}