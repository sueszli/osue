#include "generator.h"

/**
 * @file generator.c
 * @author Franz Lloyd Azucena <e1425044@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief generates results
 *
 * @details generates results and writes them into the buffer. 
 **/

int get_random_color() {
	int randomnumber;
    randomnumber = rand() % 3;	// 3 colors
	return randomnumber;
}

int* color_graph(int node_count) {
	// allocate memory for directed graph
	int* node_colors = malloc(sizeof(int)*node_count);
	for(int i = 0; i < node_count; i++) {
		node_colors[i] = get_random_color();
	}
	return node_colors;
}

int get_node_count(edge* graph, int edge_count) {
	int highest_node_name = 0;
	for(int i = 0; i < edge_count; i++) {
		int node1 = graph[i].node1;
		int node2 = graph[i].node2;
		if(highest_node_name<node1) {
			highest_node_name = node1;
		}
		if(highest_node_name<node2) {
			highest_node_name = node2;
		}
	}
	return highest_node_name;
}

void generate_graph(int argc, char** argv, edge* graph) {
	int node1;
	int node2;

	for(int i = 1; i < argc; i++) {
		edge e;
		int variables_filled = sscanf(argv[i], "%d-%d", &node1, &node2);
		
		if(variables_filled != 2 || node1<0 || node2<0) {
			remove_buffer_with_error(name, "malformed graph: example of valid edge: 1-2");
		}

		e.node1 = node1;
		e.node2 = node2;
		graph[i-1] = e;
	}
}

result get_result(edge* graph, int node_count, int edge_count, int* colored_graph) {
	result res;
	
	int result_counter = 0;
	
	// remove all edges leading to nodes that have the same color as the current node!
	for(int i = 0; i < node_count; i++) {
		// select all edges ( u, v ) for which the color of u is different to the color of v.
		for(int x = 0; x < edge_count; x++) {
			edge e = graph[x];
			if(e.node1 != i) {
				continue;
			}
			if(colored_graph[i]==colored_graph[e.node2]) {
				
				(&res)->removed_edges[result_counter] = e;
				++result_counter;
			}
		}
	}
	
	return res;
}

int main(int argc, char* argv[]) {
	
	name = argv[0];

	if(argc == 1) {
		remove_buffer_with_error(name, "graph is empty: at least one edge is needed");
	}

	int edge_count = argc -1;

	// allocate memory for directed graph
	edge* graph = malloc(sizeof(edge)*edge_count);

	init(true);

	if(sem_post(termination_sem) != 0) {
		if(EINTR == errno) {
			// nvm
		} else {
			remove_buffer_with_error(name, "failed to increment termination_sem");
		}
	}

	/*
	 * init random number generator
	 * call only ONCE per programm!
	 * generates only one random number per second
	 * --> modify seed
	 *
	 *  time(NULL) returns the time in seconds sind the epoch
	 * */
	
	int offset = get_seed_offset();
	if(offset == -1) {
		remove_buffer_with_error(name, "error while accessing seed_offset");
	}
	srand(time(NULL)+offset);
	

	generate_graph(argc, argv, graph);
		
	int node_count = get_node_count(graph, edge_count);
	
	int termination_value;
	while(true) {
		if(sem_getvalue(termination_sem, &termination_value) != 0) {
			remove_buffer_with_error(name, "error while accessing termination_sem");
		}
		if(termination_value==0) {
			// terminate
			remove_buffer();
		}
		
		int* colored_graph = color_graph(node_count);

		result res = get_result(graph, node_count, edge_count, colored_graph);

		// check if result is within the limit
		if(count_edges(&res) <= LIMIT_EDGES) {
			print_result(&res);

			if(write_buffer(&res) != 0) {
				remove_buffer_with_error(name, "error while writing to circular buffer");
			}
		}
	}

	free(graph);
	sem_post(free_space_sem);
	
	remove_buffer();
	exit(EXIT_SUCCESS);

}
