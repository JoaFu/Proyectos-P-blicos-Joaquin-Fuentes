`timescale 1ns/1ps

module procesador_tb;
    reg clock;
    reg reset;
    reg [7:0] PIND;
    reg [7:0] PINC;
    reg [7:0] PINB;
    wire [7:0] PORTD;
    wire [7:0] PORTC;
    wire [7:0] PORTB;
    
    // Instancia del procesador completo
    procesador uut (
        .clock(clock),
        .reset(reset),
        .PIND(PIND),
        .PINC(PINC),
        .PINB(PINB),
        .PORTD(PORTD),
        .PORTC(PORTC),
        .PORTB(PORTB)
    );
// Generación de reloj
initial begin
    clock = 1;
    forever #10 clock = ~clock;   // Período de 20 ns (50 MHz)
end

initial begin
    $dumpfile("procesador_tb.vcd");
    $dumpvars(0, procesador_tb);

    // Señales internas
    $dumpvars(0, uut.wire_PC_count);
    $dumpvars(0, uut.wire_instruction);
    $dumpvars(0, uut.wire_instrReg);
    $dumpvars(0, uut.wire_ALU_result);
    $dumpvars(0, uut.wire_Rd);
    $dumpvars(0, uut.wire_RrK_to_ALU);
    $dumpvars(0, uut.wire_F);
    $dumpvars(0, uut.wire_S);
    $dumpvars(0, uut.wire_ramWE);
    $dumpvars(0, uut.wire_loadPC);
    $dumpvars(0, uut.wire_Z_flag);
    $dumpvars(0, uut.wire_C_flag);
    $dumpvars(0, uut.wire_Z_decode);
    $dumpvars(0, uut.wire_C_decode);
    $dumpvars(0, uut.wire_address1);
    $dumpvars(0, uut.wire_address2);

    // Memoria (RAM)
    $dumpvars(0, uut.data_memory.mem[16]);
    $dumpvars(0, uut.data_memory.mem[17]);
    $dumpvars(0, uut.data_memory.mem[18]);
    $dumpvars(0, uut.data_memory.mem[95]);  // SREG (0x5F)

    // ------------------------------------------------
    // RESET INICIAL
    // ------------------------------------------------
    reset = 0;
    PIND = 8'h00;
    PINC = 8'h00;
    PINB = 8'h00;

    #5;
    reset = 1;
    #40;
    reset = 0;
    #60;

    // ------------------------------------------------
    // SIMULACIÓN DE ENTRADAS
    // ------------------------------------------------

   // Probar todos los valores del DIP (0x0 – 0xF) correspondientes a DISP_0 a DISP_F

    // Botón no presionado para permitir mostrar valores
    PINC = 8'b0000_0001;
    #100;

    // Recorrer cada valor hexadecimal (0x0 a 0xF)
    PINB = 8'h00;  // → 0x3F = “0”
    #1000;
    PINB = 8'h01;  // → 0x06 = “1”
    #1000;
    PINB = 8'h02;  // → 0x5B = “2”
    #1000;
    PINB = 8'h03;  // → 0x4F = “3”
    #1000;
    PINB = 8'h04;  // → 0x66 = “4”
    #1000;
    PINB = 8'h05;  // → 0x6D = “5”
    #1000;
    PINB = 8'h06;  // → 0x7D = “6”
    #1000;
    PINB = 8'h07;  // → 0x07 = “7”
    #1000;
    PINB = 8'h08;  // → 0x7F = “8”
    #1000;
    PINB = 8'h09;  // → 0x6F = “9”
    #1000;
    PINB = 8'h0A;  // → 0x77 = “A”
    #1000;
    PINB = 8'h0B;  // → 0x7C = “b”
    #1000;
    PINB = 8'h0C;  // → 0x39 = “C”
    #1000;
    PINB = 8'h0D;  // → 0x5E = “d”
    #2000;
    PINC = 8'b0000_0000;  // presionado → display apagado (PORTD = 0x00)
    #2000;
    PINC = 8'b0000_0001;  // soltar → vuelve a mostrar el último valor (F)
    #2000;
    PINB = 8'h0E;  // → 0x79 = “E”
    #1000;
    PINB = 8'h0F;  // → 0x71 = “F”
    #1000;


    // --- Fin de simulación ---
    $finish;
end

    
    // Monitor para seguimiento en consola (opcional, no interfiere con GTKwave)
    always @(posedge clock) begin
        $display("Time=%0t | PC=%h | Instr=%h | R16=%h | R17=%h | R18=%h | Z=%b | C=%b", 
                 $time, 
                 uut.wire_PC_count, 
                 uut.wire_instrReg,
                 uut.data_memory.mem[16],
                 uut.data_memory.mem[17],
                 uut.data_memory.mem[18],
                 uut.wire_Z_decode,
                 uut.wire_C_decode);
    end

endmodule