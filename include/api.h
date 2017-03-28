#ifndef API_H
#define API_H

#include "common.h"

/*****************************************************************************
*
* NOTE: Write these routines inside src/api.c for your application.
*
*****************************************************************************/

// function to initialize node_state of each node in graph
void initialize_graph(
	Graph *g // input graph to initialize
);

// provide update function for local node state for each node in graph
node_state_t update(
	node_state_t val, // current node_state value of node (which will get updated)
#ifdef EDGE_WEIGHTS
	edge_state_t edge_weight,
#endif
	node_state_t src_node_state // node_state value coming from fanin (i.e. src_node)
);

node_state_t send(
	node_state_t val // current node_state value of node
);

#endif