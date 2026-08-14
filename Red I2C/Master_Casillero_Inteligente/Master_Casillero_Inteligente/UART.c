/*
 * UART.c
 *
 * Created: 
 * Author: 
 * Description: Implementación UART para ATMega328P
 *
 * CORREGIDO: la recepción usaba un "buzon" de 1 solo byte
 * (uart_dato_rx / uart_dato_disponible). Si llegaba un byte nuevo
 * antes de que el programa principal alcanzara a leer el anterior
 * (algo muy probable, ya que el loop principal del Master hace
 * lecturas I2C, actualiza la LCD y tiene delays), la ISR lo
 * sobreescribia y el byte anterior se perdia sin dejar rastro. Con
 * comandos de varios caracteres (ej. "Manual_On\r\n") eso corrompia
 * el string recibido casi siempre. Se reemplazo por un buffer
 * circular (ring buffer) de 32 bytes que encola todos los bytes
 * recibidos sin perder ninguno mientras no se llene por completo.
 * La interfaz publica (UART_DatoDisponible / UART_LeerChar) no
 * cambio, por lo que no requiere tocar nada en el codigo que ya la
 * usa (Master).
 */
#include "UART.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Tamaño del buffer circular de recepcion. Debe ser potencia de 2
// para poder usar una mascara en vez de modulo (mas rapido en un AVR).
#define UART_RX_BUFFER_SIZE 32
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)

static volatile char    uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t uart_rx_head = 0; // proximo indice a escribir (ISR)
static volatile uint8_t uart_rx_tail = 0; // proximo indice a leer (main)

void initUART(void)
{
	// Configurar pines RX y TX
	DDRD &= ~(1<<DDD0);
	DDRD |=	 (1<<DDD1);
	
	UCSR0A = 0;
	// Habilitando interrupciones, habilitando RX y TX del UART0
	UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
	// Async, Paridad deshabilitada, 1 stop bit, 8 data bits
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
	// Setear UBRR0 = 103
	UBRR0 = 103;
}
void writeChar(char caracter)
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = caracter;
}
void writeString(char* string)
{
	while (*string != '\0')
	{
		writeChar(*string);
		string++;
	}
}
// Indica si hay al menos un byte pendiente de leer en el buffer circular
uint8_t UART_DatoDisponible(void)
{
	return (uart_rx_head != uart_rx_tail);
}
// Consume el byte mas antiguo del buffer circular (FIFO)
char UART_LeerChar(void)
{
	char dato = uart_rx_buffer[uart_rx_tail];
	uart_rx_tail = (uart_rx_tail + 1) & UART_RX_BUFFER_MASK;
	return dato;
}
// Recepción por interrupción: encola el byte en el buffer circular
ISR(USART_RX_vect)
{
	uint8_t siguiente_head = (uart_rx_head + 1) & UART_RX_BUFFER_MASK;
	char dato = UDR0; // Siempre hay que leer UDR0 para limpiar la bandera RXC0

	// Si el buffer esta lleno, se descarta el byte nuevo (evita que el
	// puntero de escritura se cruce con el de lectura y corrompa datos
	// ya encolados). Con 32 bytes de margen esto no deberia ocurrir en
	// uso normal.
	if (siguiente_head != uart_rx_tail)
	{
		uart_rx_buffer[uart_rx_head] = dato;
		uart_rx_head = siguiente_head;
	}
}