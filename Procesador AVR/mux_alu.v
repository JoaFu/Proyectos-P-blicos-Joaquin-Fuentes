module MUX_ALU (
    input [7:0] _0,
    input [7:0] _1,
    input S,
    output [7:0] Y
);

    assign Y = S ? _1 : _0;
endmodule