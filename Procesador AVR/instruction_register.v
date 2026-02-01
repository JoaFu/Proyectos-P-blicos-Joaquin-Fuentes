module instr_register (
    input clock,
    input reset,
    input enable,
    input [15:0] A,
    output reg [15:0] Q
);
    reg delay;

    always @(posedge clock or posedge reset)
        if (reset) begin
            Q <= 16'b0;
            delay <= 0;
        end else if (enable | delay) begin
            Q <= A;
            delay <= 0;
        end else if (~enable) 
            delay <= 1;
endmodule