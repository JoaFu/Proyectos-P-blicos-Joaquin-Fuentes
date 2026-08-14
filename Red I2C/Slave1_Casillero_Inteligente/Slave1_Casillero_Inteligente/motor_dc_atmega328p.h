#ifndef MOTOR_DC_ATMEGA328P_H_
#define MOTOR_DC_ATMEGA328P_H_

#include <avr/io.h>
#include <stdint.h>

// AIN1 - PD5 / D5
#define MOTOR_AIN1_PORT     PORTD
#define MOTOR_AIN1_DDR      DDRD
#define MOTOR_AIN1_PIN      PD5

// AIN2 - PD4 / D4
#define MOTOR_AIN2_PORT     PORTD
#define MOTOR_AIN2_DDR      DDRD
#define MOTOR_AIN2_PIN      PD4

// PWMA - PD6 / D6 / OC0A
#define MOTOR_PWM_PORT      PORTD
#define MOTOR_PWM_DDR       DDRD
#define MOTOR_PWM_PIN       PD6

// STBY - PD7 / D7
#define MOTOR_STBY_PORT     PORTD
#define MOTOR_STBY_DDR      DDRD
#define MOTOR_STBY_PIN      PD7

#define MOTOR_ENABLE        1U
#define MOTOR_STANDBY       0U

#define MOTOR_MAX_SPEED     255
#define MOTOR_MIN_SPEED    -255

void MotorA_Init(void);
void MotorA_Drive(int16_t speed);
void MotorA_Brake(void);
void MotorA_Standby(uint8_t state);
void MotorA_Stop(void);

#endif