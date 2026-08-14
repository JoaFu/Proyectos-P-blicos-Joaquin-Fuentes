/*
 * UART.h
 *
 * Created: 
 * Author: 
 * Description: Librería UART para ATMega328P
 */
#ifndef UART_H_
#define UART_H_
#include <stdint.h>
void initUART(void);
void writeChar(char caracter);
void writeString(char* string);
/* Recepción por interrupción (buffer circular) */
uint8_t UART_DatoDisponible(void); // 1 si hay al menos un byte nuevo por RX pendiente de leer
char UART_LeerChar(void);          // Consume el byte mas antiguo recibido
#endif /* UART_H_ */