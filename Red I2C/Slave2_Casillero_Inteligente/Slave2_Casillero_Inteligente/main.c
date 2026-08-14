/*
 * MCU3_Carrusel.c
 *
 * Author: Joaquín Fuentes, Luis Arriaza
 * Description: MCU esclavo encargado del control y posicionamiento
 *              del carrusel de muestras.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "I2C.h"
#include "Stepper.h"
#include "MCU3_config.h"


// Mapa de registros compartido con el Master mediante I2C
static volatile uint8_t registros_mcu3[NUM_REGISTROS_MCU3];

// Estado interno del carrusel
static uint8_t posicion_actual;
static uint8_t posicion_objetivo;
static uint8_t estado_carrusel;


/****************************************/
// Function prototypes
/****************************************/

static void MoverAPosicion(uint8_t objetivo);
static void HomingCarrusel(void);
static void ProcesarComandoMaster(void);
static void ActualizarRegistrosI2C(void);


/****************************************/
// Main Function
/****************************************/

int main(void)
{
    // Entrada utilizada para detectar la posición de referencia
    HOME_DDR &= ~(1 << HOME_BIT);

    Stepper_Init();

    i2c_slave_init_it(
        MCU3_I2C_ADDR,
        registros_mcu3,
        NUM_REGISTROS_MCU3
    );

    posicion_actual   = 0;
    posicion_objetivo = 0;
    estado_carrusel   = CARRUSEL_LISTO;

    registros_mcu3[REG_CMD] = CMD_NINGUNO;

    ActualizarRegistrosI2C();

    sei();

    while (1)
    {
        // Control del proceso de homing
        if (estado_carrusel == CARRUSEL_HOMING)
        {
            if (!(HOME_PIN_REG & (1 << HOME_BIT)))
            {
                Stepper_Detener();

                posicion_actual = 0;
                estado_carrusel = CARRUSEL_LISTO;
            }
            else if (!Stepper_EnMovimiento())
            {
                // No se encontró la referencia dentro del límite de pasos
                estado_carrusel = CARRUSEL_ERROR;
            }
        }

        // Verificar finalización de un movimiento normal
        else if (estado_carrusel == CARRUSEL_MOVIENDO)
        {
            if (!Stepper_EnMovimiento())
            {
                posicion_actual = posicion_objetivo;
                estado_carrusel = CARRUSEL_LISTO;
            }
        }

        // Procesar órdenes recibidas desde el Master
        ProcesarComandoMaster();

        // Actualizar información disponible por I2C
        ActualizarRegistrosI2C();
    }
}


/****************************************/
// Subrutinas
/****************************************/

// Calcula el desplazamiento e inicia el movimiento hacia una posición
static void MoverAPosicion(uint8_t objetivo)
{
    int32_t pasos;

    if (objetivo >= NUM_POSICIONES)
        return;

    pasos = ((int32_t)objetivo - (int32_t)posicion_actual)
            * PASOS_POR_POSICION;

    if (pasos == 0)
        return;

    posicion_objetivo = objetivo;

    Stepper_MoverPasos(pasos);

    estado_carrusel = CARRUSEL_MOVIENDO;
}


// Inicia el movimiento hasta encontrar la posición de referencia
static void HomingCarrusel(void)
{
    Stepper_MoverPasos(-PASOS_HOMING_MAX);

    estado_carrusel = CARRUSEL_HOMING;
}


// Interpreta los comandos escritos por el Master mediante I2C
static void ProcesarComandoMaster(void)
{
    uint8_t comando = registros_mcu3[REG_CMD];

    if (comando == CMD_NINGUNO)
        return;

    switch (comando)
    {
        case CMD_IR_POSICION:
            MoverAPosicion(registros_mcu3[REG_CMD_PARAM]);
            break;

        case CMD_HOME:
            HomingCarrusel();
            break;

        default:
            break;
    }

    registros_mcu3[REG_CMD] = CMD_NINGUNO;
}


// Publica el estado actual del carrusel en los registros I2C
static void ActualizarRegistrosI2C(void)
{
    registros_mcu3[REG_POSICION_ACTUAL] = posicion_actual;
    registros_mcu3[REG_ESTADO_CARRUSEL] = estado_carrusel;
}


/****************************************/
// Interrupt routines
/****************************************/

// I2C (TWI): ISR(TWI_vect) implementada en I2C.c
// Stepper: ISR(TIMER1_COMPA_vect) implementada en Stepper.c