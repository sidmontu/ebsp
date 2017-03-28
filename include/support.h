#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef ENABLE_TIMING
#include "papi.h" // for timing measurements
#endif

// #include "mmio.h" // for matrix-market routines..

#ifdef ENABLE_TIMING 
// Get virtual clock time (usec)
long_long get_time();
#endif

// routine to read Graph structure from the .graph file format
void read_graph_file(
	char *graph_file, // path to input .graph file
	Graph *g, // graph data structure to be populated
	bool_t has_edge_weights // flag to indicate if graph has edge weights
);

// creates a .dot graph file (useful for visualizing a BSP graph using the .dot open-source format)
void create_dot_graph(
	Graph *g // input graph
);

// partitions an input Graph into an eGraph data structure
void partition_graph(
	unsigned num_partitions, // number of partitions to make
	Graph *g, // input complete graph to partition
	eGraph *eg, // data structure to store the partitioned input graph
	bool_t has_edge_weights // flag to indicate if graph has edge weights
);

// pack a node's fanout into 2 bytes
uint16_t fanout_pack(
	unsigned target_pe, // target_ecore_id
	unsigned target_node // local address of target node in target_ecoreid
);

// compute raw 32b address of a node's fanout
e_addr_t get_global_address(
	unsigned target_pe, // target_ecore_id
	unsigned target_node // local address of target node in target_ecoreid
);

// load graph data structure onto an eCore
void e_load_graph(
	unsigned x, // eCore's x-coordinate
	unsigned y, // eCore's y-coordinate
	unsigned coreid, // eCore's coreid (precomputed)
	e_epiphany_t *dev, // eCore handle
	unsigned bsp_iterations, // number of bsp iterations to run on eCore
	eGraph *eg // graph data structure to load
);

// loads the contents of each eCore's local memory, and sets app status to an init state
void e_init_memory(
	e_platform_t *platform, // platform handle
	e_epiphany_t *dev, // epiphany device handle
	eGraph *eg, // graph partitions data structure for epiphany eCores
	e_status_t *e_status, // eCore runtime status flag
	unsigned bsp_iterations // number of bsp iterations to run on each eCore
);

#endif