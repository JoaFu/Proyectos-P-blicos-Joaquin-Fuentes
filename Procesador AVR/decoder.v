module decoder (
    input [15:0] instrReg,
    input Z_decode,
    input C_decode,
    output S,
    output [2:0] F,
    output [7:0] address1,
    output [7:0] address2,
    output ramWE,
    output loadPC,
    output instrEN,
    output PCinstr,
    output [6:0] k,
    output [7:0] K // Esta es la salida de la constante K. Esta salida sustituye al bitswizzling en la interconexión de módulos. Está conectado con el 'wire_K' desde el decoder hacia el MUX de la ALU
);
 
    reg _S, _ramWE, _loadPC, _PCinstr, _instrEN;
    reg [2:0] _F;
    reg [6:0] _k;
    reg [7:0] _K, _address1, _address2;
    wire [3:0] instruction;
 
    assign instruction = instrReg[15:12];
 
    initial begin
        _S = 0;
        _ramWE = 0;
        _loadPC = 0;
        _PCinstr = 0;
        _instrEN = 1;
        _F = 0;
        _k = 0;
        _address1 = 0;
        _address2 = 0;
    end
 
    parameter _pasaRrk = 3'b000;
    parameter _suma = 3'b001;
    parameter _and = 3'b010;
    parameter _resta = 3'b011;
    parameter _xor = 3'b100;
    parameter _pasaRd = 3'b101;
    parameter _or = 3'b110;
 
    always @(*) begin
        casez (instrReg)
            // ADD
            16'b0000_00??_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;  // Señal que determina si se hace o no 'load' en el PC
                _PCinstr = 0; // Señal que modifica el PC_ALU para decidir entre saltos absolutos (JMP) y saltos relativos (BRBS)
                _instrEN = 1;  // Señal para habilitar el Instruction Register
                _F = _suma;  // Señal de la función de la ALU
                _k = 0;  // Constante de dirección para saltos del PC
                _K = {
                    instrReg[11:8], instrReg[3:0]
                };  // Constante de datos para instrucciones LDI, ANDI, CPI
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {3'b000, instrReg[9], instrReg[3:0]};
            end
            // AND
            16'b0001_00??_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _and;
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {3'b000, instrReg[9], instrReg[3:0]};
            end
            // SUB
            16'b0010_00??_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b011;  // ALU -> SUB
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {3'b000, instrReg[9], instrReg[3:0]};
            end
            // CLR
            16'b0011_00??_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b100;  // ALU -> XOR
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {3'b000, instrReg[9], instrReg[3:0]};
            end
            // ANDI
            16'b0100_????_????_????: begin
                _S = 0;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b010;  // AND
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {4'b0001, instrReg[7:4]};
                _address2 = 0;
            end
            // BRBS
            16'b0101_00??_????_????: begin
                case (instrReg[2:0])
                    3'b000:
                    if (C_decode == 1) begin
                        _S = 0;
                        _ramWE = 0;
                        _loadPC = 1;
                        _PCinstr = 0;
                        _instrEN = 0;
                        _F = 0;
                        _k = instrReg[9:3];
                        _K = {instrReg[11:8], instrReg[3:0]};
                        _address1 = 0;
                        _address2 = 0;
                    end
                    3'b001:
                    if (Z_decode == 1) begin
                        _S = 0;
                        _ramWE = 0;
                        _loadPC = 1;
                        _PCinstr = 0;
                        _instrEN = 0;
                        _F = 0;
                        _k = instrReg[9:3];
                        _K = {instrReg[11:8], instrReg[3:0]};
                        _address1 = 0;
                        _address2 = 0;
                    end
                    default: begin
                        _S = 0;
                        _ramWE = 0;
                        _loadPC = 0;
                        _PCinstr = 0;
                        _instrEN = 0;
                        _F = 0;
                        _k = 0;
                        _address1 = 0;
                        _address2 = 0;
                    end
                endcase
            end
            // CPI
            16'b0110_????_????_????: begin
                _S = 0;  // Dejar pasar K
                _ramWE = 0;  // Deshabilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b011;  // Resta
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {4'b0001, instrReg[7:4]};
                _address2 = 0;
            end
            // JMP
            16'b0111_0000_0???_????: begin
                _S = 0;
                _ramWE = 0;
                _loadPC = 1;
                _PCinstr = 1;
                _instrEN = 0;
                _F = 0;
                _k = instrReg[6:0];
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = 0;
                _address2 = 0;
            end
            // IN
            16'b1000_0???_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _pasaRrk;
                _k = instrReg[6:0];
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {2'b00, instrReg[10:9], instrReg[3:0]} + 8'h20;
            end
            // LDI
            16'b1001_????_????_????: begin
                _S = 0;  // Dejar pasar K
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b000;  // Dejar pasar RrK
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {4'b0001, instrReg[7:4]};
                _address2 = 0;
            end
            // LDS (probar)
            16'b1010_????_????_????: begin
                _S = 1;  // Dejar pasar Rr
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _pasaRrk;  // Dejar pasar RrK
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {4'b0001, instrReg[7:4]};
                _address2 = {instrReg[11:8], instrReg[3:0]};
            end
            // MOV
            16'b1011_00??_????_????: begin
                _S = 1;  // Dejar pasar RrK
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b000;  // Dejar pasar RrK
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {3'b000, instrReg[8:4]};
                _address2 = {3'b000, instrReg[9], instrReg[3:0]};
            end
            // OUT
            16'b1100_0???_????_????: begin
                _S = 1;  // Dejar pasar Rr
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = 3'b000;  // Dejar pasar RrK
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {2'b00, instrReg[10:9], instrReg[3:0]} + 8'h20;
                _address2 = {3'b000, instrReg[8:4]};
            end
            // STS (probar)
            16'b1101_????_????_????: begin
                _S = 1;  // Dejar pasar Rr
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _pasaRrk;  // Dejar pasar RrK
                _k = 0;
                _K = {instrReg[11:8], instrReg[3:0]};
                _address1 = {instrReg[11:8], instrReg[3:0]};
                _address2 = {4'b0001, instrReg[7:4]};
            end
            // CBI (pendiente)
            16'b1110_????_????_????: begin
                _S = 0;  // Dejar pasar K
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _and;
                _k = 0;
                _address1 = {3'b000, instrReg[7:3]} + 8'h20;
                _address2 = 0;
                case (instrReg[2:0])
                    3'b000:  _K = 8'b1111_1110;
                    3'b001:  _K = 8'b1111_1101;
                    3'b010:  _K = 8'b1111_1011;
                    3'b011:  _K = 8'b1111_0111;
                    3'b100:  _K = 8'b1110_1111;
                    3'b101:  _K = 8'b1101_1111;
                    3'b110:  _K = 8'b1011_1111;
                    3'b111:  _K = 8'b0111_1111;
                    default: _K = 8'b0000_0000;
                endcase
            end
            // SBI (pendiente)
            16'b1111_????_????_????: begin
                _S = 0;  // Dejar pasar K
                _ramWE = 1;  // Habilitar escritura en RAM
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 1;
                _F = _or;
                _k = 0;
                _address1 = {3'b000, instrReg[7:3] + 8'h20};
                _address2 = 0;
                case (instrReg[2:0])
                    3'b000:  _K = 8'b0000_0001;
                    3'b001:  _K = 8'b0000_0010;
                    3'b010:  _K = 8'b0000_0100;
                    3'b011:  _K = 8'b0000_1000;
                    3'b100:  _K = 8'b0001_0000;
                    3'b101:  _K = 8'b0010_0000;
                    3'b110:  _K = 8'b0100_0000;
                    3'b111:  _K = 8'b1000_0000;
                    default: _K = 8'b0000_0000;
                endcase
            end
            default: begin
                _S = 0;
                _ramWE = 0;
                _loadPC = 0;
                _PCinstr = 0;
                _instrEN = 0;
                _F = 0;
                _k = 0;
                _address1 = 0;
                _address2 = 0;
            end
 
        endcase
    end
 
    assign S = _S;
    assign PCinstr = _PCinstr;
    assign ramWE = _ramWE;
    assign loadPC = _loadPC;
    assign F = _F;
    assign address1 = _address1;
    assign address2 = _address2;
    assign k = _k;
    assign K = _K;
    assign instrEN = _instrEN;
endmodule