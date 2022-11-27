#include <semaphore.h>

sem_t *sem_free_space;
sem_t *sem_used_space;
sem_t *sem_write_index;
circular_buffer_t *shared_memory;
int *shared_write_index;
int shm_fd;
int shm_write_idx_fd;
int write_index = 0;

void usage(void);
edge_t *parse_edges(int count_edges, char *input_edges[]);
edge_t parse_edge(char *input_edge);
vertex_data_t *get_vertices(edge_t *edges);
void extract_vertices(graph_data_t *graph);
void check_and_add_vertex(graph_data_t *graph, vertex_t vertex);
int contains_vertex(graph_data_t *graph, vertex_t vertex);
void check_malloc(void *pointer);
void check_realloc(void *pointer);
void color_vertices(graph_data_t *graph);
int remove_color_pairs(graph_data_t *graph, edge_t *removed_edges);
int get_vertex_color(graph_data_t *graph, vertex_t vertex);
int contains_edge(edge_t *edges, int edges_count, edge_t edge);

void handle_signal(int signal);
void init_sem_shm(void);

void exit_error(void);

void free_memory(void);

void handle_close(int fd);
void handle_sem_close(sem_t *sem);