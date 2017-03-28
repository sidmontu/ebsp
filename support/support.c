#include "common.h"
#include "support.h"

#ifdef ENABLE_TIMING
long_long get_time() {
	return PAPI_get_virt_usec();
}
#endif

// routine to read Graph structure from the .graph file format
void read_graph_file(char *graph_file, Graph *g, bool_t has_edge_weights) {

	FILE *f = fopen(graph_file,"r");
	int i,j; // local variables
	char *line = NULL; // line buffer
	size_t len = 0;
	ssize_t read;

	unsigned *fanin_count, *fanout_count, *froms, *tos;
	edge_state_t *edge_weights;
	int count = 0;
	int nodes, edges;
	//get nodes and edges count
	while ((read = getline(&line, &len, f)) != -1) {
		if (strstr(line, "#") != NULL){
			if (strstr(line, "Nodes")) {
				char c_nodes[8], c_edges[8];
				//# Nodes: 5242 Edges: 28980
				sscanf(line,"# Nodes: %s Edges: %s",c_nodes,c_edges);
				g->M = atoi(c_nodes); nodes = atoi(c_nodes);
				g->N  = atoi(c_edges); edges = atoi(c_edges);
				g->a_index_fanin = (unsigned *)malloc((nodes+1)*sizeof(unsigned));
				g->col_index_fanin = (unsigned *)malloc((edges)*sizeof(unsigned));
				g->a_index_fanout = (unsigned *)malloc((nodes+1)*sizeof(unsigned));
				g->col_index_fanout = (unsigned *)malloc((edges)*sizeof(unsigned));
				if (has_edge_weights) {
					g->edge_state = (edge_state_t *)malloc((edges)*sizeof(edge_state_t));
				}
				g->fanout_inedge_num = (unsigned *)malloc((edges)*sizeof(unsigned));
				fanin_count = (unsigned *)malloc((nodes)*sizeof(unsigned));
				fanout_count = (unsigned *)malloc((nodes)*sizeof(unsigned));
				froms = (unsigned *)malloc((edges)*sizeof(unsigned));
				tos = (unsigned *)malloc((edges)*sizeof(unsigned));
				if (has_edge_weights) {
					edge_weights = (edge_state_t *)malloc((edges)*sizeof(edge_state_t));
				}

				for (i=0;i<nodes;i++) {
					fanin_count[i] = 0;
					fanout_count[i] = 0;
				}
			}
		} else {
			char from[16], to[16], weights[16];
			if (has_edge_weights) {
				sscanf(line,"%s\t%s\t%s",from,to,weights);
				edge_weights[count] = (edge_state_t)atof(weights); // TODO: abstract this for all data types
			} else {
				sscanf(line,"%s\t%s",from,to);	
			}
			fanin_count[atoi(to)]++;
			fanout_count[atoi(from)]++;
			froms[count] = atoi(from);
			tos[count] = atoi(to);
			count++;
		}
	}

	fclose(f); // close file
	if (line) // free buffer
		free(line);

	(g->a_index_fanout)[0] = 0;
	(g->a_index_fanin)[0] = 0;
	
	int cifout_counter = 0;
	int cifin_counter = 0;

	unsigned *fanin_map = (unsigned *)malloc((edges)*sizeof(unsigned));

	for (i=0;i<nodes;i++){
		(g->a_index_fanout)[i+1] = (g->a_index_fanout)[i] + fanout_count[i];
		(g->a_index_fanin)[i+1] = (g->a_index_fanin)[i] + fanin_count[i];
		
		int fanin_read = fanin_count[i];
		int fanout_read = fanout_count[i];
		for (j=0;j<edges;j++){
			if (fanin_read == 0 && fanout_read == 0)
				break;
			if (froms[j] == i){
				(g->col_index_fanout)[cifout_counter] = tos[j];
				if (has_edge_weights) {
					(g->edge_state)[cifout_counter] = edge_weights[j];
				}
				cifout_counter++;
				fanout_read--;
			}
			if (tos[j] == i){
				(g->col_index_fanin)[cifin_counter] = froms[j];
				fanin_map[j] = cifin_counter;
				cifin_counter++;
				fanin_read--;
			}
		}
	}

	//to populate fanout_inedge_num
	cifout_counter = 0;
	for (i=0;i<nodes;i++){
		int fanout_read = fanout_count[i];
		for (j=0;j<edges;j++){
			if (fanout_read == 0)
				break;
			if (froms[j] == i) {
				(g->fanout_inedge_num)[cifout_counter] = fanin_map[j];
				cifout_counter++;
				fanout_read--;
			}
		}
	}

	// free memory
	free(fanin_map);
}

// creates a .dot graph file (useful for visualizing a BSP graph using the .dot open-source format)
void create_dot_graph(Graph *g) {
		int i,j;
		FILE *f;
		f = fopen("../graph.dot", "w");
		fprintf(f,"digraph visual {\n");
		for (i = 0; i < g->M; i++){
			fprintf(f,"\t%d[ label = \"%d\"];\n",i,i);
		}
		for (i = 0; i < g->M; i++){
			for (j = g->a_index_fanin[i]; j < g->a_index_fanin[i+1]; j++){
				fprintf(f,"\t%d -> %d [ label = \"\" ];\n",g->col_index_fanin[j],i);
			}
		}
		fprintf(f,"}\n");
		fclose(f);
}

void partition_graph(unsigned num_partitions, Graph *g, eGraph *eg, bool_t has_edge_weights) {

	unsigned M = g->M; // number of nodes in input graph
	unsigned N = g->N; // number of edges in input graph

	unsigned* placement = (unsigned *)malloc(M*sizeof(unsigned));

	// allocate memory in eGraph data structure
	eg->node_c = (unsigned *)malloc(num_partitions*sizeof(unsigned));
	eg->inedge_c = (unsigned *)malloc(num_partitions*sizeof(unsigned));
	eg->outedge_c = (unsigned *)malloc(num_partitions*sizeof(unsigned));

	int i,j,k; // local variables

	//init counts
	for (i = 0; i < num_partitions; i++){
		eg->node_c[i] = 0;
		eg->inedge_c[i] = 0;
		eg->outedge_c[i] = 0;
	}

	//assign random placement for each node
	for (i = 0; i < M; i++){
		unsigned pe = ((unsigned)rand()%num_partitions);
		//check that pe can hold the nodes/inedges/outedges (i.e. not at capacity)
		unsigned inedges = g->a_index_fanin[i+1] - g->a_index_fanin[i];
		unsigned outedges = g->a_index_fanout[i+1] - g->a_index_fanout[i];
	
		bool_t success = 0;
		unsigned orig_pe = pe;
		while (!success){

			if ((eg->node_c[pe]) < MAX_NODES_ECORE && 
			    (eg->inedge_c[pe]+inedges) <= MAX_INEDGES_ECORE && 
			    (eg->outedge_c[pe]+outedges) <= MAX_OUTEDGES_ECORE) 
			{
				placement[i] = pe;
				eg->node_c[pe]++;
				eg->inedge_c[pe] += inedges;
				eg->outedge_c[pe] += outedges;
				success = 1;
			} else { // pe is full, try new pe, round-robin style
				pe = (pe+1)%num_partitions;
				if (orig_pe == pe){ //cycled back to original PE, throw error
					printf("Cannot fit node into any PE! Exiting..\n");
					exit(1);
				}
			}
		}
	}

	// malloc shard arrays
	eg->inedge_offset = (uint16_t **)malloc(num_partitions*sizeof(uint16_t *));
	eg->outedge_offset = (uint16_t **)malloc(num_partitions*sizeof(uint16_t *));
#ifdef ADDR_GEN
	eg->outedge_state = (unsigned **)malloc(num_partitions*sizeof(unsigned *));
#else
	eg->outedge_state = (uint16_t **)malloc(num_partitions*sizeof(uint16_t *));
#endif
	eg->node_state = (node_state_t **)malloc(num_partitions*sizeof(node_state_t *));
	if (has_edge_weights) {
		eg->edge_state = (edge_state_t **)malloc(num_partitions*sizeof(edge_state_t *));
	}
	eg->node_id_shard = (unsigned **)malloc(num_partitions*sizeof(unsigned *));
	//node_id map, discard after using
	unsigned *node_id_map = (unsigned *)malloc(M*sizeof(unsigned));
	unsigned *fanin_local_num = (unsigned *)malloc(N*sizeof(unsigned));

	//shard all arrays
	for (i = 0; i < num_partitions; i++){
		//malloc inner shard arrays
		eg->inedge_offset[i] = (uint16_t *)malloc((eg->node_c[i]+1)*sizeof(uint16_t));
		eg->outedge_offset[i] = (uint16_t *)malloc((eg->node_c[i]+1)*sizeof(uint16_t));
		eg->node_state[i] = (node_state_t *)malloc((eg->node_c[i])*sizeof(node_state_t));
		if (has_edge_weights) {
			eg->edge_state[i] = (edge_state_t *)malloc((eg->inedge_c[i])*sizeof(edge_state_t));
		}
#ifdef ADDR_GEN
		eg->outedge_state[i] = (unsigned *)malloc(eg->outedge_c[i]*sizeof(unsigned));
#else
		eg->outedge_state[i] = (uint16_t *)malloc(eg->outedge_c[i]*sizeof(uint16_t));
#endif	
		
		//offset initialization
		eg->outedge_offset[i][0] = 0;
		eg->inedge_offset[i][0] = 0;

		eg->node_id_shard[i] = (unsigned *)malloc(eg->node_c[i]*sizeof(unsigned));

		unsigned counter = 0;
		unsigned counter_e = 0;
		for (j = 0; j < M; j++){
			if (placement[j] == i){
				eg->node_id_shard[i][counter] = j;
				node_id_map[j] = counter;
				for (k=g->a_index_fanin[j];k<g->a_index_fanin[j+1];k++){
					fanin_local_num[k] = counter_e;
					if (has_edge_weights) {
						eg->edge_state[i][counter_e] = g->edge_state[k];	
					}
					counter_e++;
				}
				eg->inedge_offset[i][counter+1] = counter_e;
				counter++;
			}
		}

	}

	//construct outedge_offset and outedge_state
	for (i = 0; i < num_partitions; i++){
		int local_edge_counter = 0;
		for (j = 0; j < eg->node_c[i]; j++){
			unsigned source_node_id = eg->node_id_shard[i][j]; //get original id of this node
			
			eg->node_state[i][j] = g->node_state[source_node_id]; // copy node state over from g to eg
			int outedges = g->a_index_fanout[source_node_id+1] - g->a_index_fanout[source_node_id];
			eg->outedge_offset[i][j+1] = eg->outedge_offset[i][j] + outedges;
			
			for (k = g->a_index_fanout[source_node_id]; k < g->a_index_fanout[source_node_id+1]; k++){
				unsigned target_node_id = g->col_index_fanout[k]; //get original id of target node
				unsigned localized_target_edge_id = fanin_local_num[g->fanout_inedge_num[k]];
				unsigned target_pe = placement[target_node_id];

#ifdef ADDR_GEN
				eg->outedge_state[i][local_edge_counter] = get_global_address(target_pe,localized_target_edge_id);
#else
				eg->outedge_state[i][local_edge_counter] = fanout_pack(target_pe,localized_target_edge_id);
#endif				
				local_edge_counter++;
			}
		}
	}

	// free memory
	free(node_id_map);
	free(fanin_local_num);
}

// pack a node's fanout into 2 bytes
uint16_t fanout_pack(unsigned target_pe, unsigned target_node) {

	int col = target_pe%4;
	int row = (target_pe - col)/4;

	uint16_t result = 0;
	result += (row << 14);
	result += (col << 12);
	result += target_node;

	return result;
}

// compute raw 32b address of a node's fanout
unsigned get_global_address(unsigned target_pe, unsigned target_node) {
	
	int col = target_pe % 4;
	int row = (target_pe - col)/4;

	unsigned offset = 0x2ff4 + sizeof(node_state_t)*target_node;
	unsigned address = 0x80800000 + row*(0x4000000) + col*(0x100000) + offset;
	
	return address;
}

// load graph data structure onto an eCore
void e_load_graph(unsigned i, unsigned j, unsigned coreid, e_epiphany_t *dev, unsigned bsp_iterations, eGraph *eg) {

	e_write(dev,i,j,E_NUM_NODES_ADDR,&(eg->node_c[coreid]),sizeof(unsigned));
	e_write(dev,i,j,E_NUM_INEDGES_ADDR,&(eg->inedge_c[coreid]),sizeof(unsigned));
	e_write(dev,i,j,E_NUM_OUTEDGES_ADDR,&(eg->outedge_c[coreid]),sizeof(unsigned));
	e_write(dev,i,j,E_TIMESTEPS_ADDR,&bsp_iterations,sizeof(unsigned));
	e_write(dev,i,j,E_NODE_STATE_ADDR,&(eg->node_state[coreid]),eg->node_c[coreid]*sizeof(node_state_t));
	if (EDGE_WEIGHTS == 1) { // if BSP graph has edge weights
		e_write(dev,i,j,E_EDGE_STATE_ADDR,&(eg->edge_state[coreid]),eg->inedge_c[coreid]*sizeof(edge_state_t));
	}
#ifdef ADDR_GEN
	e_write(dev,i,j,E_OUTEDGE_STATE_ADDR,&(eg->outedge_state[coreid]),eg->outedge_c[coreid]*sizeof(e_addr_t));
#else
	e_write(dev,i,j,E_OUTEDGE_STATE_ADDR,&(eg->outedge_state[coreid]),eg->outedge_c[coreid]*sizeof(uint16_t));
#endif
	e_write(dev,i,j,E_INEDGE_OFFSET_ADDR,&(eg->inedge_offset[coreid]),(eg->node_c[coreid]+1)*sizeof(uint16_t));
	e_write(dev,i,j,E_OUTEDGE_OFFSET_ADDR,&(eg->outedge_offset[coreid]),(eg->node_c[coreid]+1)*sizeof(uint16_t));
}

// loads the contents of each eCore's local memory, and sets app status to an init state
void e_init_memory(e_platform_t *platform, e_epiphany_t *dev, eGraph *eg, e_status_t *e_status, unsigned bsp_iterations) {
	int i,j;
	for (i = 0; i < platform->rows; i++){
		for (j = 0; j < platform->cols; j++){
			unsigned coreid = i*platform->rows + j;
			e_load_graph(i,j,coreid,dev,bsp_iterations,eg);
			// set eCore app status
			e_write(dev,i,j,E_STATUS_ADDR,e_status,sizeof(e_status_t));
		}
	}
}
