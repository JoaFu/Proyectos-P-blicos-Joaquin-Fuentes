module RAM (
    input clock,
    input [7:0] address1,
    input [7:0] address2,
    input [7:0] regWrite,
    input WE,
    input Z,
    input C,
    input [7:0] PIND,
    input [7:0] PINC,
    input [7:0] PINB,
    output [7:0] data1,
    output [7:0] data2,
    output [7:0] PORTD,
    output [7:0] PORTC,
    output [7:0] PORTB,
    output Z_decode,
    output C_decode
);
    // RAM completa (Incluye registros, I/O, espacio genérico)
    reg [7:0] mem[0:255];
 
    /*
     Las instrucciones IN, OUT, SBI, CBI se escriben utilizando direcciones
     bajas, PERO realmente se mappean a direcciones más altas.
 
     Es decir, cuando utilizamos la instrucción IN tiene este opcode
 
     1000_0AAd_dddd_AAAA
 
     Las letras A hacen referencia a localidades desde la 0 hasta la 64, pero
     realmente tenemos que sumarle 0x20 a la localidad indicada. Esto lo hacemos
     mejor en el decoder
    */
 
    // Read from RAM (ANY address)
    assign data1 = mem[address1];
    assign data2 = mem[address2];
 
    // Writing to RAM (ANY address)
    always @(posedge clock) begin
        if (WE) mem[address1] <= regWrite;
 
        mem[8'h5F] <= {6'b0, Z, C};  // Para actualizar los valores del SREG
        mem[8'h29] <= PIND;
        mem[8'h26] <= PINC;
        mem[8'h23] <= PINB;
    end
 
    /*
     En el ATmega328p realmente sólo hay 3 sets de pines y estos se pueden
     configurar como una ENTRADA o como una SALIDA. En nuestro caso vamos a
     implementar 6 sets de pines en total: 3 sets de entrada y 3 sets de salida.
 
     Esto es para simplificar el funcionamiento.
 */
 
    // Output ports
    assign PORTD = mem[8'h2B];
    assign PORTC = mem[8'h28];
    assign PORTB = mem[8'h25];
 
    // Input ports
    always @(PIND, PINC, PINB) begin
        mem[8'h29] <= PIND;
        mem[8'h26] <= PINC;
        mem[8'h23] <= PINB;
    end
 
    // SREG Output
    assign Z_decode = mem[8'h5F][1];
    assign C_decode = mem[8'h5F][0];
endmodule
 