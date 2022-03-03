#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <unistd.h>

#define P 2
#define V 2
#define W 5
#define L 5
#define Np 40
#define Nv 80
#define Dp 1
#define Dv 1
#define Vp 1
#define Vv 1
#define t 5

#define DEBUG 1

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


void print_matrix(int **matrix, int row, int col) {
    for (size_t i = 0; i < row; i++) {
        for (size_t j = 0; j < col; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void print_array(noise_source *arr, int numel) {
    printf("(X, Y) = noise dB\n");
    for (size_t i = 0; i < numel; i++) {
        printf("(%d, %d) = %d dB\n", arr[i].x, arr[i].y, arr[i].noise_level);
    }
    printf("\n");
}


int read_sim_params(noise_source **ptr_sources) {
    *ptr_sources = malloc((P+V) * sizeof(noise_source));
    noise_source *sources = *ptr_sources;
        
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
    printf("\nPIPPO\n");
    
    return V+P; // total number of elements added into sources
}

int **init_noise_sqm() {
    int **noise_sqm = calloc(L, 1+sizeof(int*)); // alloc one extra ptr to check later for NULL on freeing

    for(size_t i = 0; i < L; i++) {
        noise_sqm[i] = calloc(W, sizeof(int));
    }
    noise_sqm[L] = NULL; // set the extra ptr to NULL

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

    print_matrix(noise_sqm, L, W);

    for (size_t i = 0; i < num_elem; i++) {

        int x = sources[i].x;
        int y = sources[i].y;

        noise_sqm[y][x] = sum_noises(noise_sqm[y][x], sources[i].noise_level);

        /* for (size_t j = 0; j < sources[i].distance_affected; j++) {

            for (size_t k = 0; k < sources[i].distance_affected; k++) {
                if (x+j < L && y+k < W) {
                    noise_sqm[y+k][x+j] = sum_noises(noise_sqm[y+k][x+j], sources[i].noise_level);
                }

                if (x-j >= 0 && y+k < W) {
                    noise_sqm[y+k][x-j] = sum_noises(noise_sqm[y+k][x-j], sources[i].noise_level);
                }

                if (x+j < L && y-k >= 0) {
                    noise_sqm[y-k][x+j] = sum_noises(noise_sqm[y-k][x+j], sources[i].noise_level);
                }

                if (x-j >= 0 && y-k >= 0) {
                    noise_sqm[y-k][x-j] = sum_noises(noise_sqm[y-k][x-j], sources[i].noise_level);
                }     
            }
        } */
    }

    print_matrix(noise_sqm, L, W);

    return noise_sqm;
}

void move_noise_sources(noise_source *sources, int num_elem) {
    for (size_t i = 0; i < num_elem; i++) {

        int delta_x = sources[i].moving_speed * (rand() % 3 - 1); // Possible values of (rand() % 3 - 1) are -1, 0, 1
        int delta_y = sources[i].moving_speed * (rand() % 3 - 1);

        if (sources[i].x + delta_x >= L || sources[i].x + delta_x < 0 ) {
            delta_x = -delta_x;
        }

        if (sources[i].y + delta_y >= W || sources[i].y + delta_y < 0 ) {
            delta_y = -delta_y;
        }

        sources[i].x += delta_x;
        sources[i].y += delta_y;
    }
}

/* void init_struct_noise_source(MPI_Datatype *mpi_noise_source) {
    // Struct noise_source
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
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, mpi_noise_source);
    MPI_Type_commit(mpi_noise_source);
}

void init_struct_noise_data(MPI_Datatype *mpi_noise_data) {
    // Struct noise_data
    int struct_len = 3;
    int block_lens[] = { 1, 1, 1 };
    MPI_Aint displacements[] = { 
        offsetof(noise_data, noise_level),
        offsetof(noise_data, x),
        offsetof(noise_data, y)
    };
    MPI_Datatype types[] = { MPI_INT, MPI_INT, MPI_INT };

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, mpi_noise_data);
    MPI_Type_commit(mpi_noise_data);
} */


int main(int argc, char** argv) {
    // Init random number generator
    srand(time(NULL));

    MPI_Init(NULL, NULL);
 
    MPI_Datatype mpi_noise_data;
    MPI_Datatype mpi_noise_source;
    /*
    init_struct_noise_source(&mpi_noise_data);
    init_struct_noise_data(&mpi_noise_source); */

    // Struct noise_source
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
    int struct_len2 = 3;
    int block_lens2[] = { 1, 1, 1 };
    MPI_Aint displacements2[] = { 
        offsetof(noise_data, noise_level),
        offsetof(noise_data, x),
        offsetof(noise_data, y)
    };
    MPI_Datatype types2[] = { MPI_INT, MPI_INT, MPI_INT };

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len2, block_lens2, displacements2, types2, &mpi_noise_data);
    MPI_Type_commit(&mpi_noise_data);







    int my_rank, world_size; 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Process 0 creates the array
    noise_source *global_arr = NULL;    
    int num_elem_per_proc = 0;
    if (my_rank == 0) {
        num_elem_per_proc = read_sim_params(&global_arr) / world_size;
    }

    MPI_Bcast(&num_elem_per_proc, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // For each process, create a receive buffer
    noise_source *local_arr = malloc(sizeof(noise_source) * num_elem_per_proc);

    int noises_per_proc = W * L / world_size;

    // Scatter the random numbers from process 0 to all processes
    MPI_Scatter(global_arr, num_elem_per_proc, mpi_noise_source, local_arr, num_elem_per_proc, mpi_noise_source, 0, MPI_COMM_WORLD);

    print_array(local_arr, num_elem_per_proc);

    
    while(1) {
        
        // Compute the noise for each square meter in the region (W x L)
        int **noise_sqm = compute_noise_sqm(local_arr, num_elem_per_proc);
        

        int count = 0;
        noise_data *my_noises = malloc(sizeof(noise_data) * noises_per_proc);

        // Shuffle
        // Sending and receiving noises
        MPI_Request req;
        for (size_t i = 0; i < L; i++) {
            for (size_t j = 0; j < W; j++) {
                int receiver = (i*W + j) % world_size;
                // I am the receiver
                if (receiver == my_rank) {

                    my_noises[count].noise_level = noise_sqm[i][j];
                    my_noises[count].x = i;
                    my_noises[count].y = j;

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


        // Gather all partial results in process 0
        noise_data *gather_buffer = NULL;
        if (my_rank == 0) {
            gather_buffer = malloc(sizeof(noise_data) * world_size * noises_per_proc);
        }
        MPI_Gather(&my_noises, noises_per_proc, mpi_noise_data, gather_buffer, noises_per_proc, mpi_noise_data, 0, MPI_COMM_WORLD);


        // Print
        if (my_rank == 0) {
            for (size_t i = 0; i < world_size * noises_per_proc; i++) {
                printf("Noise in cell (%d, %d) is %d dB\n", gather_buffer[i].x, gather_buffer[i].y, gather_buffer[i].noise_level);
            }
        }

        move_noise_sources(local_arr, num_elem_per_proc);

        sleep(t);
    }



/*



    // TODO FREEEEEEE

    // Clean up
    if (my_rank == 0) {
        free(global_arr);
        //free(gather_buffer);
    }
    //free(local_arr);
    */

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}