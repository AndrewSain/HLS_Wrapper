# HLS\_Wrapper
## WARNING:
This program is incomplete, but still useful. Currently, it will not create 100% of the wrapper. It does however:
- Create the connections from the top-level verilog file to its unrolled registers/wires (barring "ap\_clk" & "ap\_rst"
- Instantiate said registers and wires
- Create the shift in/out logic
- Create the reset logic for the wrapper registers

## Compiling
Requires C++17 support (for fold expressions)

Run:
```
g++ main.cpp -std=c++17 -o hls_wrapper
```
in the `src` forlder.

## Usage
The program is currenly built to run from the `src` folder. It will load the contents of `inputs/parser1.out`, process them, and output the results to `outputs/proto_wrapper_new.sv`. An example input and output is given. If the input file is correctly placed, simply run:
```
./hls_wrapper
```

## TODO:
- Implement passing filepaths for the input and output files as arguments
- Implement the creation of the IO to and from the wrapper (bus, control, clock, reset)
- Implement total wrapper creation (perhaps using some kind of base template)
- Implement better error handling
