#ifndef E_TYPES_H
#define E_TYPES_H

#define e_addr_t unsigned // data type of epiphany global memory address
#define node_state_t float // data type of local node state (recommended: FP-single)
#define edge_state_t float // data type of edge weight attached to each edge in graph
#define bool_t uint8_t // data type used for boolean variables
#define e_status_t unsigned

// Graph data structure
typedef struct {
	unsigned *a_index_fanin;
	unsigned *col_index_fanin;
	unsigned *a_index_fanout;
	unsigned *col_index_fanout;
	unsigned *fanout_inedge_num;
	unsigned M;
	unsigned N;
	node_state_t *node_state;
	edge_state_t *edge_state;
} Graph;

// Graph data structure for the Epiphany
typedef struct {
	uint16_t **inedge_offset, **outedge_offset; 
#ifdef ADDR_GEN
	e_addr_t **outedge_state; // raw 32b global address
#else
	uint16_t **outedge_state; // 16b packed data structure
#endif
	unsigned **node_id_shard;
	unsigned *node_c, *inedge_c, *outedge_c;
	node_state_t **node_state;
	edge_state_t **edge_state;
} eGraph;

#endif