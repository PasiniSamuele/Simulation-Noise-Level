#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <mpi.h>
#include <math.h>
#include <unistd.h>

#define P 1
#define V 1
#define MAX_Y 5 // W
#define MAX_X 5 // L
#define Np 40
#define Nv 45
#define Dp 1
#define Dv 2
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
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n\n");
}

void print_matrix_file(int **matrix, int max_x, int max_y) {
    FILE *fp;
    fp = fopen("test.txt", "w+");

    for (int i = 0; i < max_y; i++) {
        for (int j = 0; j < max_x; j++) {
            fprintf(fp, "%d\t", matrix[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n\n");
    fclose(fp);
}

void print_counts_and_displs(int world_size, int *counts, int *displs) {
    printf("Counts: [");
    for (int i = 0; i < world_size; i++) {
        printf("%d, ", counts[i]);
    }
    printf("]\n");

    printf("Displs: [");
    for (int i = 0; i < world_size; i++) {
        printf("%d, ", displs[i]);
    }
    printf("]\n");
}

void print_array(noise_source *arr, int numel) {
    printf("(X, Y) = noise dB\n");
    for (int i = 0; i < numel; i++) {
        printf("(%d, %d) = %d dB\n", arr[i].x, arr[i].y, arr[i].noise_level);
    }
    printf("\n");
}


int read_sim_params(noise_source **ptr_sources) {
    *ptr_sources = malloc((P+V) * sizeof(noise_source));
    noise_source *sources = *ptr_sources;
        
    // Noise sources for people
    for (int i = 0; i < P; i++) {
        // Compute random number between 0 and MAX_X for coord X
        int x = rand() % MAX_X;

        // Compute random number between 0 and MAX_Y for coord Y
        int y = rand() % MAX_Y;

        //sources[i].x = x;
        //sources[i].y = y;
        sources[i].x = 4;
        sources[i].y = 4;
        sources[i].noise_level = Np;
        sources[i].distance_affected = Dp;
        sources[i].moving_speed = Vp;
    }

    // Noise sources for vehicles
    for (int i = P; i < V+P; i++) {
        // Compute random number between 0 and MAX_X for coord X
        int x = rand() % MAX_X;

        // Compute random number between 0 and MAX_Y for coord Y
        int y = rand() % MAX_Y;

        //sources[i].x = x;
        //sources[i].y = y;
        sources[i].x = 4;
        sources[i].y = 4;
        sources[i].noise_level = Nv;
        sources[i].distance_affected = Dv;
        sources[i].moving_speed = Vv;
    }
    
    return V+P; // total number of elements added into sources
}

int **init_noise_sqm() {
    int **noise_sqm = calloc(MAX_Y, 1+sizeof(int*)); // alloc one extra ptr to check later for NULL on freeing

    for(int i = 0; i < MAX_Y; i++) {
        noise_sqm[i] = calloc(MAX_X, sizeof(int));
    }
    noise_sqm[MAX_Y] = NULL; // set the extra ptr to NULL

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

    return round(10 * log10( pow(10, (noise1 / 10.0)) + pow(10, (noise2 / 10.0)) ) );
}

int **compute_noise_sqm(noise_source *sources, int num_elem) {
    int **noise_sqm = init_noise_sqm();

    for (int i = 0; i < num_elem; i++) {

        int x = sources[i].x;
        int y = sources[i].y;

        noise_sqm[y][x] = sum_noises(noise_sqm[y][x], sources[i].noise_level);

        for (int j = 1; j <= sources[i].distance_affected; j++) {
            if (x+j < MAX_X) {
                noise_sqm[y][x+j] = sum_noises(noise_sqm[y][x+j], sources[i].noise_level);
            }

            if (x-j >= 0) {
                noise_sqm[y][x-j] = sum_noises(noise_sqm[y][x-j], sources[i].noise_level);
            }

            if (y-j >= 0) {
                noise_sqm[y-j][x] = sum_noises(noise_sqm[y-j][x], sources[i].noise_level);
            }

            if (y+j < MAX_Y) {
                noise_sqm[y+j][x] = sum_noises(noise_sqm[y+j][x], sources[i].noise_level);
            }  
        }


        for (int j = 1; j <= sources[i].distance_affected; j++) {
            for (int k = 1; k <= sources[i].distance_affected; k++) {
                if (x+j < MAX_X && y+k < MAX_Y) {
                    noise_sqm[y+k][x+j] = sum_noises(noise_sqm[y+k][x+j], sources[i].noise_level);
                }

                if (x-j >= 0 && y+k < MAX_Y) {
                    noise_sqm[y+k][x-j] = sum_noises(noise_sqm[y+k][x-j], sources[i].noise_level);
                }

                if (x+j < MAX_X && y-k >= 0) {
                    noise_sqm[y-k][x+j] = sum_noises(noise_sqm[y-k][x+j], sources[i].noise_level);
                }

                if (x-j >= 0 && y-k >= 0) {
                    noise_sqm[y-k][x-j] = sum_noises(noise_sqm[y-k][x-j], sources[i].noise_level);
                }     
            }
        }
    }

    return noise_sqm;
}

void move_noise_sources(noise_source *sources, int num_elem) {
    for (int i = 0; i < num_elem; i++) {

        int delta_x = sources[i].moving_speed * (rand() % 3 - 1); // Possible values of (rand() % 3 - 1) are -1, 0, 1
        int delta_y = sources[i].moving_speed * (rand() % 3 - 1);

        if (sources[i].x + delta_x >= MAX_X || sources[i].x + delta_x < 0 ) {
            delta_x = -delta_x;
        }

        if (sources[i].y + delta_y >= MAX_Y || sources[i].y + delta_y < 0 ) {
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


void print_my_noise(noise_data *my_noise) {
    printf("Noise in (%d, %d) = %d dB\n", my_noise->x, my_noise->y, my_noise->noise_level);
}

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

    // Element to be sent for each process
    int elem_send_count[world_size];
    // Array displacement of each process
    int elem_displs[world_size];

    int elem_res = (P + V) % world_size;
    int elem_inc = 0;

    for (int i = 0; i < world_size; i++) {
        elem_displs[i] = elem_inc;
        elem_send_count[i] = (i + 1 <= elem_res) ? num_elem_per_proc + 1 : num_elem_per_proc;
        elem_inc += elem_send_count[i];
    }

    // For each process, create a receive buffer
    int elem_size = elem_send_count[my_rank];
    noise_source *local_arr = malloc(sizeof(noise_source) * elem_size);

    int noises_per_proc = MAX_Y * MAX_X / world_size;
    int noises_recv_count[world_size];
    int noises_displs[world_size];

    int noises_res = (MAX_X * MAX_Y) % world_size;
    int noises_inc = 0;

    for (int i = 0; i < world_size; i++) {
        noises_displs[i] = noises_inc;
        noises_recv_count[i] = (i + 1 <= noises_res) ? noises_per_proc + 1 : noises_per_proc;
        noises_inc += noises_recv_count[i];
    }

    int noise_size = noises_recv_count[my_rank];
    noise_data *my_noises = malloc(sizeof(noise_data) * noise_size);

    /* if(!my_rank) {
        printf("-- Elem send --\n");
        print_counts_and_displs(world_size, elem_send_count, elem_displs);

        printf("-- Noise recv --\n");
        print_counts_and_displs(world_size, noises_recv_count, noises_displs);
    }*/

    // Scatter the random numbers from process 0 to all processes
    MPI_Scatterv(global_arr, elem_send_count, elem_displs, mpi_noise_source, local_arr, elem_size, mpi_noise_source, 0, MPI_COMM_WORLD);
    
    while(1) {
        // Compute the noise for each square meter in the region (MAX_Y x MAX_X)
        int **noise_sqm = compute_noise_sqm(local_arr, num_elem_per_proc);

        if (my_rank == 0) {
            print_matrix(noise_sqm, MAX_Y, MAX_X);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);

        if (my_rank != 0) {
            print_matrix(noise_sqm, MAX_Y, MAX_X);
        }
        
        int count = 0;

        // Shuffle
        // Sending and receiving noises
        MPI_Request req;
        for (int i = 0; i < MAX_Y; i++) {
            for (int j = 0; j < MAX_X; j++) {
                int receiver = (i * MAX_X + j) % world_size;
                // I am the receiver
                if (receiver == my_rank) {

                    my_noises[count].noise_level = noise_sqm[i][j];
                    my_noises[count].x = j;
                    my_noises[count].y = i;

                    // I receive one message from any other process
                    for (int p = 0; p < world_size-1; p++) {
                        int rec_buffer;
                        MPI_Recv(&rec_buffer, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                        //printf("PRIMA (x, y) = noise_level, rec_buffer: (%d, %d) = %d, %d\n", j, i, my_noises[count].noise_level, rec_buffer);
                        my_noises[count].noise_level = sum_noises(my_noises[count].noise_level, rec_buffer);
                        //printf("DOPO  (x, y) = noise_level, rec_buffer: (%d, %d) = %d, %d\n", j, i, my_noises[count].noise_level, rec_buffer);
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

        MPI_Gatherv(my_noises, noise_size, mpi_noise_data, gather_buffer, noises_recv_count, noises_displs, mpi_noise_data, 0, MPI_COMM_WORLD);

        // Print
        if (my_rank == 0) {
            /*for (int i = 0; i < noises_per_proc * world_size; i++) {
                printf("(x, y) = noise_level: (%d, %d) = %d\n", gather_buffer[i].x, gather_buffer[i].y, gather_buffer[i].noise_level);
            }*/

            int **matrix = init_noise_sqm();

            for (int i = 0; i <  MAX_X * MAX_Y; i++) {
                matrix[gather_buffer[i].y][gather_buffer[i].x] = gather_buffer[i].noise_level;
            }

            print_matrix(matrix, MAX_X, MAX_Y);
            //print_matrix_file(matrix, MAX_X, MAX_Y);
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