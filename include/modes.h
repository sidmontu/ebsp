#ifndef MODES_H
#define MODES_H

#define ENABLE_TIMING // uncomment/comment to enable/disable PAPI timing measurement
#define UNROLL_SEND 2 // enable unroll of send routines, with specified unroll factor
#define UNROLL_COMPUTE 2 // enable unroll of local compute, with specified unroll factor
#define ADDR_GEN// uncomment/comment to enable/disable address precompute optimization (memory tradeoff)
#define SQUELCH_SEND 0.1 // squelch send messages, specified with a threshold factor (floating-point value)
#define VERBOSE // produces some extra outputs (debug-related, or visualizations)
#define EDGE_WEIGHTS 0 // allocates memory for storing edge weights if enabled

#endif