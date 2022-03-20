#ifndef NOISE_SIM_H
#define NOISE_SIM_H

#include <mpi.h>
#include <math.h>
#include <unistd.h>

#include "config_parser.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
     
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


typedef struct all_config_t {
    simulation_config sim_conf;
    mqtt_config mqtt_conf;
    int num_elem_per_proc;
} all_config;

typedef struct noise_source_t {
    int noise_level;
    int distance_affected;
    int moving_speed;
    int x;
    int y;
} noise_source;

typedef struct noise_data_t {
    int noise_level;
    int x;
    int y;
} noise_data;

void init_struct_sim_conf(MPI_Datatype *mpi_sim_conf);

void init_struct_mqtt_conf(MPI_Datatype *mpi_mqtt_conf);

void init_struct_all_conf(MPI_Datatype *mpi_all_conf, MPI_Datatype *mpi_sim_conf, MPI_Datatype *mpi_mqtt_conf);

void init_struct_noise_source(MPI_Datatype *mpi_noise_source);

void init_struct_noise_data(MPI_Datatype *mpi_noise_data);

void print_matrix(int **matrix, int row, int col);

void print_matrix_file(int **matrix, int max_x, int max_y);

void print_counts_and_displs(int world_size, int *counts, int *displs); 

void print_array(noise_source *arr, int numel);

void print_my_noise(noise_data *my_noise);

int init_sources_array(simulation_config *sim_conf, noise_source **ptr_sources);

int sum_noises(int noise1, int noise2);

void init_noise_sqm(simulation_config *sim_conf, int ***noise_sqm);

void compute_noise_sqm(simulation_config *sim_conf, int ***noise_sqm, noise_source *sources, int num_elem);

void move_noise_sources(simulation_config *sim_conf, noise_source *sources, int num_elem);

void reset_matrix(int ***mymatrix, int m, int n);

void free_matrix(int ***matrix, int size_y);

#endif // NOISE_SIM_H