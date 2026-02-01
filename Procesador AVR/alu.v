module ALU (
    input [7:0] Rd,
    input [7:0] RrK,
    input [2:0] F,
    output reg [7:0] Y,
    output reg Z,
    output reg C
);

    // Parámetros para las operaciones
    parameter _passRrk = 3'b000;
    parameter _suma    = 3'b001;
    parameter _and     = 3'b010;
    parameter _resta   = 3'b011;
    parameter _xor     = 3'b100;
    parameter _passRd  = 3'b101;
    parameter _or      = 3'b110;

    // Registro temporal para operaciones con carry
    reg [8:0] temp_result;

    always @(*) begin
        case (F)
            _passRrk: begin
                temp_result = {1'b0, RrK};  // Pasar RrK
                Y = RrK;
            end
            
            _suma: begin
                temp_result = Rd + RrK;     // Suma con carry
                Y = temp_result[7:0];
            end
            
            _and: begin
                temp_result = {1'b0, Rd & RrK};  // AND bit a bit
                Y = Rd & RrK;
            end
            
            _resta: begin
                temp_result = Rd - RrK;     // Resta con carry
                Y = temp_result[7:0];
            end
            
            _xor: begin
                temp_result = {1'b0, Rd ^ RrK};  // XOR bit a bit (corregido)
                Y = Rd ^ RrK;
            end
            
            _passRd: begin
                temp_result = {1'b0, Rd};   // Pasar Rd
                Y = Rd;
            end
            
            _or: begin
                temp_result = {1'b0, Rd | RrK};  // OR bit a bit
                Y = Rd | RrK;
            end
            
            default: begin
                temp_result = 9'b0;
                Y = 8'b0;
            end
        endcase
        
        // Calcular bandera Zero (resultado = 0)
        Z = (Y == 8'b0);
        
        // Calcular bandera Carry (bit más significativo del resultado extendido)
        C = temp_result[8];
    end

endmodule