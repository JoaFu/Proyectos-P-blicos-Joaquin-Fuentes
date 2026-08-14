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

/* Recepción por interrupción (buffer de 1 byte) */
uint8_t UART_DatoDisponible(void); // 1 si llegó un byte nuevo por RX
char UART_LeerChar(void);          // Consume el byte recibido

#endif /* UART_H_ */