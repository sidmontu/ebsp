#include <stdint.h>
#include <string.h>
#include "e_lib.h"
#include "common.h"
#include "api.h"
#include "e_macros.h"

// global variables
unsigned *num_nodes, *num_inedges, *num_outedges, *bsp_iterations;
node_state_t *node_state, *inedge_state;
uint16_t *inedge_offset, *outedge_offset;
#ifdef SQUELCH_SEND
uint8_t *squelch_node;
#endif
#ifdef EDGE_WEIGHTS
edge_state_t *edge_state;
#endif
#ifdef ADDR_GEN
unsigned *outedge_state;
#else
uint16_t *outedge_state;
#endif

// e_barrier handle
volatile e_barrier_t  barriers[NUM_ECORES];
	e_barrier_t *tgt_bars[NUM_ECORES];

void e_send();
void e_update();
void init_pointers();

int main(void) {

	// initialize pointer to eCore_status and set it to E_CORE_RUNNING
	e_status_t *e_status = (e_status_t *)E_STATUS_ADDR;
	*e_status = E_CORE_RUNNING;

	// initialize barriers
	e_barrier_init(barriers, tgt_bars);

	// initialize all other pointers
	init_pointers();

	int i;
	for (i = 0; i < *bsp_iterations; i++) {
		e_send();
		e_barrier(barriers, tgt_bars);
		e_update();
		e_barrier(barriers, tgt_bars);
	}

	// all eCores synchronize at the end
	e_barrier(barriers, tgt_bars);

	*e_status = E_CORE_COMPLETE;

	return 0;
}

void e_update() {

	int i, j;
	for (i = 0; i < *num_nodes; i++) {
		node_state_t new_node_state;
		for (j = inedge_offset[i]; j < inedge_offset[i+1]; j++)
			new_node_state = update(inedge_val[j],node_state[i]);
	}
}

void e_send() {

	int i, j;
	for (i = 0; i < *num_nodes; i++){
		uint16_t start = outedge_offset[i];
		uint16_t end = outedge_offset[i+1];

		node_state_t sendValue = send(node_state[i]);

#ifdef UNROLL_SEND
	#ifdef ADDR_GEN
		#ifdef SQUELCH_SEND
			SEND_FANOUTS_UNROLL_ADDRGEN_SQUELCH(UNROLL_SEND,SQUELCH_SEND);
		#else
			SEND_FANOUTS_UNROLL_ADDRGEN(UNROLL_SEND);
		#endif
	#else
		SEND_FANOUTS_UNROLL(UNROLL_SEND);
	#endif
#else
	#ifdef ADDR_GEN
		#ifdef SQUELCH_SEND
			SEND_FANOUTS_ADDRGEN_SQUELCH(SQUELCH_SEND);
		#else
			SEND_FANOUTS_ADDRGEN();
		#endif
	#else
		#ifdef SQUELCH_SEND
			SEND_FANOUTS_SQUELCH(SQUELCH_SEND);
		#else
			// baseline version -- no optimizations
			for (j = start; j < end; j++) {

				uint16_t fanout = outedge_state[j];
				uint16_t row = (fanout & 0xc000) >> 14;
				uint16_t col = (fanout & 0x3000) >> 12;
				uint16_t target_edge = fanout & 0x0fff;

				node_state_t *n;
				n = (node_state_t *)E_INEDGE_STATE_ADDR;
				n += target_edge;
				
				unsigned *target = (unsigned *)e_get_global_address(row,col,n);
				*target = sendValue;
			}
		#endif
#endif
	}
}

void init_pointers() {

	num_nodes = (unsigned *)E_NUM_NODES_ADDR;
	num_inedges = (unsigned *)E_NUM_INEDGES_ADDR;
	num_outedges = (unsigned *)E_NUM_OUTEDGES_ADDR;
	bsp_iterations = (unsigned *)E_TIMESTEPS_ADDR;
	node_state = (node_state_t *)E_NODE_STATE_ADDR;
#ifdef EDGE_WEIGHTS
	edge_state = (edge_state_t *)E_EDGE_STATE_ADDR;
#endif
	inedge_state = (node_state_t *)E_INEDGE_STATE_ADDR;
#ifdef ADDR_GEN
	outedge_state = (unsigned *)E_OUTEDGE_STATE_ADDR;
#else
	outedge_state = (uint16_t *)E_OUTEDGE_STATE_ADDR;
#endif
	inedge_offset = (uint16_t *)E_INEDGE_OFFSET_ADDR;
	outedge_offset = (uint16_t *)E_OUTEDGE_OFFSET_ADDR;
#ifdef SQUELCH_SEND
	squelch_node = (uint8_t *)E_SQUELCH_VECTOR_ADDR;
#endif

}