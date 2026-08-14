/*****************
*Author: Joaquín Fuentes, Luis Arriaza
*Control de motor dc mediante el driver TB6612FNG
*****************/
#include "motor_dc_atmega328p.h"
// Inicializción de pines
void MotorA_Init(void){
	// Haz que los pines sean de salida
	MOTOR_AIN1_DDR |= (1 << MOTOR_AIN1_PIN);
	MOTOR_AIN2_DDR |= (1 << MOTOR_AIN2_PIN);
	MOTOR_STBY_DDR |= (1 << MOTOR_STBY_PIN);
	MOTOR_PWM_DDR  |= (1 << MOTOR_PWM_PIN);
	// Detener el motor y habilitar el driver
	MOTOR_AIN1_PORT &= ~(1 << MOTOR_AIN1_PIN);
	MOTOR_AIN2_PORT &= ~(1 << MOTOR_AIN2_PIN);
	MotorA_Standby(MOTOR_ENABLE); // 1 = Activo, 0 = Standby
	// Configuración del Timer 0 para Fast PWM
	TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << CS01)   | (1 << CS00); // Prescaler de 64
	OCR0A  = 0;
}
/***********************/
// Non interrupt
/***********************/
void MotorA_Drive(int16_t speed){
	if (speed > 0){
		MOTOR_AIN1_PORT |=  (1 << MOTOR_AIN1_PIN);
		MOTOR_AIN2_PORT &= ~(1 << MOTOR_AIN2_PIN);
		// No más de 255
		if (speed > MOTOR_MAX_SPEED) speed = MOTOR_MAX_SPEED;
		OCR0A = (uint8_t)speed;
	}
	else if (speed < 0){
		MOTOR_AIN1_PORT &= ~(1 << MOTOR_AIN1_PIN);
		MOTOR_AIN2_PORT |=  (1 << MOTOR_AIN2_PIN);
		speed = -speed;
		if (speed > MOTOR_MAX_SPEED) speed = MOTOR_MAX_SPEED;
		OCR0A = (uint8_t)speed;
	}
	else {
		MOTOR_AIN1_PORT &= ~(1 << MOTOR_AIN1_PIN);
		MOTOR_AIN2_PORT &= ~(1 << MOTOR_AIN2_PIN);
		OCR0A = 0;
	}
}
// Freno en seco
void MotorA_Brake(void){
	MOTOR_AIN1_PORT |= (1 << MOTOR_AIN1_PIN);
	MOTOR_AIN2_PORT |= (1 << MOTOR_AIN2_PIN);
	OCR0A = 0; // el freno se da por la lógica de AIN
}
// Controla el pin de Standby 0 = Driver apagado, 1 = Driver encendido
void MotorA_Standby(uint8_t state){
	if (state){
		MOTOR_STBY_PORT |= (1 << MOTOR_STBY_PIN);
	}
	else{
		MOTOR_STBY_PORT &= ~(1 << MOTOR_STBY_PIN);
	}
}
// Detiene el motor sin aplicar freno (coast, AIN1=AIN2=0)
void MotorA_Stop(void){
	MOTOR_AIN1_PORT &= ~(1 << MOTOR_AIN1_PIN);
	MOTOR_AIN2_PORT &= ~(1 << MOTOR_AIN2_PIN);
	OCR0A = 0;
}