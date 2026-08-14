/*
 * i2c.c
 * Driver I2C (TWI) para ATmega328P
 */

#include "I2C.h"
#include <avr/interrupt.h>

// Inicializa el módulo TWI a la frecuencia indicada.
void i2c_init(uint32_t scl_freq)
{
    // Prescaler = 1
    TWSR &= ~((1 << TWPS1) | (1 << TWPS0));

    // Configura la velocidad SCL
    TWBR = (uint8_t)(((F_CPU / scl_freq) - 16) / 2);

    // Habilita el módulo TWI
    TWCR = (1 << TWEN);
}

// Genera START y envía la dirección del esclavo.
uint8_t i2c_start(uint8_t address, uint8_t rw)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    // Espera el fin de la operación
    while (!(TWCR & (1 << TWINT)));

    uint8_t status = i2c_get_status();

    if (status != TW_START && status != TW_REP_START)
    {
        return I2C_ERROR;
    }

    uint8_t sla_rw = (address << 1) | (rw & 0x01);

    return i2c_write(sla_rw);
}

// Envía un byte al bus.
uint8_t i2c_write(uint8_t data)
{
    TWDR = data;

    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));

    uint8_t status = i2c_get_status();

    // Verifica recepción de ACK
    if (status == TW_MT_SLA_ACK ||
        status == TW_MT_DATA_ACK ||
        status == TW_MR_SLA_ACK)
    {
        return I2C_OK;
    }

    return I2C_ERROR;
}

// Lee un byte y responde con ACK.
uint8_t i2c_read_ack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);

    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}

// Lee el último byte respondiendo con NACK.
uint8_t i2c_read_nack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);

    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}

// Genera la condición STOP.
void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);

    while (TWCR & (1 << TWSTO));
}

// Devuelve el código de estado del TWI.
uint8_t i2c_get_status(void)
{
    return (TWSR & 0xF8);
}

/*==========================
 * Funciones de alto nivel
 *=========================*/

// Escribe un byte en un registro del esclavo.
uint8_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    if (i2c_start(dev_addr, I2C_WRITE) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_write(reg) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_write(data) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    i2c_stop();
    return I2C_OK;
}

// Lee un registro del esclavo.
uint8_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data)
{
    if (i2c_start(dev_addr, I2C_WRITE) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_write(reg) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    // Cambia a modo lectura
    if (i2c_start(dev_addr, I2C_READ) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    *data = i2c_read_nack();

    i2c_stop();

    return I2C_OK;
}

// Lee varios registros consecutivos.
uint8_t i2c_read_multi(uint8_t dev_addr, uint8_t reg,
                       uint8_t *buffer, uint8_t len)
{
    if (len == 0)
        return I2C_ERROR;

    if (i2c_start(dev_addr, I2C_WRITE) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_write(reg) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_start(dev_addr, I2C_READ) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        if (i == (len - 1))
            buffer[i] = i2c_read_nack();
        else
            buffer[i] = i2c_read_ack();
    }

    i2c_stop();

    return I2C_OK;
}

// Escribe varios registros consecutivos.
uint8_t i2c_write_multi(uint8_t dev_addr, uint8_t reg,
                        uint8_t *buffer, uint8_t len)
{
    if (len == 0)
        return I2C_ERROR;

    if (i2c_start(dev_addr, I2C_WRITE) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    if (i2c_write(reg) != I2C_OK)
    {
        i2c_stop();
        return I2C_ERROR;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        if (i2c_write(buffer[i]) != I2C_OK)
        {
            i2c_stop();
            return I2C_ERROR;
        }
    }

    i2c_stop();

    return I2C_OK;
}

/*==========================
 * Modo Esclavo por POLLING
 *=========================*/

// Configura la dirección propia del esclavo.
void i2c_slave_init(uint8_t own_address, uint8_t gen_call)
{
    TWAR = (own_address << 1) | (gen_call & 0x01);

    TWCR = (1 << TWEN) | (1 << TWEA);
}

// Configura la máscara de dirección.
void i2c_slave_set_addr_mask(uint8_t mask)
{
    TWAMR = (mask << 1);
}

// Recibe datos como esclavo mediante polling.
uint8_t i2c_slave_receive(void)
{
    uint8_t data = 0;
    uint8_t status;

    for (;;)
    {
        // Espera un evento del bus
        while (!(TWCR & (1 << TWINT)));

        status = i2c_get_status();

        switch (status)
        {
            case TW_SR_SLA_ACK:
            case TW_SR_GCALL_ACK:

                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
                break;

            case TW_SR_DATA_ACK:
            case TW_SR_GCALL_DATA_ACK:

                data = TWDR;

                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
                break;

            case TW_SR_STOP:

                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
                return data;

            default:

                // Reinicia el reconocimiento
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
                break;
        }
    }
}

/*==========================
 * Modo Esclavo por INTERRUPCIÓN (receptor + transmisor)
 * Protocolo de mapa de registros con índice auto-incremental.
 *=========================*/

static volatile uint8_t *i2c_regs        = 0;
static volatile uint8_t  i2c_regs_len    = 0;
static volatile uint8_t  i2c_reg_index   = 0;
static volatile uint8_t  i2c_esperando_indice = 1;

// Configura el esclavo en modo interrupción sobre un mapa de registros propio.
void i2c_slave_init_it(uint8_t direccion, volatile uint8_t *mapa_registros, uint8_t num_registros)
{
    i2c_regs      = mapa_registros;
    i2c_regs_len  = num_registros;
    i2c_reg_index = 0;
    i2c_esperando_indice = 1;

    TWAR = (direccion << 1);
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
}

ISR(TWI_vect)
{
    uint8_t status = i2c_get_status();

    switch (status)
    {
        /* ---- Modo receptor: el Master escribe ---- */
        case TW_SR_SLA_ACK:
        case TW_SR_GCALL_ACK:

            i2c_esperando_indice = 1;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        case TW_SR_DATA_ACK:
        case TW_SR_GCALL_DATA_ACK:

            if (i2c_esperando_indice)
            {
                // Primer byte de la transacción: indice de registro
                i2c_reg_index = (TWDR < i2c_regs_len) ? TWDR : 0;
                i2c_esperando_indice = 0;
            }
            else if (i2c_reg_index < i2c_regs_len)
            {
                i2c_regs[i2c_reg_index] = TWDR;
                i2c_reg_index++;
            }

            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        case TW_SR_STOP:
        case TW_SR_DATA_NACK:
        case TW_SR_GCALL_DATA_NACK:

            i2c_esperando_indice = 1;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        /* ---- Modo transmisor: el Master lee ---- */
        case TW_ST_SLA_ACK:
        case TW_ST_DATA_ACK:

            if (i2c_reg_index < i2c_regs_len)
            {
                TWDR = i2c_regs[i2c_reg_index];
                i2c_reg_index++;
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            }
            else
            {
                TWDR = 0xFF; // Fuera de rango
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            }
            break;

        case TW_ST_DATA_NACK:
        case TW_ST_LAST_DATA:

            i2c_esperando_indice = 1;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;

        /* ---- Estado inesperado: reinicia ---- */
        default:

            i2c_esperando_indice = 1;
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA) | (1 << TWIE);
            break;
    }
}