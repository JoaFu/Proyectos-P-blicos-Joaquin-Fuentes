/*
 * AHT10.c
 * Created: 2026-08-06
 * Author: Joaquín
 * Description: Implementación de la librería para el sensor AHT10 (I2C).
 */
/***************************************/

// Encabezado (Libraries)
/****************************************/
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "AHT10.h"
#include "I2C.h"  // Aquí toma tus macros I2C_WRITE e I2C_READ
#include <util/delay.h>

// Function prototypes
/****************************************/

// NON-Interrupt subroutines
/****************************************/

/* Inicializa el sensor AHT10 enviando el comando de calibración */
void AHT10_Init(void)
{
    _delay_ms(40); // Esperar a que el sensor encienda

    // Usa tu función i2c_start con la dirección 0x38 y el macro I2C_WRITE (0)
    if (i2c_start(AHT10_ADDRESS, I2C_WRITE) == I2C_OK) 
    {
        i2c_write(AHT10_CMD_INIT);
        i2c_write(0x08);
        i2c_write(0x00);
    }
    i2c_stop();

    _delay_ms(20); // Tiempo de calibración
}

/* 
 * Dispara una medición, lee los datos y calcula Temperatura y Humedad.
 * Retorna 1 si fue exitoso, 0 si hubo un error.
 */
uint8_t AHT10_Read(float *temperature, float *humidity)
{
    uint8_t buffer[6];
    uint32_t raw_hum, raw_temp;

    // 1. Enviar comando de medición
    if (i2c_start(AHT10_ADDRESS, I2C_WRITE) != I2C_OK) 
    {
        i2c_stop();
        return 0; // Falla de comunicación
    }
    i2c_write(AHT10_CMD_MEASURE);
    i2c_write(0x33);
    i2c_write(0x00);
    i2c_stop();

    // 2. Esperar a que la medición termine (Mínimo 75ms)
    _delay_ms(80);

    // 3. Leer 6 bytes del sensor
    if (i2c_start(AHT10_ADDRESS, I2C_READ) != I2C_OK) 
    {
        i2c_stop();
        return 0; // Falla al solicitar lectura
    }
    
    buffer[0] = i2c_read_ack();  // Status
    
    // Si el bit 7 está en 1, el sensor sigue ocupado (Busy)
    if ((buffer[0] & 0x80) != 0) 
    {
        // Limpiar el bus leyendo el resto o abortando
        i2c_stop();
        return 0; 
    }

    buffer[1] = i2c_read_ack();  // Humedad [19:12]
    buffer[2] = i2c_read_ack();  // Humedad [11:4]
    buffer[3] = i2c_read_ack();  // Humedad [3:0] y Temperatura [19:16]
    buffer[4] = i2c_read_ack();  // Temperatura [15:8]
    buffer[5] = i2c_read_nack(); // Temperatura [7:0]
    i2c_stop();

    // 4. Unir los bits para la humedad (20 bits)
    raw_hum = ((uint32_t)buffer[1] << 12) | ((uint32_t)buffer[2] << 4) | (buffer[3] >> 4);
    
    // 5. Unir los bits para la temperatura (20 bits)
    raw_temp = (((uint32_t)(buffer[3] & 0x0F)) << 16) | ((uint32_t)buffer[4] << 8) | buffer[5];

    // 6. Convertir los valores crudos a reales
    *humidity = ((float)raw_hum * 100.0) / 1048576.0;
    *temperature = ((float)raw_temp * 200.0) / 1048576.0 - 50.0;

    return 1; // Éxito
}

// Interrupt routines
/****************************************/
/* Vacio */