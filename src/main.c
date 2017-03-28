#include <e-hal.h> // epiphany sdk

#include "modes.h" // modify this to enable/disable timing+optimization passes
#include "common.h" // common libraries/datatypes
#include "api.h" // API functions for the BSP abstraction
#include "support.h" // support functions

int main(int argc, char** argv) {

	// safety check to ensure all arguments are provided
	if (argc < 3) {
		printf("\n-------------------ERROR----------------------\n");
		printf("Usage: make run <num_bsp_iterations> <input_graph_file>\n");
		printf("----------------------------------------------\n\n");
		exit(1);
	} else if (atoi(argv[1]) <= 0) { // sanity check
		printf("Number of BSP iterations must be a positive integer!");
		exit(1);
	}

	int i,j,k; // local variables (for iterators, etc)

	// Read and store the arguments from the program
	unsigned bsp_iterations = atoi(argv[1]);
	char *graph_file = argv[2];

#ifdef ENABLE_TIMING
	long_long t0, t1, papi_overhead; // timestamps
	// compute overhead of grabbing timestaps -- i.e. remove any systematic errors
	t0 = get_time();
	t1 = get_time();
	papi_overhead = (t1-t0);
#endif

	// read graph from input .graph file, and construct the relevant data structures
	Graph g; // BSP graph
	eGraph eg; // BSP graph partitions for the Epiphany eCores

	printf("Constructing graph from .graph file %s ...",graph_file);
	read_graph_file(graph_file,&g,EDGE_WEIGHTS); // construct graph from .graph file
	printf("Done. Nodes = %d, Edges = %d.\n",g.M,g.N);
#ifdef VERBOSE
	create_dot_graph(&g);
#endif
	initialize_graph(&g); // user-supplied function
	// partition the graph into NUM_ECORES partitions
	printf("Partitioning graph randomly into %d partitions...",NUM_ECORES);
	partition_graph(NUM_ECORES,&g,&eg,EDGE_WEIGHTS);
	printf("Done.\n");


	/*******************************************************************/
	/*********************** sequential solver *************************/
	/*******************************************************************/

#ifdef ENABLE_TIMING
	t0 = get_time();
#endif
	printf("[ARM] Running %d BSP iterations...\n",bsp_iterations);
	for (i = 0; i < bsp_iterations; i++) {
		for (j = 0; j < g.M; j++) {
			node_state_t node_state = 0.0f;
			for (k = g.a_index_fanin[j]; k < g.a_index_fanin[j+1]; k++) {
				node_state = update(node_state,g.node_state[g.col_index_fanin[k]]);
			}
		}
	}
#ifdef ENABLE_TIMING
	t1 = get_time();
	printf("[ARM] %d BSP iterations evaluated in %lld us.\n",bsp_iterations,(t1-t0-papi_overhead));
#endif

	/*******************************************************************/
	/************************ parallel solver **************************/
	/*******************************************************************/

	e_platform_t platform;
	e_epiphany_t dev;

	//initialize device
	e_init(NULL);
	e_reset_system();
	e_get_platform_info(&platform);

	// Open a workgroup
	e_open(&dev, 0, 0, platform.rows, platform.cols);

	// Initialize eCores' local memory
	e_status_t e_status  = E_CORE_INIT; // set app status of each eCore as init
	e_init_memory(&platform,&dev,&eg,&e_status,bsp_iterations);
	
	// Load srec
	e_load_group("bin/e_main.srec", &dev, 0, 0, platform.rows, platform.cols, E_FALSE);

#ifdef ENABLE_TIMING
	t0=get_time();
#endif

	// start execution on the Epiphany
	e_start_group(&dev);

	// poll eCore(0,0) for status flag
	while (e_status != E_CORE_COMPLETE){
		e_read(&dev,0,0,E_STATUS_ADDR,&e_status,sizeof(e_status_t));
	}

#ifdef ENABLE_TIMING
	t1=get_time();
	printf("[EPIPHANY] %d BSP iterations evaluated in %lld us.\n",bsp_iterations,(t1-t0-papi_overhead));
#endif

	// TODO: functional correctness check

	return 0;
}