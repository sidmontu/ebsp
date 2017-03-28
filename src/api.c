#include "api.h"

// provide function to initialize node_state of each node in graph
void initialize_graph(Graph *g) {
	
	// allocate memory for node_state
	g->node_state = malloc(sizeof g->node_state * g->M);

	// write code here..
	int i;
	for (i = 0; i < g->M; i++) {
		g->node_state[i] = 0.15f;
	}
}

// provide update function for local node state for each node in graph
node_state_t update(
	node_state_t val,
#ifdef EDGE_WEIGHTS
	edge_state_t edge_weight,
#endif
	node_state_t src_node_state
) {
	
	// write code here..
	
	return (val+src_node_state*edge_weight);
}

node_state_t send(node_state_t val) {
	
	// write code here..

	return val;
}