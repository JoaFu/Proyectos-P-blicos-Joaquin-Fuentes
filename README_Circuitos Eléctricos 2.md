# Corolla Montarraz - Vehículo Eléctrico de Tracción 4x4

Proyecto de robótica móvil que integra múltiples tipos de motores eléctricos para crear un sistema de tracción completo con control de velocidad, dirección y suspensión activa.

## Descripción

Sistema mecánico móvil controlado electrónicamente que combina:
- **2 motores DC** para tracción en las 4 ruedas
- **1 servomotor** para control direccional del eje frontal
- **1 motor stepper** para sistema de elevación del chasis mediante piñón-cremallera

El proyecto implementa circuitos de control basados en el temporizador 555, puentes H, MOSFETs y circuitos integrados como el CD4017 y 74HC157 para lograr un control preciso e independiente de cada actuador.

## Características principales

- Control de velocidad mediante señales PWM
- Inversión de dirección de marcha (adelante/atrás)
- Sistema de dirección controlable en 180°
- Operación independiente de todos los motores sin interferencias
- Chasis diseñado en CAD y fabricado en MDF

## Tecnologías utilizadas

### Electrónica
- Temporizador 555 (modo astable) para generación de PWM
- MOSFET IRF3205 para control de potencia eficiente
- Puente H para inversión de polaridad en motores DC
- CD4017 (contador/divisor decimal)
- 74HC157 (multiplexor 4 a 1)
- Diodos de protección y transistores de potencia

### Mecánica
- Diseño CAD de componentes estructurales
- Fabricación mediante corte láser en MDF
- Sistema de piñón-cremallera para elevación

## Circuitos implementados

El proyecto consta de tres módulos principales de control:

1. **Módulo de motores DC**: PWM con 555 + MOSFET + puente H para control bidireccional
2. **Módulo de servomotor**: 555 + potenciómetro para control angular
3. **Módulo de stepper**: 555 + CD4017 + multiplexor para control de paso y dirección

## Aplicaciones

Este tipo de sistema tiene utilidad en:
- Prototipos de robótica móvil
- Plataformas automatizadas de inspección
- Vehículos de exploración en terrenos irregulares
- Sistemas de suspensión activa
- Educación en control de motores eléctricos

## Equipo de desarrollo

Proyecto realizado como parte del curso de Circuitos Eléctricos 2 en la Universidad del Valle de Guatemala.

## Demostración

Video del proyecto en funcionamiento: [Ver en YouTube](https://youtu.be/kfCb7a841fM)

---

**Nota**: Este es un proyecto académico con fines educativos que demuestra la integración de conceptos de circuitos eléctricos, control de motores y diseño mecánico.
