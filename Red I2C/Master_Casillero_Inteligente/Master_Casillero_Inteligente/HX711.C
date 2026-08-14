/*
 * ============================================================
 *  HX711.c - Driver de bajo nivel (bit-banging) para HX711
 * ============================================================
 */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "HX711.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define HX711_DOUT_DDR    DDRD
#define HX711_DOUT_PORT   PORTD
#define HX711_DOUT_PIN    PIND
#define HX711_DOUT_BIT    PD4

#define HX711_SCK_DDR     DDRD
#define HX711_SCK_PORT    PORTD
#define HX711_SCK_BIT     PD5

static int32_t hx711_offset = 0;
static float   hx711_scale  = 1.0f;

void HX711_Init(void)
{
    HX711_DOUT_DDR &= ~(1 << HX711_DOUT_BIT); // DOUT como entrada
    HX711_SCK_DDR  |=  (1 << HX711_SCK_BIT);  // SCK como salida
    HX711_SCK_PORT &= ~(1 << HX711_SCK_BIT);  // SCK en bajo -> chip activo
}

uint8_t HX711_IsReady(void)
{
    // El HX711 avisa "dato listo" poniendo DOUT en bajo
    return ((HX711_DOUT_PIN & (1 << HX711_DOUT_BIT)) == 0);
}

int32_t HX711_ReadRaw(void)
{
    uint32_t valor = 0;

    // Espera activa a que haya dato (usar HX711_IsReady() antes para no
    // quedar bloqueado aqui en el loop principal).
    while (!HX711_IsReady());

    uint8_t sreg = SREG;
    cli(); // Los pulsos de reloj no deben interrumpirse por mucho tiempo
           // (>60us seguidos en alto apagarian el HX711).

    for (uint8_t i = 0; i < 24; i++)
    {
        HX711_SCK_PORT |= (1 << HX711_SCK_BIT);
        _delay_us(1);

        valor <<= 1;

        HX711_SCK_PORT &= ~(1 << HX711_SCK_BIT);
        _delay_us(1);

        if (HX711_DOUT_PIN & (1 << HX711_DOUT_BIT))
        {
            valor |= 1;
        }
    }

    // Pulso 25: fija canal A / ganancia 128 para la siguiente conversion
    HX711_SCK_PORT |= (1 << HX711_SCK_BIT);
    _delay_us(1);
    HX711_SCK_PORT &= ~(1 << HX711_SCK_BIT);
    _delay_us(1);

    SREG = sreg;

    // Extension de signo (dato de 24 bits en complemento a 2)
    if (valor & 0x00800000UL)
    {
        valor |= 0xFF000000UL;
    }

    return (int32_t)valor;
}

void HX711_Tare(uint8_t num_muestras)
{
    int64_t suma = 0;

    if (num_muestras == 0) num_muestras = 1;

    for (uint8_t i = 0; i < num_muestras; i++)
    {
        suma += HX711_ReadRaw();
    }

    hx711_offset = (int32_t)(suma / num_muestras);
}

int32_t HX711_GetOffset(void)
{
    return hx711_offset;
}

void HX711_SetScale(float escala)
{
    if (escala != 0.0f)
    {
        hx711_scale = escala;
    }
}

float HX711_GetWeightGrams(void)
{
    int32_t raw = HX711_ReadRaw();
    return ((float)(raw - hx711_offset)) / hx711_scale;
}

void HX711_PowerDown(void)
{
    HX711_SCK_PORT &= ~(1 << HX711_SCK_BIT);
    HX711_SCK_PORT |=  (1 << HX711_SCK_BIT);
    _delay_us(70); // >60us pone al HX711 en power-down
}

void HX711_PowerUp(void)
{
    HX711_SCK_PORT &= ~(1 << HX711_SCK_BIT);
}