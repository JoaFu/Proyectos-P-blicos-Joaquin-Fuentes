START:
    LDI R21, 0x00      

MAIN_LOOP:
    IN   R20, PINC
    ANDI R20, 0x01 ;masking que me quede solo el bit 0
    CPI  R20, 0x00      ; Comparar con 0 (presionado)
    BRBS 1, BUTTON_OFF  ; Si Z=1 (presionado), apagar
    
    ; PINC=1 (NO presionado) → mostrar valor
    JMP SHOW_VALUE

BUTTON_OFF:
    OUT PORTD, R21      ; Apagar display
    JMP MAIN_LOOP

SHOW_VALUE:
    IN   R18, PINB ;Leer valor del PINB
    ANDI R18, 0x0F ;Mantener solo nibble bajo

    CPI  R18, 0x00
    BRBS 1, DISP_0
    CPI  R18, 0x01
    BRBS 1, DISP_1
    CPI  R18, 0x02
    BRBS 1, DISP_2
    CPI  R18, 0x03
    BRBS 1, DISP_3
    CPI  R18, 0x04
    BRBS 1, DISP_4
    CPI  R18, 0x05
    BRBS 1, DISP_5
    CPI  R18, 0x06
    BRBS 1, DISP_6
    CPI  R18, 0x07
    BRBS 1, DISP_7
    CPI  R18, 0x08
    BRBS 1, DISP_8
    CPI  R18, 0x09
    BRBS 1, DISP_9
    CPI  R18, 0x0A
    BRBS 1, DISP_A
    CPI  R18, 0x0B
    BRBS 1, DISP_b
    CPI  R18, 0x0C
    BRBS 1, DISP_C
    CPI  R18, 0x0D
    BRBS 1, DISP_d
    CPI  R18, 0x0E
    BRBS 1, DISP_E
    CPI  R18, 0x0F
    BRBS 1, DISP_F
    JMP MAIN_LOOP

; --------------------------------------
; Tabla de patrones (0–F)
; Asumiendo un display de cátodo común (1 = encendido)
; ---------------------------------------

DISP_0: 
    LDI R16, 0x3F
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_1:
    LDI R16, 0x06
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_2:
    LDI R16, 0x5B
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_3:
    LDI R16, 0x4F
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_4:
    LDI R16, 0x66
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_5:
    LDI R16, 0x6D
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_6:
    LDI R16, 0x7D
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_7:
    LDI R16, 0x07
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_8:
    LDI R16, 0x7F
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_9:
    LDI R16, 0x6F
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_A:
    LDI R16, 0x77
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_b:
    LDI R16, 0x7C
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_C:
    LDI R16, 0x39
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_d:
    LDI R16, 0x5E
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_E:
    LDI R16, 0x79
    OUT PORTD, R16
    JMP MAIN_LOOP

DISP_F:
    LDI R16, 0x71
    OUT PORTD, R16
    JMP MAIN_LOOP