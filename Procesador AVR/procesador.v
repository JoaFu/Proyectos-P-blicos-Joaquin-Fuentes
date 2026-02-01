module procesador (
    input clock,
    input reset,
    input [7:0] PIND,
    input [7:0] PINC,
    input [7:0] PINB,
    output [7:0] PORTD,
    output [7:0] PORTC,
    output [7:0] PORTB
);

    // ========== WIRES DE INTERCONEXIÓN ==========
    
    // Program Counter
    wire [6:0] wire_PC_count;           // Dirección actual del PC
    wire [6:0] wire_newPC;              // Nueva dirección calculada por PC_ALU
    wire wire_loadPC;                   // Señal para cargar nuevo valor en PC
    
    // Flash Memory (Memoria de programa)
    wire [15:0] wire_instruction;       // Instrucción de 16 bits desde Flash
    
    // Instruction Register
    wire [15:0] wire_instrReg;          // Instrucción almacenada en el registro
    wire wire_instrEN;                  // Enable para cargar nueva instrucción
    
    // Decoder
    wire wire_S;                        // Selector MUX: 0=Constante K, 1=Registro Rr
    wire [2:0] wire_F;                  // Función de la ALU
    wire [7:0] wire_address1;           // Dirección 1 de RAM (lectura/escritura)
    wire [7:0] wire_address2;           // Dirección 2 de RAM (solo lectura)
    wire wire_ramWE;                    // Write Enable de RAM
    wire wire_PCinstr;                  // Tipo de salto: 0=Relativo(BRBS), 1=Absoluto(JMP)
    wire [6:0] wire_k;                  // Constante de 7 bits para saltos (PC_ALU)
    wire [7:0] wire_K;                  // Constante de 8 bits para datos (ALU)
    
    // RAM
    wire [7:0] wire_Rd;                 // Dato leído desde address1 (primer operando ALU)
    wire [7:0] wire_RrK_from_RAM;       // Dato leído desde address2 (segundo operando)
    wire wire_Z_decode;                 // Flag Zero desde SREG
    wire wire_C_decode;                 // Flag Carry desde SREG
    
    // MUX para seleccionar entre registro Rr o constante K
    wire [7:0] wire_RrK_to_ALU;         // Entrada RrK hacia la ALU (post-MUX)
    
    // ALU
    wire [7:0] wire_ALU_result;         // Resultado de la operación de la ALU
    wire wire_Z_flag;                   // Flag Zero generado por ALU
    wire wire_C_flag;                   // Flag Carry generado por ALU
    
    
    // ========== INSTANCIACIÓN DE MÓDULOS ==========
    
    // Program Counter: Mantiene la dirección de la instrucción actual
    pc program_counter (
        .clock(clock),
        .reset(reset),
        .load(wire_loadPC),                 // Cargar nuevo valor cuando hay salto
        .loadValue(wire_newPC),             // Nuevo valor del PC desde PC_ALU
        .count(wire_PC_count)               // Dirección actual
    );
    
    // PC ALU: Calcula la nueva dirección del PC para saltos
    pc_alu pc_arithmetic (
        .k(wire_k),                         // Constante de desplazamiento (7 bits)
        .PC_count(wire_PC_count),           // PC actual
        .instruction(wire_PCinstr),         // 0=BRBS(relativo), 1=JMP(absoluto)
        .newPC(wire_newPC)                  // Nueva dirección calculada
    );
    
    // Flash Memory: Almacena el programa (instrucciones)
    flash_memory program_memory (
        .address(wire_PC_count),            // Dirección desde PC
        .data(wire_instruction)             // Instrucción de 16 bits
    );
    
    // Instruction Register: Almacena la instrucción actual
    instr_register instruction_reg (
        .clock(clock),
        .reset(reset),
        .enable(wire_instrEN),              // Enable desde decoder
        .A(wire_instruction),               // Instrucción desde Flash
        .Q(wire_instrReg)                   // Instrucción almacenada
    );
    
    // Decoder: Decodifica la instrucción y genera señales de control
    decoder instruction_decoder (
        .instrReg(wire_instrReg),           // Instrucción a decodificar
        .Z_decode(wire_Z_decode),           // Flag Z desde RAM (SREG)
        .C_decode(wire_C_decode),           // Flag C desde RAM (SREG)
        .S(wire_S),                         // Selector MUX (Rr vs K)
        .F(wire_F),                         // Función de la ALU
        .address1(wire_address1),           // Dirección RAM 1
        .address2(wire_address2),           // Dirección RAM 2
        .ramWE(wire_ramWE),                 // Write Enable RAM
        .loadPC(wire_loadPC),               // Load PC para saltos
        .instrEN(wire_instrEN),             // Enable Instruction Register
        .PCinstr(wire_PCinstr),             // Tipo de salto
        .k(wire_k),                         // Constante para PC_ALU
        .K(wire_K)                          // Constante para ALU
    );
    
    // RAM: Almacena registros, I/O y datos
    RAM data_memory (
        .clock(clock),
        .address1(wire_address1),           // Dirección para lectura/escritura
        .address2(wire_address2),           // Dirección solo para lectura
        .regWrite(wire_ALU_result),         // Dato a escribir (desde ALU)
        .WE(wire_ramWE),                    // Write Enable
        .Z(wire_Z_flag),                    // Flag Z desde ALU
        .C(wire_C_flag),                    // Flag C desde ALU
        .PIND(PIND),                        // Puerto de entrada D
        .PINC(PINC),                        // Puerto de entrada C
        .PINB(PINB),                        // Puerto de entrada B
        .data1(wire_Rd),                    // Dato leído (operando 1)
        .data2(wire_RrK_from_RAM),          // Dato leído (operando 2)
        .PORTD(PORTD),                      // Puerto de salida D
        .PORTC(PORTC),                      // Puerto de salida C
        .PORTB(PORTB),                      // Puerto de salida B
        .Z_decode(wire_Z_decode),           // Flag Z desde SREG
        .C_decode(wire_C_decode)            // Flag C desde SREG
    );

    MUX_ALU mux_alu(
        .S(wire_S),          // Selector: 0=K, 1=Rr
        ._0(wire_K),     // Constante inmediata K
        ._1(wire_RrK_from_RAM),// Valor desde RAM (Rr)
        .Y(wire_RrK_to_ALU)    // Salida combinada hacia ALU
    );
    
    // ALU: Realiza operaciones aritméticas y lógicas
    ALU arithmetic_logic_unit (
        .Rd(wire_Rd),                       // Primer operando (desde RAM)
        .RrK(wire_RrK_to_ALU),              // Segundo operando (Rr o K)
        .F(wire_F),                         // Función a ejecutar
        .Y(wire_ALU_result),                // Resultado
        .Z(wire_Z_flag),                    // Flag Zero
        .C(wire_C_flag)                     // Flag Carry
    );

endmodule