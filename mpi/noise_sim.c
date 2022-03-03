#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <mpi.h>

#define P 100000
#define V 50000
#define W 10000
#define L 10000
#define Np 40
#define Nv 80
#define Dp 5
#define Dv 20
#define Vp 2
#define Vv 40
#define t 10

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


noise_source *read_sim_params() {
    noise_source *sources = malloc((P+V) * sizeof(noise_source));
    
    // Noise sources for people
    for (size_t i = 0; i < P; i++) {
        // Compute random number between 0 and L for coord X
        int x = rand() % L;

        // Compute random number between 0 and W for coord Y
        int y = rand() % W;

        sources[i].x = x;
        sources[i].y = y;
        sources[i].noise_level = Np;
        sources[i].distance_affected = Dp;
        sources[i].moving_speed = Vp;
    }

    // Noise sources for vehicles
    for (size_t i = P; i < V+P; i++) {
        // Compute random number between 0 and L for coord X
        int x = rand() % L;

        // Compute random number between 0 and W for coord Y
        int y = rand() % W;

        sources[i].x = x;
        sources[i].y = y;
        sources[i].noise_level = Nv;
        sources[i].distance_affected = Dv;
        sources[i].moving_speed = Vv;
    }

    return sources;
}

int **init_noise_sqm() {
    int **noise_sqm = calloc(W, 1+sizeof(int*)); // alloc one extra ptr to check later for NULL on freeing

    for(size_t i = 0; i < W; i++) {
        noise_sqm[i] = calloc(L, sizeof(int));
    }
    noise_sqm[W] = NULL; // set the extra ptr to NULL

    return noise_sqm;
}

int sum_noises(int noise1, int noise2) {
    // Two checks to avoid floor and get better approximation
    if (noise1 == 0) {
        return noise2;
    }

    if (noise2 == 0) {
        return noise1;
    }

    return 10 * log10( pow(10, (noise1 / 10)) + pow(10, (noise2 / 10)) );
}

int **compute_noise_sqm(noise_source *sources, int num_elem) {
    int **noise_sqm = init_noise_sqm();

    for (size_t i = 0; i < num_elem; i++) {

        int x = sources[i].x;
        int y = sources[i].y;

        noise_sqm[x][y] = sum_noises(noise_sqm[x][y], sources[i].noise_level);

        for (size_t j = 0; j < sources[i].distance_affected; j++) {

            for (size_t k = 0; k < sources[i].distance_affected; k++) {
                if (x+j < W && y+k < L) {
                    noise_sqm[x+j][y+k] = sum_noises(noise_sqm[x+j][y+k], sources[i].noise_level);
                }

                if (x-j >= 0 && y+k < L) {
                    noise_sqm[x-j][y+k] = sum_noises(noise_sqm[x-j][y+k], sources[i].noise_level);
                }

                if (x+j < W && y-k >= 0) {
                    noise_sqm[x+j][y-k] = sum_noises(noise_sqm[x+j][y-k], sources[i].noise_level);
                }

                if (x-j >= 0 && y-k >= 0) {
                    noise_sqm[x-j][y-k] = sum_noises(noise_sqm[x-j][y-k], sources[i].noise_level);
                }     
            }
        }
    }

    return **noise_sqm;
}


int main(int argc, char** argv) {
    // Init random number generator
    srand(time(NULL));

    MPI_Init(NULL, NULL);

    MPI_Datatype mpi_noise_source;
    int struct_len = 5;
    int block_lens[] = { 1, 1, 1, 1, 1 };
    MPI_Aint displacements[] = { 
        offsetof(noise_source, noise_level),
        offsetof(noise_source, distance_affected),
        offsetof(noise_source, moving_speed),
        offsetof(noise_source, x),
        offsetof(noise_source, y)
    };
    MPI_Datatype types[] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT };

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &mpi_noise_source);
    MPI_Type_commit(&mpi_noise_source);



    // Struct noise_data
    MPI_Datatype mpi_noise_data;
    int struct_len = 3;
    int block_lens[] = { 1, 1, 1 };
    MPI_Aint displacements[] = { 
        offsetof(noise_data, noise_level),
        offsetof(noise_data, x),
        offsetof(noise_data, y)
    };
    MPI_Datatype types[] = { MPI_INT, MPI_INT, MPI_INT };

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &mpi_noise_data);
    MPI_Type_commit(&mpi_noise_data);




    int my_rank, world_size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Process 0 creates the array
    noise_source *global_arr = NULL;
    int num_elem_per_proc = 0;
    if (my_rank == 0) {
        global_arr = read_sim_params();
        num_elem_per_proc = sizeof(global_arr) / sizeof(global_arr[0]);
    }

    // For each process, create a receive buffer
    noise_source *local_arr = malloc(sizeof(noise_source) * num_elem_per_proc);

    // Scatter the random numbers from process 0 to all processes
    MPI_Scatter(global_arr, num_elem_per_proc, mpi_noise_source, local_arr, num_elem_per_proc, mpi_noise_source, 0, MPI_COMM_WORLD);

    while(1) {
        // Compute the noise for each square meter in the region (W x L)
        int noise_sqm[W][L] = compute_noise_sqm(local_arr, num_elem_per_proc);


        // Shuffle
        // Sending and receiving noises
        MPI_Request req;
        for (size_t i = 0; i < W; i++) {
            for (size_t j = 0; j < L; j++) {
                int receiver = (i*L + j) % world_size;
                // I am the receiver
                if (receiver == my_rank) {

                    noise_data *my_noise = malloc(sizeof(noise_data));

                    my_noise.noise_level = noise_sqm[i][j];
                    my_noise.x = i;
                    my_noise.y = j;

                    // I receive one message from any other process
                    for (int p = 0; p < world_size-1; p++) {
                        int rec_buffer;
                        MPI_Recv(&rec_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        my_noise.noise_level = sum_noise(my_noise.noise_level, rec_buffer);
                    }
                }
                // I am a sender: I can send asynchronously
                else {
                    MPI_Isend(&noise_sqm[i][j], 1, MPI_INT, receiver, 0, MPI_COMM_WORLD, &req);
                }
            }
        }


        // Gather all partial results in process 0
        float *gather_buffer = NULL;
        if (my_rank == 0) {
            gather_buffer = (float *) malloc(sizeof(float) * world_size);
        }
        MPI_Gather(&local_average, 1, MPI_FLOAT, gather_buffer, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

        sleep(t);
    }

    // Compute the final average in process 0
    if (my_rank == 0) {
        float result = compute_final_average(gather_buffer, world_size);
        printf("The average is %f\n", result);

        // Sequential code to check correctness
        float sequential_result = compute_average(global_arr, num_elements_per_proc * world_size);
        printf("The average (sequential computation) is %f\n", sequential_result);
    }







    // Clean up
    if (my_rank == 0) {
        free(global_arr);
        //free(gather_buffer);
    }
    //free(local_arr);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}