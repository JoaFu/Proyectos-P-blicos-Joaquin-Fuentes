module pc (
    input clock,
    input reset,
    input load,
    input [6:0] loadValue,
    output reg [6:0] count
);

reg delay; 

    always @(posedge clock or posedge reset) begin
        if (reset) begin
            count <= 7'b0;
            delay <= 0;
        end else if (delay) begin
            count <= count + 1;
            delay <= 0;
        end else if (load) begin
            count <= loadValue;
            delay <= 1;
        end else count <= count + 1;
    end
endmodule

