#include "noise_sim.h"

void init_struct_sim_conf(MPI_Datatype *mpi_sim_conf) {
    // Struct sim_config
    int struct_len = 11;
    int block_lens[] = {
        1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1
    };
    MPI_Aint displacements[] = { 
        offsetof(simulation_config, P),
        offsetof(simulation_config, V),
        offsetof(simulation_config, MAX_Y),
        offsetof(simulation_config, MAX_X),
        offsetof(simulation_config, Np),
        offsetof(simulation_config, Nv),
        offsetof(simulation_config, Dp),
        offsetof(simulation_config, Dv),
        offsetof(simulation_config, Vp),
        offsetof(simulation_config, Vv),
        offsetof(simulation_config, t)
    };
    MPI_Datatype types[] = {
        MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT,
        MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT
    };
    MPI_Datatype tmp_type;
    MPI_Aint lb, extent;

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, mpi_sim_conf);
    MPI_Type_commit(mpi_sim_conf);
}

void init_struct_mqtt_conf(MPI_Datatype *mpi_mqtt_conf) {
    // Struct all_config
    int struct_len = 6;
    int block_lens[] = { SIZE_CONF_STR, 1, 1, SIZE_CONF_STR, SIZE_CONF_STR, SIZE_CONF_STR};
    MPI_Aint displacements[] = { 
        offsetof(mqtt_config, ip),
        offsetof(mqtt_config, port),
        offsetof(mqtt_config, keep_alive),
        offsetof(mqtt_config, username),
        offsetof(mqtt_config, password),
        offsetof(mqtt_config, topic)
    };
    MPI_Datatype types[] = { MPI_CHAR, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR, MPI_CHAR };
    MPI_Datatype tmp_type;
    MPI_Aint lb, extent;

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, mpi_mqtt_conf);
    MPI_Type_commit(mpi_mqtt_conf);
}

void init_struct_all_conf(MPI_Datatype *mpi_all_conf, MPI_Datatype *mpi_sim_conf, MPI_Datatype *mpi_mqtt_conf) {
    // Struct all_conf
    int struct_len = 3;
    int block_lens[] = { 11, 6, 1 };
    MPI_Aint displacements[] = { 
        offsetof(all_config, sim_conf),
        offsetof(all_config, mqtt_conf),
        offsetof(all_config, num_elem_per_proc)
    };
    MPI_Datatype types[] = { *mpi_sim_conf, *mpi_mqtt_conf, MPI_INT };
    MPI_Datatype tmp_type;
    MPI_Aint lb, extent;

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, mpi_all_conf);
    MPI_Type_commit(mpi_all_conf);
}

void init_struct_noise_source(MPI_Datatype *mpi_noise_source) {
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
    MPI_Datatype tmp_type;
    MPI_Aint lb, extent;

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, mpi_noise_source);
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
    MPI_Datatype tmp_type;
    MPI_Aint lb, extent;

    // Create and commit the data structure
    MPI_Type_create_struct(struct_len, block_lens, displacements, types, &tmp_type);
    MPI_Type_get_extent(tmp_type, &lb, &extent);
    MPI_Type_create_resized(tmp_type, lb, extent, mpi_noise_data);
    MPI_Type_commit(mpi_noise_data);
}

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

void print_my_noise(noise_data *my_noise) {
    printf("Noise in (%d, %d) = %d dB\n", my_noise->x, my_noise->y, my_noise->noise_level);
}

int init_sources_array(simulation_config *sim_conf, noise_source **ptr_sources) {
    *ptr_sources = malloc((sim_conf->P + sim_conf->V) * sizeof(noise_source));
    noise_source *sources = *ptr_sources;
        
    // Noise sources for people
    for (int i = 0; i < sim_conf->P; i++) {
        // Compute random number between 0 and MAX_X for coord X
        int x = rand() % sim_conf->MAX_X;

        // Compute random number between 0 and MAX_Y for coord Y
        int y = rand() % sim_conf->MAX_Y;

        sources[i].x = x;
        sources[i].y = y;
        sources[i].noise_level = sim_conf->Np;
        sources[i].distance_affected = sim_conf->Dp;
        sources[i].moving_speed = sim_conf->Vp;
    }

    // Noise sources for vehicles
    for (int i = sim_conf->P; i < sim_conf->V + sim_conf->P; i++) {
        // Compute random number between 0 and MAX_X for coord X
        int x = rand() % sim_conf->MAX_X;

        // Compute random number between 0 and MAX_Y for coord Y
        int y = rand() % sim_conf->MAX_Y;

        sources[i].x = x;
        sources[i].y = y;
        sources[i].noise_level = sim_conf->Nv;
        sources[i].distance_affected = sim_conf->Dv;
        sources[i].moving_speed = sim_conf->Vv;
    }
    
    return sim_conf->V + sim_conf->P; // total number of elements added into sources
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

void init_noise_sqm(simulation_config *sim_conf, int ***noise_sqm) {
    *noise_sqm = calloc(sim_conf->MAX_Y, 1+sizeof(int*)); // alloc one extra ptr to check later for NULL on freeing

    for(int i = 0; i < sim_conf->MAX_Y; i++) {
        (*noise_sqm)[i] = calloc(sim_conf->MAX_X, sizeof(int));
    }
    (*noise_sqm)[sim_conf->MAX_Y] = NULL; // set the extra ptr to NULL
}

void compute_noise_sqm(simulation_config *sim_conf, int ***noise_sqm, noise_source *sources, int num_elem) {
    for (int i = 0; i < num_elem; i++) {

        int max_x_increment = sources[i].x - sources[i].distance_affected;
        int min_x_increment = sources[i].x + sources[i].distance_affected;
        int max_y_increment = sources[i].y - sources[i].distance_affected;
        int min_y_increment = sources[i].y + sources[i].distance_affected;

        for (int y = max(0, max_y_increment); y <= min(min_y_increment, sim_conf->MAX_Y - 1); y++) {
            for (int x = max(0, max_x_increment); x <= min(min_x_increment, sim_conf->MAX_X - 1); x++) {
                (*noise_sqm)[y][x] = sum_noises((*noise_sqm)[y][x], sources[i].noise_level);
            }
        }
    }
}

void move_noise_sources(simulation_config *sim_conf, noise_source *sources, int num_elem) {
    for (int i = 0; i < num_elem; i++) {

        int delta_x = sources[i].moving_speed * (rand() % 3 - 1); // Possible values of (rand() % 3 - 1) are -1, 0, 1
        int delta_y = sources[i].moving_speed * (rand() % 3 - 1);

        if (sources[i].x + delta_x >= sim_conf->MAX_X || sources[i].x + delta_x < 0 ) {
            delta_x = -delta_x;
        }

        if (sources[i].y + delta_y >= sim_conf->MAX_Y || sources[i].y + delta_y < 0 ) {
            delta_y = -delta_y;
        }

        sources[i].x += delta_x;
        sources[i].y += delta_y;
    }
}

void reset_matrix(int ***mymatrix, int max_x, int max_y) {
    for (int i = 0; i < max_y; i++) {
        for (int j = 0; j < max_x; j++) {
            (*mymatrix)[i][j] = 0;
        }
    }
}

void free_matrix(int **matrix, int size_y) {
    for (int i = 0; i < size_y; i++) {
        free(matrix[i]);
    }
    free(matrix);
}