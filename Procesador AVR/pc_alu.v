module pc_alu (
    input [6:0] k,
    input [6:0] PC_count,
    input instruction,
    output [6:0] newPC
);
    reg [6:0] _newPC, neg_k;

    always @(k, PC_count, instruction) begin

        if (instruction) // JMP
            _newPC <= k;
        else begin //BRBS
         if (k[6]) begin // negative number
            neg_k = ~k + 1; // Complemento a 2
            _newPC = PC_count - neg_k;
         end else _newPC = PC_count + k;
         end

    end

    assign newPC = _newPC;

endmodule