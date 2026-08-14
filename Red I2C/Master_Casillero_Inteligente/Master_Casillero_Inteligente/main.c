/*
 * Master_Casillero_Inteligente
 *
 * Created: 
 * Author: Joaquín Fuentes y Luis Arriaza
 * Description: Master con pantalla LCD y enlace UART hacia el ESP32
 *              (que a su vez habla con Adafruit IO). Lee periódicamente
 *              los registros de MCU2/MCU3 por I2C, actualiza la LCD y
 *              procesa comandos de control (modo manual + accionamiento
 *              de servo, stepper y motor DC) enviados desde el ESP32.
 *
 *

****************************************/
// Encabezado (Libraries)
/****************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "I2C.h"
#include "UART.h"
#include "Master_config.h"
#include "AHT10.h"
#include "lcd_hd44780.h"



// Variables de control de tiempo y marquesina LCD
static volatile uint16_t contador_lectura_ms = 0; // volatile: se modifica en ISR y se lee en main
static volatile uint8_t tick = 0;
static volatile uint8_t flag_dezplazamiento = 0;
// Columnas para el display
#define COL_HUMEDAD    0
#define COL_TEMP       9
#define COL_CERROJO    17
#define COL_PUERTA     26

// Variables globales para almacenar lecturas de los esclavos I2C
static uint8_t var_humedad = 0;
static float   var_temperatura = 0.0f; // Reemplaza a "var_masa" (celda de carga retirada)
static uint8_t var_cerradura = 0; // 0 = Bloqueado, 1 = Desbloqueado
static uint8_t var_puerta = 0;    // 0 = Cerrada, 1 = Abierta

// Historial para control de transmisiones óptimas por UART hacia el ESP32
static uint8_t ultimo_estado_puerta = 0xFF;
static uint8_t ultimo_estado_pestillo = 0xFF;

// Estado del modo manual: 0 = Automatico, 1 = Manual
static uint8_t modo_manual = 0;

// Modo automático DC:
typedef enum {
	AUTO_DC_OFF = 0,
	AUTO_DC_ETAPA_100,
	AUTO_DC_ETAPA_50,
	AUTO_DC_ETAPA_20
} auto_dc_estado_t;

static auto_dc_estado_t auto_dc_estado = AUTO_DC_OFF;
static volatile uint16_t auto_dc_timer_ms = 0; // temporizador propio, no bloqueante (se decrementa en ISR Timer0)

/****************************************/
// Function prototypes
/****************************************/
static void InicializarTimer0(void);
static void InicializarTimer1(void);
static void LeerYMostrarMCU2(void);
static void LeerYMostrarAHT10(void);
static void ProcesarComandoESP32(char *cadena);
static void LeerBufferUART(void);
static void ActualizarPantallaLCD(void);
static void EjecutarComandoDC(int16_t porcentaje);
static void ControlAutomaticoDC(void);

/****************************************/
// Main Function
/****************************************/
int main(void)
{
	// Inicialización de módulos de Hardware
	InicializarTimer0();
	InicializarTimer1();
	initUART();
	i2c_init(I2C_SCL_100KHZ);
	AHT10_Init();
	lcd_init();

	contador_lectura_ms = PERIODO_LECTURA_MS;

	// Habilitar interrupciones globales
	sei();

	// Configurar encabezado estático en la primera línea (Fila 0)
	lcd_set_cursor(0, 0);
	lcd_write_string("Humedad  Temp  Cerrojo  Puerta");

	while (1)
	{
		// Lectura periódica de esclavos I2C y envío de telemetría al ESP32
		if (contador_lectura_ms == 0)
		{
			contador_lectura_ms = PERIODO_LECTURA_MS;

			LeerYMostrarMCU2();
			LeerYMostrarAHT10();
		}

		// Escuchar y procesar comandos de control enviados desde el ESP32
		LeerBufferUART();
		
		// Lógica no bloqueante de encendido automatico del DC segun humedad
		ControlAutomaticoDC();

		// Actualizar datos en la pantalla LCD basados en las lecturas de los esclavos
		ActualizarPantallaLCD();

		// Pequeño retardo para estabilidad visual e incremento de fluidez en pantalla
		_delay_ms(50);

		// Control del desplazamiento físico (Efecto Marquesina de la LCD)
		if (flag_dezplazamiento == 0)
		{
			if (tick < 17)
			{
				lcd_command(LCD_SHIFT_DISPLAY_LEFT);
				flag_dezplazamiento = 1;
			}
			else
			{
				tick = 0;
				lcd_command(LCD_RETURN_HOME);
				flag_dezplazamiento = 1;
			}
		}
	}
}

/****************************************/
// Subrutinas de Control y Pantalla
/****************************************/

// Escribe los valores reales leídos por I2C en la Fila 1 de la pantalla LCD
static void ActualizarPantallaLCD(void)
{
	char buffer_lcd[16];
	char buffer_temp[8];

	// Humedad (Fila 1)
	lcd_set_cursor(COL_HUMEDAD, 1);
	snprintf(buffer_lcd, sizeof(buffer_lcd), "%u%%  ", var_humedad);
	lcd_write_string(buffer_lcd);

	// Temperatura (Fila 1) - reemplaza a la antigua "Masa"
	lcd_set_cursor(COL_TEMP, 1);
	{
		/* dtostrf en vez de "%f" en snprintf: evita depender de que el
		 toolchain tenga enlazada la variante de printf con soporte de
		 punto flotante (el proyecto ya usaba itoa/formateo manual). */
		char *p;
		dtostrf(var_temperatura, 5, 1, buffer_temp); // ej: " 23.5" o "-5.0"
		p = buffer_temp;
		while (*p == ' ') p++; // recorta espacios de relleno a la izquierda
		snprintf(buffer_lcd, sizeof(buffer_lcd), "%sC  ", p);
	}
	lcd_write_string(buffer_lcd);
	
	// Cerrojo / Cerradura (Fila 1)
	lcd_set_cursor(COL_CERROJO, 1);
	lcd_write_string("    "); // Limpieza rápida de residuos anteriores
	lcd_set_cursor(COL_CERROJO, 1);
	if (var_cerradura == 1) {
		lcd_write_string("DESB "); // Desbloqueado (MCU2_PESTILLO_DESBLOQUEADO)
		} else {
		lcd_write_string("BLOQ "); // Bloqueado
	}

	// Puerta (Fila 1)
	lcd_set_cursor(COL_PUERTA, 1);
	if (var_puerta == 1) {
		lcd_write_string("ABIER "); // Abierta (MCU2_PUERTA_ABIERTA)
		} else {
		lcd_write_string("CERR  "); // Cerrada
	}
}

// Reconstruye el búfer UART proveniente del ESP32 hasta detectar el fin de línea
static void LeerBufferUART(void)
{
	static char buffer_rx[24]; // Aumentado de 16 a 24 para comandos parametrizados
	static uint8_t indice_rx = 0;

	while (UART_DatoDisponible())
	{
		char c = UART_LeerChar();

		if (c == '\n' || c == '\r')
		{
			if (indice_rx > 0)
			{
				buffer_rx[indice_rx] = '\0';
				ProcesarComandoESP32(buffer_rx);
				indice_rx = 0;
			}
		}
		else if (indice_rx < (sizeof(buffer_rx) - 1))
		{
			buffer_rx[indice_rx++] = c;
		}
	}
}

// Procesa las directivas del ESP32 y envía comandos a los esclavos por I2C
static void ProcesarComandoESP32(char *cadena)
{
	uint8_t datos[2];

	// --- CONTROL DE MODO MANUAL / AUTOMATICO ---
	if (strcmp(cadena, "Manual_On") == 0)
	{
		modo_manual = 1;
		writeString("ACK_Manual_On\n");
		return;
	}
	else if (strcmp(cadena, "Manual_Off") == 0)
	{
		modo_manual = 0;
		writeString("ACK_Manual_Off\n");
		return;
	}

	// A partir de aqui, los comandos de accionamiento SOLO se ejecutan si
	// el sistema esta en modo manual. Evita mover motores por accidente

	if (!modo_manual)
	{
		return;
	}

	// --- COMANDOS DE SERVO (Cerradura / Pestillo MCU2) ---
	// Se aceptan "Servo_close/Servo_open" y tambien "Lock_On/Lock_Off"
	// (alias que usa el dashboard de Adafruit/ESP32 para el mismo control).
	if (strcmp(cadena, "Servo_close") == 0 ||
	    strcmp(cadena, "Lock_On") == 0     ||
	    strcmp(cadena, "Lock_ON") == 0)
	{
		i2c_write_reg(MCU2_I2C_ADDR, MCU2_REG_CMD, MCU2_CMD_BLOQUEAR);
	}
	else if (strcmp(cadena, "Servo_open") == 0 ||
	         strcmp(cadena, "Lock_Off") == 0   ||
	         strcmp(cadena, "Lock_OFF") == 0)
	{
		i2c_write_reg(MCU2_I2C_ADDR, MCU2_REG_CMD, MCU2_CMD_DESBLOQUEAR);
	}

	// --- COMANDOS DE STEPPER (Carrusel MCU3) ---
	else if (strcmp(cadena, "Stepper_Iz") == 0)
	{
		i2c_write_reg(MCU3_I2C_ADDR, MCU3_REG_CMD, MCU3_CMD_HOME);
	}
	else if (strcmp(cadena, "Stepper_Stop") == 0)
	{
		i2c_write_reg(MCU3_I2C_ADDR, MCU3_REG_CMD, MCU3_CMD_LIMPIAR_ALERTA);
	}
	else if (strncmp(cadena, "Stepper_Der(", 12) == 0)
	{
		// Formato "Stepper_Der(N)" -> mueve el carrusel a la posicion N (0-5)
		char *inicio = cadena + 12;
		char *fin = strchr(inicio, ')');
		if (fin != NULL)
		{
			*fin = '\0';
			int posicion = atoi(inicio);

			if (posicion < 0) posicion = 0;
			if (posicion > 5) posicion = 5;

			datos[0] = MCU3_CMD_IR_POSICION;
			datos[1] = (uint8_t)posicion;
			i2c_write_multi(MCU3_I2C_ADDR, MCU3_REG_CMD, datos, 2);
		}
	}
	else if (strcmp(cadena, "Stepper_Der") == 0)
	{
		// Compatibilidad: si llega sin parametro, se asume posicion 1
		datos[0] = MCU3_CMD_IR_POSICION;
		datos[1] = 1;
		i2c_write_multi(MCU3_I2C_ADDR, MCU3_REG_CMD, datos, 2);
	}

    // --- COMANDO DE MOTOR DC (MCU2, driver TB6612FNG) ---
    // Nuevo Formato "DC[Valor]": "Valor" va de 0 (apagado) a 100 (máximo).
	else if (strncmp(cadena, "DC", 2) == 0)
	{
		int porcentaje = atoi(cadena + 2);
		EjecutarComandoDC(porcentaje);
	}
    // Formato "DC_Brake": frenado dinámico inmediato del motor DC (MCU2)
    else if (strcmp(cadena, "DC_Brake") == 0)
    {
	    datos[0] = MCU2_CMD_DC_FRENO;
	    datos[1] = 0;
	    i2c_write_multi(MCU2_I2C_ADDR, MCU2_REG_CMD_DC, datos, 2);
    }

}

// Lee el esclavo MCU2 (Puerta y Pestillo), actualiza variables globales y reporta al ESP32
static void LeerYMostrarMCU2(void)
{
	uint8_t regs[MCU2_NUM_REGISTROS];
	uint8_t resultado;

	resultado = i2c_read_multi(MCU2_I2C_ADDR, 0, regs, MCU2_NUM_REGISTROS);
	if (resultado != I2C_OK) return;

	// Guardar lecturas en variables globales para que la LCD las pinte
	var_puerta = (regs[MCU2_REG_ESTADO_PUERTA] == MCU2_PUERTA_ABIERTA) ? 1 : 0;
	var_cerradura = (regs[MCU2_REG_ESTADO_PESTILLO] == MCU2_PESTILLO_DESBLOQUEADO) ? 0 : 1;

	// Envío de reportes condicionales UART al ESP32 ante cambios físicos reales
	if (regs[MCU2_REG_ESTADO_PUERTA] != ultimo_estado_puerta)
	{
		ultimo_estado_puerta = regs[MCU2_REG_ESTADO_PUERTA];
		if (ultimo_estado_puerta == MCU2_PUERTA_ABIERTA) {
			writeString("Gate_Op\n");
			} else {
			writeString("Gate_Cl\n");
		}
	}

	if (regs[MCU2_REG_ESTADO_PESTILLO] != ultimo_estado_pestillo)
	{
		ultimo_estado_pestillo = regs[MCU2_REG_ESTADO_PESTILLO];
		if (ultimo_estado_pestillo == MCU2_PESTILLO_DESBLOQUEADO) {
			writeString("Lock_Off\n");
			} else {
			writeString("Lock_On\n");
		}
	}
}

// Lee el sensor ambiental AHT10 (humedad Y temperatura en la misma
// transaccion I2C), actualiza las variables globales y reporta ambos
// valores al ESP32. La temperatura se envia bajo el prefijo "M", el
// mismo que antes usaba la masa/celda de carga, para no tener que
// modificar los feeds ya configurados en Adafruit IO.
static void LeerYMostrarAHT10(void)
{
	float temperatura;
	float humedad;
	char buffer_uart[8];
	char *p;

	if (!AHT10_Read(&temperatura, &humedad)) return;

	// Validar rango operacional de humedad (0 - 100%)
	if (humedad < 0.0) humedad = 0.0;
	if (humedad > 100.0) humedad = 100.0;

	var_humedad = (uint8_t)humedad; // Guardar lectura en variable global para la LCD

	// Validar rango tipico del AHT10 (-40 a 85 °C) como resguardo
	if (temperatura < -40.0) temperatura = -40.0;
	if (temperatura > 85.0)  temperatura = 85.0;

	var_temperatura = temperatura; // Guardar lectura en variable global para la LCD

	// Enviar string formateado "H'valor'" al ESP32 (humedad, sin cambios)
	writeString("H");
	itoa(var_humedad, buffer_uart, 10);
	writeString(buffer_uart);
	writeString("\n");

	// Enviar string formateado "M'valor'" al ESP32 (ahora es temperatura,
	// pero se conserva el prefijo "M" que antes identificaba la masa)
	writeString("M");
	dtostrf(var_temperatura, 5, 1, buffer_uart); // ej: " 23.5" o "-5.0"
	p = buffer_uart;
	while (*p == ' ') p++; // recorta espacios de relleno a la izquierda
	writeString(p);
	writeString("\n");
}

// Mapea un porcentaje (0-100) a PWM y escribe el comando DC del MCU2.
// La reutilizan tanto el comando manual "DC(N)" como el ciclo automatico.
static void EjecutarComandoDC(int16_t porcentaje)
{
	uint8_t datos[2];

	if (porcentaje < 0)   porcentaje = 0;
	if (porcentaje > 100) porcentaje = 100;

	if (porcentaje > 0)
	{
		uint8_t velocidad_pwm = (uint8_t)((porcentaje * 255) / 100);
		datos[0] = MCU2_CMD_DC_ADELANTE;
		datos[1] = velocidad_pwm;
	}
	else
	{
		datos[0] = MCU2_CMD_DC_DETENER;
		datos[1] = 0;
	}

	i2c_write_multi(MCU2_I2C_ADDR, MCU2_REG_CMD_DC, datos, 2);
}

// FSM automática
static void ControlAutomaticoDC(void)
{
	static uint8_t flag_transicion = 0; // Evita múltiples transiciones por ciclo
	
	if (modo_manual)
	{
		if (auto_dc_estado != AUTO_DC_OFF)
		{
			EjecutarComandoDC(0);
			auto_dc_estado = AUTO_DC_OFF;
			flag_transicion = 0;
		}
		return;
	}

	// Solo procesar si no hubo transición en este ciclo
	if (flag_transicion) {
		flag_transicion = 0; // Resetear para el próximo ciclo
		return;
	}

	switch (auto_dc_estado)
	{
		case AUTO_DC_OFF:
		if (var_humedad >= UMBRAL_HUMEDAD_AUTO_DC)
		{
			EjecutarComandoDC(100);
			auto_dc_timer_ms = AUTO_DC_ETAPA1_MS;
			auto_dc_estado = AUTO_DC_ETAPA_100;
			flag_transicion = 1; // Marcar que hubo transición
		}
		break;

		case AUTO_DC_ETAPA_100:
		if (auto_dc_timer_ms == 0)
		{
			EjecutarComandoDC(50);
			auto_dc_timer_ms = AUTO_DC_ETAPA2_MS;
			auto_dc_estado = AUTO_DC_ETAPA_50;
			flag_transicion = 1;
		}
		break;

		case AUTO_DC_ETAPA_50:
		if (auto_dc_timer_ms == 0)
		{
			EjecutarComandoDC(20);
			auto_dc_timer_ms = AUTO_DC_ETAPA3_MS;
			auto_dc_estado = AUTO_DC_ETAPA_20;
			flag_transicion = 1;
		}
		break;

		case AUTO_DC_ETAPA_20:
		if (auto_dc_timer_ms == 0)
		{
			EjecutarComandoDC(0);
			auto_dc_estado = AUTO_DC_OFF;
			flag_transicion = 1;
		}
		break;
	}
}

/****************************************/
// Configuraciones de Hardware (Timers)
/****************************************/

// Configura Timer0 en modo CTC para generar la bandera de control interna de 1ms
static void InicializarTimer0(void)
{
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01) | (1 << CS00); // Prescaler 64
	OCR0A  = 249;                        // 1ms @ 16MHz
	TIMSK0 = (1 << OCIE0A);
}

static void InicializarTimer1(void)
{
	TCCR1B = (1 << WGM12) | (1 << CS12); // Modo CTC, Prescaler 256
	TCCR1A = 0x00;
	OCR1AH = 0xF4; // Configuración exacta para 1 segundo @ 16MHz
	OCR1AL = 0x23;
	TIMSK1 = (1 << OCIE1A);              // Habilitar interrupción por comparación A
}

ISR(TIMER0_COMPA_vect)
{
	// Contadores
	if (contador_lectura_ms > 0) contador_lectura_ms--;
	if (auto_dc_timer_ms > 0)	auto_dc_timer_ms--;
}

ISR(TIMER1_COMPA_vect)
{
	tick++;
	flag_dezplazamiento = 0;
}