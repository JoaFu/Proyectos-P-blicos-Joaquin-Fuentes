# Compilador AVR

Un compilador de ensamblador AVR simplificado que traduce código assembly a código máquina hexadecimal. Implementa un subconjunto de instrucciones de la arquitectura AVR (como la usada en Arduino) con soporte para etiquetas, saltos condicionales y operaciones de I/O.

## Características

- **16 instrucciones AVR** implementadas: `ADD`, `SUB`, `AND`, `CLR`, `ANDI`, `CPI`, `LDI`, `LDS`, `STS`, `MOV`, `IN`, `OUT`, `JMP`, `BRBS`, `SBI`, `CBI`
- **Saltos condicionales** con alias (`BREQ`, `BRCS`)
- **Sistema de etiquetas** para facilitar navegación en el código
- **Mapeo de puertos I/O** predefinidos (`PORTB`, `PORTC`, `PORTD`, `PINB`, `PINC`, `PIND`, `SREG`)
- **Compilación en dos pasadas** para resolución de referencias hacia adelante
- **Múltiples formatos numéricos**: decimal, hexadecimal (`0x`), binario (`0b`)

## Requisitos

- Python 3.6 o superior

## Uso

```bash
python compilador.py [archivo_entrada.asm] [archivo_salida.hex]
```

Si no se especifican argumentos, usa por defecto `programa.asm` como entrada y `instrucciones.hex` como salida.

### Ejemplo

```bash
python compilador.py mi_programa.asm salida.hex
```

## Sintaxis del Ensamblador

### Estructura básica

```assembly
; Esto es un comentario

inicio:
    LDI R16, 0xFF      ; Cargar valor inmediato
    OUT PORTB, R16     ; Escribir a puerto
    JMP inicio         ; Salto incondicional
```

### Instrucciones soportadas

#### Operaciones aritméticas y lógicas
- `ADD Rd, Rr` - Suma dos registros
- `SUB Rd, Rr` - Resta dos registros
- `AND Rd, Rr` - AND lógico entre registros
- `CLR Rd` - Limpia registro (pone en 0)
- `ANDI Rd, K` - AND con constante (solo R16-R31)

#### Carga y almacenamiento
- `LDI Rd, K` - Carga valor inmediato (solo R16-R31)
- `LDS Rd, k` - Carga desde memoria (solo R16-R31)
- `STS k, Rr` - Almacena en memoria (solo R16-R31)
- `MOV Rd, Rr` - Copia entre registros

#### Entrada/Salida
- `IN Rd, A` - Lee desde puerto I/O
- `OUT A, Rr` - Escribe a puerto I/O
- `SBI A, b` - Pone bit en 1 en puerto
- `CBI A, b` - Pone bit en 0 en puerto

#### Control de flujo
- `JMP k` - Salto incondicional
- `BRBS s, k` - Salta si bit del SREG está en 1
- `BREQ label` - Salta si es igual (Z=1)
- `BRCS label` - Salta si hay carry (C=1)

#### Comparación
- `CPI Rd, K` - Compara registro con constante (solo R16-R31)

### Puertos predefinidos

El compilador reconoce estos nombres de puertos:

```
PORTB = 0x05    PINB = 0x03
PORTC = 0x08    PINC = 0x06
PORTD = 0x0B    PIND = 0x09
SREG  = 0x3F
```

### Bits del SREG

```
C = 0  (Carry)
Z = 1  (Zero)
```

## Ejemplo completo

```assembly
; Programa que parpadea un LED en PORTB bit 5

inicio:
    LDI R16, 0b00100000    ; Configurar bit 5 como salida
    OUT PORTB, R16         ; Encender LED
    
    LDI R17, 0xFF          ; Cargar contador
delay:
    SUB R17, R16           ; Decrementar
    CPI R17, 0             ; Comparar con 0
    BREQ toggle            ; Si es 0, cambiar estado
    JMP delay              ; Continuar delay
    
toggle:
    IN R18, PORTB          ; Leer estado actual
    ANDI R18, 0b00100000   ; Aislar bit 5
    BREQ encender          ; Si está apagado, encender
    CBI PORTB, 5           ; Apagar bit 5
    JMP inicio
    
encender:
    SBI PORTB, 5           ; Encender bit 5
    JMP inicio
```

## Formato de salida

El compilador genera un archivo `.hex` con una instrucción por línea en formato hexadecimal de 16 bits:

```
9F0F
BF05
E005
...
```

## Arquitectura del compilador

### Primera pasada (First Pass)
- Escanea el archivo completo
- Identifica y registra todas las etiquetas con sus direcciones
- Calcula direcciones de memoria para cada instrucción

### Segunda pasada (Second Pass)
- Ensambla cada instrucción a código máquina
- Resuelve referencias a etiquetas
- Genera el archivo hexadecimal final

### Codificación de instrucciones

Cada instrucción AVR se codifica en 16 bits siguiendo el formato específico de la arquitectura:

```
Ejemplo: ADD R16, R17
Formato: 0000 00rd dddd rrrr
Resultado: 0001 0000 0001 0001 = 0x1011
```

## Limitaciones

- Subconjunto limitado del ISA AVR completo
- Sin soporte para instrucciones de 32 bits
- Saltos relativos limitados a ±63 palabras
- Sin directivas de ensamblador (.org, .equ, etc.)
- Sin macros ni preprocesador

## Manejo de errores

El compilador reporta errores con:
- Número de línea donde ocurrió el error
- Contenido de la línea problemática
- Descripción del error

Ejemplo:
```
Error en línea 15: LDI R5, 100
  LDI solo acepta R16-R31
```

## Contribuciones

Las contribuciones son bienvenidas. Algunas ideas para expandir el proyecto:

- Agregar más instrucciones AVR
- Implementar directivas de ensamblador
- Soporte para macros
- Generador de archivos Intel HEX para programación real
- Desensamblador (hex → asm)
- Optimizaciones de código

## Licencia

Este proyecto es de código abierto y está disponible bajo la licencia MIT.

## Autor

Creado como proyecto educativo para comprender compiladores y arquitectura de computadoras a bajo nivel.

---

**Nota**: Este compilador es con fines educativos. Para desarrollo real en AVR, utiliza herramientas oficiales como `avr-gcc` y `avrdude`.
