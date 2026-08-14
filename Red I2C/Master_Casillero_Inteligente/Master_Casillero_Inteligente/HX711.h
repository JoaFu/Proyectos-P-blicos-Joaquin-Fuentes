/*
 * ============================================================
 *  HX711.h - Driver de bajo nivel (bit-banging) para HX711
 *  ATmega328P - Canal A, ganancia 128
 *
 *  DOUT = PD4 (entrada)
 *  SCK  = PD5 (salida)
 * ============================================================
 */
#ifndef HX711_H
#define HX711_H

#include <stdint.h>

/* Configura pines DOUT/SCK. SCK queda en bajo -> modulo activo. */
void HX711_Init(void);

/* Retorna 1 si el HX711 tiene un dato listo para leer (DOUT en bajo).
 * Usar esto en el loop principal para NO bloquear esperando datos.  */
uint8_t HX711_IsReady(void);

/* Lee 24 bits crudos (complemento a 2) + 1 pulso extra (canal A, ganancia 128).
 * BLOQUEANTE por ~50us (protegido con cli/sei), y espera activamente si
 * aun no esta listo -> llamar solo despues de comprobar HX711_IsReady(). */
int32_t HX711_ReadRaw(void);

/* Promedia 'num_muestras' lecturas crudas y las guarda como offset (tara). */
void HX711_Tare(uint8_t num_muestras);

int32_t HX711_GetOffset(void);

/* Ajusta el factor de escala (unidades crudas por gramo). Calibrar con
 * un peso patron: escala = (raw_con_peso - offset) / peso_conocido_g   */
void HX711_SetScale(float escala);

/* Retorna el peso en gramos ya compensado por tara y escala. */
float HX711_GetWeightGrams(void);

/* Apaga el HX711 (SCK en alto >60us) / lo reactiva (SCK en bajo). */
void HX711_PowerDown(void);
void HX711_PowerUp(void);

#endif /* HX711_H */