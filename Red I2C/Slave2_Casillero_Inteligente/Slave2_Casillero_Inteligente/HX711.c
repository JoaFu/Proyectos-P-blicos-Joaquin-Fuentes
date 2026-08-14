/*
 * HX711.c
 * Driver de celda de carga (amplificador HX711) para ATmega328P
 * Canal A, ganancia 128. Lectura por bit-banging (DT/SCK).
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "HX711.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define HX711_DATA_DDR      DDRD
#define HX711_DATA_PORT     PORTD
#define HX711_DATA_PIN_REG  PIND
#define HX711_DATA_BIT      PD2

#define HX711_CLK_DDR       DDRD
#define HX711_CLK_PORT      PORTD
#define HX711_CLK_BIT        PD3

static int32_t hx711_offset = 0;
static float   hx711_escala = 1.0f;

// Configura pines DT (entrada) y SCK (salida).
void HX711_Init(void)
{
    HX711_DATA_DDR &= ~(1 << HX711_DATA_BIT);
    HX711_CLK_DDR  |= (1 << HX711_CLK_BIT);
    HX711_CLK_PORT &= ~(1 << HX711_CLK_BIT);
}

// DT en bajo indica que hay un dato listo para leer.
uint8_t HX711_IsReady(void)
{
    return (HX711_DATA_PIN_REG & (1 << HX711_DATA_BIT)) ? 0 : 1;
}

// Genera 24 pulsos de reloj (dato) + 1 pulso extra (canal A, ganancia 128).
int32_t HX711_ReadRaw(void)
{
    uint32_t dato = 0;
    uint8_t i;
    uint8_t sreg;

    while (!HX711_IsReady());

    sreg = SREG;
    cli();

    for (i = 0; i < 24; i++)
    {
        HX711_CLK_PORT |= (1 << HX711_CLK_BIT);
        _delay_us(1);
        HX711_CLK_PORT &= ~(1 << HX711_CLK_BIT);

        dato <<= 1;
        _delay_us(1);

        if (HX711_DATA_PIN_REG & (1 << HX711_DATA_BIT))
        {
            dato |= 1;
        }
    }

    // 25º pulso: fija canal A / ganancia 128 para la siguiente lectura
    HX711_CLK_PORT |= (1 << HX711_CLK_BIT);
    _delay_us(1);
    HX711_CLK_PORT &= ~(1 << HX711_CLK_BIT);
    _delay_us(1);

    SREG = sreg;

    // Extiende el signo de 24 a 32 bits (complemento a 2)
    if (dato & 0x00800000)
    {
        dato |= 0xFF000000;
    }

    return (int32_t)dato;
}

// Promedia "muestras" lecturas y las guarda como offset de tara.
void HX711_Tare(uint8_t muestras)
{
    int64_t suma = 0;
    uint8_t i;

    if (muestras == 0) muestras = 1;

    for (i = 0; i < muestras; i++)
    {
        suma += HX711_ReadRaw();
    }

    hx711_offset = (int32_t)(suma / muestras);
}

// Fija el factor de calibración (cuentas crudas por unidad de peso).
void HX711_SetScale(float factor)
{
    hx711_escala = (factor != 0.0f) ? factor : 1.0f;
}

// Retorna el peso ya con tara y escala aplicadas.
float HX711_GetWeight(void)
{
    int32_t crudo = HX711_ReadRaw();
    return (float)(crudo - hx711_offset) / hx711_escala;
}