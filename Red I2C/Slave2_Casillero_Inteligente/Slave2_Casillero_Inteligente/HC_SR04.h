// HC_SR04.h
// Driver de bajo nivel para HC-SR04 en ATmega328P
// TRIG -> PD2 (salida), ECHO -> PD3 (entrada)
// Usa Timer2 (8 bits + overflow) como base de tiempo, dejando Timer1 libre

#ifndef HC_SR04_H_
#define HC_SR04_H_

// Configura pines, arranca Timer2. Llamar una vez antes de leer.
void HC_SR04_Init(void);

// Dispara el sensor, mide el eco (bloqueante) y retorna distancia en cm.
// Retorna -1.0f si hubo timeout.
float HC_SR04_ReadDistance(void);

#endif /* HC_SR04_H_ */