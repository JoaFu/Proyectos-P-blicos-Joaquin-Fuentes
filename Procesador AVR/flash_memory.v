module flash_memory (
    input [6:0] address,
    output [15:0] data
);
    reg [15:0]mem [128];

    assign data = mem[address];

    initial $readmemh ("instrucciones.hex", mem);

endmodule