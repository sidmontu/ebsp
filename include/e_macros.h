#ifndef E_MACROS_H
#define E_MACROS_H

#define SEND_FANOUTS_UNOPT() 	for (j = start; j < end; j++) { \
									uint16_t fanout = outedge_state[j]; \
									uint16_t row = (fanout & 0xc000) >> 14; \
									uint16_t col = (fanout & 0x3000) >> 12; \
									uint16_t target_edge = fanout & 0x0fff; \
									node_state_t *n; \
									n = (node_state_t *)E_INEDGE_STATE_ADDR; \
									n += target_edge; \
									unsigned *target = (unsigned *)e_get_global_address(row,col,n); \
									*target = sendValue; \
								}

#define SEND_FANOUTS_ADDRGEN()	for (j = start; j < end; j++) { \
									unsigned *target = (unsigned *)(outedge_state[j]); \
									*target = sendValue; \
								}

#define SEND_FANOUTS_SQUELCH() 	if (!squelch_node[i]) { \
									SEND_FANOUTS_UNOPT() \
								}

#define SEND_FANOUTS_ADDRGEN_SQUELCH()	if (!squelch_node[i]) { \
											SEND_FANOUTS_ADDRGEN() \
										}

#define SEND_FANOUTS_UNROLL(u) 3
#define SEND_FANOUTS_UNROLL_ADDRGEN(u) 2
#define SEND_FANOUTS_UNROLL_ADDRGEN_SQUELCH(u) 4

#endif
