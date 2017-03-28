# eBSP
### An open-source library for managing NoC-based communication for the Adapteva Epiphany-III Processor

### How to use:
Create your BSP graph. The format can be found in an example graph file in the examples folder. Once the input graph is created, add your send() and update() function that define your application in the "api.c" inside the src/ folder. Type make in the top folder, and the tools will compile your program, which you can run with `make run <arguments>`. `make run` currently takes in two inputs: num_bsp_iterations, and input_graph_file.

### Development status:
The library is being standardized further and is in active development. We will be releasing a completed stable version very soon.

### Paper
More details about this work can be found in the [DATE 2017 paper](https://sidmontu.github.io/pubs_pdfs/ebsp_date2017.pdf).