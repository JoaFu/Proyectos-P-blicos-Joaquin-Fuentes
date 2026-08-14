/*
 * MCU2_Acceso.c
 *
 * Created: 8-8-2026
 * Author: Joaquín Fuentes, Luis Arriaza
 * Description: MCU esclavo encargado del acceso al casillero (puerta + pestillo)
 *              y del motor DC (driver TB6612FNG, canal A) usado por el
 *              carrusel/mecanismo controlado desde el Master en modo manual.
 *
 *
 */
/****************************************/
// Encabezado (Libraries)
/****************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "I2C.h"
#include "HC_SR04.h"
#include "Servo_AVR.h"
#include "MCU2_config.h"
#include "motor_dc_atmega328p.h"

// Mapa de registros compartido con el Master (leído/escrito por ISR TWI)
static volatile uint8_t registros_mcu2[NUM_REGISTROS_MCU2];

// Bandera de 1ms generada por Timer0
static volatile uint8_t bandera_tick_ms = 0;

// Estado interno
static uint8_t estado_puerta;
static uint8_t estado_pestillo;
static uint8_t error_sensor;
static float   distancia_cm;
static uint8_t cierre_pendiente;
static uint16_t contador_medicion_ms;
static uint16_t contador_cierre_ms;

/****************************************/
// Function prototypes
/****************************************/
static void InicializarTimer(void);
static void MedirDistancia(void);
static void ActualizarEstadoPuerta(void);
static void AbrirPestillo(void);
static void CerrarPestillo(void);
static void ProcesarComandoMaster(void);
static void ProcesarComandoMotorDC(void);

/****************************************/
// Main Function
/****************************************/
int main(void)
{
    InicializarTimer();
    HC_SR04_Init();
    Servo_Init();
    MotorA_Init();
    i2c_slave_init_it(MCU2_I2C_ADDR, registros_mcu2, NUM_REGISTROS_MCU2);

    estado_puerta    = PUERTA_CERRADA;
    estado_pestillo  = PESTILLO_BLOQUEADO;
    error_sensor     = 0;
    cierre_pendiente = 0;
    contador_medicion_ms = 0;

    Servo_SetAngle(ANGULO_BLOQUEADO);

    registros_mcu2[REG_ESTADO_PUERTA]   = estado_puerta;
    registros_mcu2[REG_ESTADO_PESTILLO] = estado_pestillo;
    registros_mcu2[REG_ERROR_SENSOR]    = 0;
    registros_mcu2[REG_CMD]             = CMD_NINGUNO;
    registros_mcu2[REG_CMD_DC]          = CMD_DC_NINGUNO;
    registros_mcu2[REG_VEL_DC]          = 0;

    sei();

    while (1)
    {
        // Base de tiempo (1ms)
        if (bandera_tick_ms)
        {
            bandera_tick_ms = 0;

            if (contador_medicion_ms > 0) contador_medicion_ms--;
            if (cierre_pendiente && contador_cierre_ms > 0) contador_cierre_ms--;
        }

        // Medición periódica de la puerta
        if (contador_medicion_ms == 0)
        {
            contador_medicion_ms = PERIODO_MEDICION_MS;

            MedirDistancia();
            ActualizarEstadoPuerta();
        }

        // Cierre automático tras el tiempo de espera
        if (cierre_pendiente && contador_cierre_ms == 0)
        {
            cierre_pendiente = 0;

            if (estado_puerta == PUERTA_CERRADA)
            {
                CerrarPestillo();
            }
        }

        // Comando pendiente del Master
        ProcesarComandoMaster();
    }
}

/****************************************/
// NON-Interrupt subroutines
/****************************************/

// Dispara el HC-SR04 y actualiza distancia_cm / registro de error
static void MedirDistancia(void)
{
    float distancia = HC_SR04_ReadDistance();

    if (distancia < 0)
    {
        error_sensor = 1;
        registros_mcu2[REG_ERROR_SENSOR] = 1;
        return;
    }

    error_sensor = 0;
    registros_mcu2[REG_ERROR_SENSOR] = 0;

    if (distancia > 255.0f) distancia = 255.0f;

    distancia_cm = distancia;
    registros_mcu2[REG_DISTANCIA_CM] = (uint8_t)distancia;
}

// Determina apertura/cierre con histéresis y arma el cierre automático
static void ActualizarEstadoPuerta(void)
{
    if (error_sensor) return;

    if (estado_puerta == PUERTA_ABIERTA && distancia_cm <= UMBRAL_PUERTA_CERRADA_CM)
    {
        estado_puerta = PUERTA_CERRADA;

        if (estado_pestillo == PESTILLO_DESBLOQUEADO)
        {
            cierre_pendiente  = 1;
            contador_cierre_ms = TIEMPO_CIERRE_MS;
        }
    }
    else if (estado_puerta == PUERTA_CERRADA && distancia_cm >= UMBRAL_PUERTA_ABIERTA_CM)
    {
        estado_puerta   = PUERTA_ABIERTA;
        cierre_pendiente = 0;
    }

    registros_mcu2[REG_ESTADO_PUERTA] = estado_puerta;
}

// Mueve el servo a la posición desbloqueada
static void AbrirPestillo(void)
{
    Servo_SetAngle(ANGULO_DESBLOQUEADO);
    estado_pestillo = PESTILLO_DESBLOQUEADO;
    registros_mcu2[REG_ESTADO_PESTILLO] = estado_pestillo;
}

// Mueve el servo a la posición bloqueada
static void CerrarPestillo(void)
{
    Servo_SetAngle(ANGULO_BLOQUEADO);
    estado_pestillo = PESTILLO_BLOQUEADO;
    registros_mcu2[REG_ESTADO_PESTILLO] = estado_pestillo;
}

// Ejecuta el comando de motor DC escrito por el Master en REG_CMD_DC/REG_VEL_DC
static void ProcesarComandoMotorDC(void)
{
    if (registros_mcu2[REG_CMD_DC] == CMD_DC_NINGUNO) return;

    uint8_t velocidad = registros_mcu2[REG_VEL_DC];

    switch (registros_mcu2[REG_CMD_DC])
    {
        case CMD_DC_ADELANTE:
            MotorA_Drive((int16_t)velocidad);
            break;

        case CMD_DC_ATRAS:
            MotorA_Drive(-(int16_t)velocidad);
            break;

        case CMD_DC_DETENER:
            MotorA_Stop();
            break;

        case CMD_DC_FRENO:
            MotorA_Brake();
            break;

        default:
            break;
    }

    registros_mcu2[REG_CMD_DC] = CMD_DC_NINGUNO;
}

// Ejecuta el comando escrito por el Master en REG_CMD y en REG_CMD_DC
static void ProcesarComandoMaster(void)
{
    if (registros_mcu2[REG_CMD] == CMD_DESBLOQUEAR)
    {
        AbrirPestillo();
        cierre_pendiente = 0;

        registros_mcu2[REG_CMD] = CMD_NINGUNO;
    }
    else if (registros_mcu2[REG_CMD] == CMD_BLOQUEAR)
    {
        CerrarPestillo();
        cierre_pendiente = 0; // cancela cualquier cierre automatico pendiente

        registros_mcu2[REG_CMD] = CMD_NINGUNO;
    }

    ProcesarComandoMotorDC();
}

// Configura Timer2 en CTC para generar un tick de 1ms
// (Timer0 queda libre y exclusivo para el PWM del motor DC, ver NOTA 2)
static void InicializarTimer(void)
{
    TCCR2A = (1 << WGM21);           // CTC
    TCCR2B = (1 << CS22);            // Prescaler 64
    OCR2A  = 249;                    // 1ms @ 16MHz
    TIMSK2 = (1 << OCIE2A);
}

/****************************************/
// Interrupt routines
/****************************************/
// Nota: la comunicación I2C (TWI) se maneja en el ISR(TWI_vect) de I2C.c

ISR(TIMER2_COMPA_vect)
{
    bandera_tick_ms = 1;
}