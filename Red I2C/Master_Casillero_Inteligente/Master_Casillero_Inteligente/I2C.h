/*
 * ============================================================
 *  I2C.h - Librería I2C (TWI) para ATmega328P
 *
 *  Basada en el módulo TWI (Two Wire Interface) del ATmega328P
 *  Registros utilizados: TWBR, TWCR, TWSR, TWDR, TWAR, TWAMR
 * ============================================================
 */

#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <util/twi.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 16000000UL   /* Frecuencia del oscilador (ajustar según el proyecto) */
#endif

/* ------------------------------------------------------------
 *  Códigos de retorno propios de la librería
 * ------------------------------------------------------------ */
#define I2C_OK              0
#define I2C_ERROR           1

/* Bit R/W que se agrega a la dirección de 7 bits del esclavo */
#define I2C_WRITE           0
#define I2C_READ            1

/* Frecuencia SCL por defecto (100 KHz, la más utilizada) */
#define I2C_SCL_100KHZ   100000UL
#define I2C_SCL_400KHZ   400000UL

/* ------------------------------------------------------------
 *  Funciones de bajo nivel (control directo del bus)
 * ------------------------------------------------------------ */


void i2c_init(uint32_t scl_freq); // Inicializa el módulo TWI como Maestro a la frecuencia SCL indicada 

/* Genera la condición de START (o START repetido) y envía SLA+R/W
 * address : dirección de 7 bits del esclavo (sin desplazar)
 * rw      : I2C_WRITE (0) o I2C_READ (1)
 * Retorna I2C_OK si hubo ACK, I2C_ERROR en caso contrario           */

uint8_t i2c_start(uint8_t address, uint8_t rw);

void i2c_stop(void); // Genera la condición de STOP

uint8_t i2c_write(uint8_t data); //Envía un byte de datos por el bus. Retorna I2C_OK / I2C_ERROR

uint8_t i2c_read_ack(void);// Lee un byte y envía ACK (para pedir más datos)

uint8_t i2c_read_nack(void); // Lee un byte y envía NACK (para indicar que es el último byte) 

uint8_t i2c_get_status(void); //Retorna el código de estado actual del registro TWSR (bits 7:3)

/* ------------------------------------------------------------
 *  Funciones de alto nivel (uso típico con sensores/EEPROM/RTC)
 * ------------------------------------------------------------ */

uint8_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data); // Escribe un byte en un registro interno de un dispositivo esclavo 

uint8_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data); // Lee un byte de un registro interno de un dispositivo esclavo

uint8_t i2c_read_multi(uint8_t dev_addr, uint8_t reg, uint8_t *buffer, uint8_t len); // Lee "len" bytes consecutivos a partir de un registro interno

uint8_t i2c_write_multi(uint8_t dev_addr, uint8_t reg, uint8_t *buffer, uint8_t len); // Escribe "len" bytes consecutivos a partir de un registro interno

/* ------------------------------------------------------------
 *  Funciones opcionales para configurar el ATmega328P como Esclavo
 * ------------------------------------------------------------ */

/* Configura la dirección propia del dispositivo (registro TWAR)
 * own_address : dirección de 7 bits propia
 * gen_call    : 1 para responder a General Call, 0 para ignorarlo */
void i2c_slave_init(uint8_t own_address, uint8_t gen_call);


void i2c_slave_set_addr_mask(uint8_t mask); // Configura la máscara de dirección (registro TWAMR)

uint8_t i2c_slave_receive(void); // OJO función bloqueante

#endif /* I2C_H */