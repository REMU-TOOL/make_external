module top (
    input clk,
    input rst,
    input en,
    input i,
    output o
);

    foo u_foo (
        .clk(clk),
        .rst(rst),
        .en(en),
        .i(i),
        .o(o)
    );

endmodule

module foo (
    input clk,
    input rst,
    input en,
    input i,
    output o
);

    bar u_bar (
        .clk(clk),
        .rst(rst),
        .en(en),
        .i(i),
        .o(o)
    );

endmodule

module bar (
    input clk,
    input rst,
    input en,
    input i,
    output o
);

    (* make_external = "output" *)
    reg [31:0] data;

    always @(posedge clk) begin
        if (rst) data <= 32'd0;
        else if (en) data <= {data[31:1], i};
    end

    assign o = data[31];

endmodule
