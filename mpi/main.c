#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#include <math.h>
#include <string.h>

#include "noise_sim.h"
#include "config_parser.h"
#include "mqtt.h"

#define DEBUG 0

int main(int argc, char** argv) {
    // Init random number generator
    srand(time(NULL));

    MPI_Init(NULL, NULL);

    // Config MPI_Datatypes
    MPI_Datatype mpi_sim_conf;
    MPI_Datatype mpi_mqtt_conf;
    MPI_Datatype mpi_all_conf;

    MPI_Datatype mpi_noise_source;
    MPI_Datatype mpi_noise_data;

    init_struct_sim_conf(&mpi_sim_conf);
    init_struct_mqtt_conf(&mpi_mqtt_conf); 
    init_struct_all_conf(&mpi_all_conf, &mpi_sim_conf, &mpi_mqtt_conf); 

    init_struct_noise_source(&mpi_noise_source);
    init_struct_noise_data(&mpi_noise_data);


    int my_rank, world_size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Process 0 (root) read the config file and creates the array
    all_config all_conf;
    noise_source *global_arr = NULL;
    noise_data *gather_buffer = NULL;
    int **gather_matrix = NULL;
     
    if (my_rank == 0) {
        // Read configs from file
        read_simulation_config(&all_conf.sim_conf);
        read_mqtt_config(&all_conf.mqtt_conf);
        
        all_conf.num_elem_per_proc = init_sources_array(&all_conf.sim_conf, &global_arr) / world_size;

        if (DEBUG) {
            // Gather all partial results in process 0
            gather_buffer = malloc(sizeof(noise_data) * all_conf.sim_conf.MAX_X * all_conf.sim_conf.MAX_Y);
            init_noise_sqm(&all_conf.sim_conf, &gather_matrix);
        }
    }

    MPI_Bcast(&all_conf, 1, mpi_all_conf, 0, MPI_COMM_WORLD);

    
    int num_elem_per_proc = all_conf.num_elem_per_proc;
    simulation_config sim_conf = all_conf.sim_conf;
    mqtt_config mqtt_conf = all_conf.mqtt_conf;

    // Initialize mosquitto connection
    struct mosquitto *mosq = NULL;
    init_mosquitto(&mqtt_conf, &mosq);

    // Element to be sent for each process
    int elem_send_count[world_size];
    // Array displacement of each process
    int elem_displs[world_size];

    int elem_res = (sim_conf.P + sim_conf.V) % world_size;
    int elem_inc = 0;

    for (int i = 0; i < world_size; i++) {
        elem_displs[i] = elem_inc;
        elem_send_count[i] = (i + 1 <= elem_res) ? num_elem_per_proc + 1 : num_elem_per_proc;
        elem_inc += elem_send_count[i];
    }
    
    // For each process, create a receive buffer
    int elem_size = elem_send_count[my_rank];
    noise_source *local_arr = malloc(sizeof(noise_source) * elem_size);

    int noises_per_proc = sim_conf.MAX_Y * sim_conf.MAX_X / world_size;
    int noises_recv_count[world_size];
    int noises_displs[world_size];

    int noises_res = (sim_conf.MAX_X * sim_conf.MAX_Y) % world_size;
    int noises_inc = 0;
    
    for (int i = 0; i < world_size; i++) {
        noises_displs[i] = noises_inc;
        noises_recv_count[i] = (i + 1 <= noises_res) ? noises_per_proc + 1 : noises_per_proc;
        noises_inc += noises_recv_count[i];
    }

    int noise_size = noises_recv_count[my_rank]; // Number of noises each process manage
    noise_data *my_noises = malloc(sizeof(noise_data) * noise_size);

    // Scatter the random numbers from process 0 to all processes
    MPI_Scatterv(global_arr, elem_send_count, elem_displs, mpi_noise_source, local_arr, elem_size, mpi_noise_source, 0, MPI_COMM_WORLD);

    int **noise_sqm = NULL;
    init_noise_sqm(&sim_conf, &noise_sqm);
    
    // Cycle every t seconds
    while(1) {
        // Compute the noise for each square meter in the region (sim_conf.MAX_Y x sim_conf.MAX_X)
        compute_noise_sqm(&sim_conf, &noise_sqm, local_arr, num_elem_per_proc);

        int count = 0;

        // Shuffle
        // Sending and receiving noises
        MPI_Request req;
        for (int i = 0; i < sim_conf.MAX_Y; i++) {
            for (int j = 0; j < sim_conf.MAX_X; j++) {
                int receiver = (i * sim_conf.MAX_X + j) % world_size;
                // I am the receiver
                if (receiver == my_rank) {

                    my_noises[count].noise_level = noise_sqm[i][j];
                    my_noises[count].x = j;
                    my_noises[count].y = i;

                    // I receive one message from any other process
                    for (int p = 0; p < world_size-1; p++) {
                        int rec_buffer;
                        MPI_Recv(&rec_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        my_noises[count].noise_level = sum_noises(my_noises[count].noise_level, rec_buffer);
                    }
                    count++;
                }
                // I am a sender: I can send asynchronously
                else {
                    MPI_Isend(&noise_sqm[i][j], 1, MPI_INT, receiver, 0, MPI_COMM_WORLD, &req);
                }
            }
        }


        // Each process send the computed noise level
        publish_data(&mqtt_conf, mosq, my_noises, noise_size);


        // GATHER TO ROOT PROCESS IS LEFT FOR DEBUGGING PURPOSES

        if (DEBUG) {
            
            MPI_Gatherv(my_noises, noise_size, mpi_noise_data, gather_buffer, noises_recv_count, noises_displs, mpi_noise_data, 0, MPI_COMM_WORLD);

            if (my_rank == 0) {
                for (int i = 0; i <  sim_conf.MAX_X * sim_conf.MAX_Y; i++) {
                    gather_matrix[gather_buffer[i].y][gather_buffer[i].x] = gather_buffer[i].noise_level;
                }

                print_matrix(gather_matrix, sim_conf.MAX_X, sim_conf.MAX_Y);
            }
        }

        move_noise_sources(&sim_conf, local_arr, num_elem_per_proc);

        // Zero out matrices and arrays for next cycle
        reset_matrix(&noise_sqm, sim_conf.MAX_X, sim_conf.MAX_Y);

        sleep(sim_conf.t);
    }
    
    shutdown_mosquitto();

    // Clean up
    if (my_rank == 0) {
        free(global_arr);
        
        if (DEBUG) {
            free_matrix(&gather_matrix, sim_conf.MAX_Y);
            free(gather_buffer);
        }
    }

    free_matrix(&noise_sqm, sim_conf.MAX_Y);
    free(local_arr);
    free(my_noises);   
    
    MPI_Type_free(&mpi_all_conf);
    MPI_Type_free(&mpi_sim_conf);
    MPI_Type_free(&mpi_mqtt_conf);
    
    MPI_Type_free(&mpi_noise_source);
    MPI_Type_free(&mpi_noise_data);
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}