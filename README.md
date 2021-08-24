# HLS\_Wrapper
## Compiling
Requires C++17 support (for fold expressions)

Run:
```
g++ main.cpp -std=c++17 -o hls_wrapper
```
in the `src` forlder.

## Usage
The program is currenly built to run from the `src` folder. It will load the contents of `inputs/parser1.out`, process them, and output the results to `outputs/wrapper1.sv`. An example input and output is given. If the input file is correctly placed, simply run:
```
./hls_wrapper
```
from the `src` folder after compiling.

Edit the `template.sv` file in the `template` folder to modify the name of the wrapper module as well as to specify the correct name of the wrapped top level file, i.e. `wrapper_tf` and `coord_delay_tl` in the given file.

## TODO
- Implement passing filepaths for the input and output files as arguments
- Implement better error handling
- Improve naming for vairables
- Reorganize functions into apprioriate namespaces
