#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

char* get_grid(char* filename, int* rows, int* cols)
{
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file");
        MPI_Finalize();
        exit(-1);
    }

    fscanf(f, "%d%d", rows, cols);

    char* grid = malloc((*rows) * (*cols) * sizeof(char));

    for (int i = 0; i < *rows; i++)
    {
        for (int j = 0; j < *cols; j++)
        {
            fscanf(f, " %c", &grid[i * (*cols) + j]);
        }
    }

    if (fclose(f) != 0)
    {
        fprintf(stderr, "Error closing file");
        MPI_Finalize();
        exit(-1);
    }

    return grid;
}

void print_grid(char* grid, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%c ", grid[i * cols + j]);
        }
        printf("\n");
    }
}

int is_bacteria(char* grid, int i, int j, int cols)
{
    if (grid[i * cols + j] == 'X') return 1;
    return 0;
}

int get_nb_bacteria_around(char* grid, int src_i, int src_j, int rows, int cols)
{
    int nb_bacteria = 0;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0) continue; //skip source

            int actual_i = i + src_i;
            int actual_j = j + src_j;

            if (actual_i >= 0 && actual_i < rows && actual_j >= 0 && actual_j < cols)
            {
                if (grid[actual_i * cols + actual_j] == 'X') nb_bacteria++;
            }
        }
    }

    return nb_bacteria;
}

void bacterium_death(char* grid, int i, int j, int cols)
{
    grid[i * cols + j] = '.';
}

void spawn_bacterium(char* grid, int i, int j, int cols)
{
    grid[i * cols + j] = 'X';
}

void update_grid_point(char* grid, char* next_grid, int i, int j, int rows, int cols)
{
    int nb_bacteria = get_nb_bacteria_around(grid, i, j, rows, cols);
    if (is_bacteria(grid, i, j, cols)) 
    {
        if (nb_bacteria < 2 || nb_bacteria > 3) bacterium_death(next_grid, i, j, cols);
        else next_grid[i * cols + j] = grid[i * cols + j];
    }
    else if (!is_bacteria(grid, i, j, cols)) 
    {
        if (nb_bacteria == 3) spawn_bacterium(next_grid, i, j, cols);
        else next_grid[i * cols + j] = grid[i * cols + j];
    }
}

void write_to_output_file(char* grid, int rows, int cols, char* filename)
{
    FILE *f = fopen(filename, "w");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file");
        MPI_Finalize();
        exit(-1);
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fprintf(f, "%c ", grid[i * cols + j]);
        }
        fprintf(f, "\n");
    }

    if (fclose(f) != 0)
    {
        fprintf(stderr, "Error closing file");
        MPI_Finalize();
        exit(-1);
    }
}

int main(int argc, char** argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Format: exec filename generations_count\n");
            MPI_Finalize();
            return 0;
        }
    }
    
    char filename[50];
    strcpy(filename, argv[1]);
    int generations = atoi(argv[2]);
    int mode = 0;
    int rows = 0, cols = 0;
    char* grid = NULL;
    char* next_grid = NULL;

    if (rank == 0)
    {
        printf("Type the number of your chosen running mode: \n1 = NORMAL MODE \n2 = DEBUG MODE\n");
        fflush(stdout);
        scanf("%d", &mode);

        if (mode != 1 && mode != 2)
        {
            printf("Invalid mode number!\n");
            MPI_Finalize();
            return 0;
        }

        grid = get_grid(filename, &rows, &cols);
        next_grid = malloc(rows * cols * sizeof(char));
    }

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mode, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int up = 0, down = 0;

    if (rank == 0) up = MPI_PROC_NULL; //dummy MPI rank
    else up = rank - 1; 
    if (rank == size - 1) down = MPI_PROC_NULL;
    else down = rank + 1;

    int rest = rows % size;
    int local_rows = rows / size + (rank < rest ? 1 : 0);

    char* local_grid = malloc((local_rows + 2) * cols * sizeof(char)); //+ 2 for halo rows (upper and lower)
    char* local_next_grid = malloc((local_rows + 2) * cols * sizeof(char));
    
    int *amounts = malloc(size * sizeof(int)); //nb of chars per rank 
    int *indexes = malloc(size * sizeof(int)); //start indexes for each amount of data for each rank 
    int cursor = 0;

    for (int i = 0; i < size; i++) //for each rank
    {
        int rest = rows % size;
        int r = rows / size + (i < rest ? 1 : 0);
        
        amounts[i] = r * cols; //number of characters
        indexes[i] = cursor; //starting location in the global grid
        cursor = cursor + amounts[i]; //move cursor for next index
    }

    MPI_Scatterv(grid, amounts, indexes, MPI_CHAR, &local_grid[cols], //skip top halo row
    local_rows * cols, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    double start = MPI_Wtime();

    for (int g = 0; g < generations; g++)
    {
        //sending top real row to bottom halo of upper rank process
        MPI_Sendrecv(&local_grid[cols], cols, MPI_CHAR, up, 0,
        &local_grid[(local_rows + 1) * cols], cols, MPI_CHAR, down, 0,
        MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //sending bottom real row to upper halo of lower rank process
        MPI_Sendrecv(&local_grid[local_rows * cols], cols, MPI_CHAR, down, 0,
        &local_grid[0], cols, MPI_CHAR, up, 0,
        MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (mode == 2)
        {
            MPI_Gatherv(&local_grid[cols], local_rows * cols, MPI_CHAR, 
                grid, amounts, indexes, MPI_CHAR, 0, MPI_COMM_WORLD);

            if (rank == 0)
            {
                printf("====================\nGENERATION %d\n", g + 1);
                print_grid(grid, rows, cols);
                fflush(stdout);
            }
        }

        for (int i = 1; i <= local_rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                update_grid_point(local_grid, local_next_grid, i, j, local_rows + 2, cols);
            }
        }

        char* temp = local_grid;
        local_grid = local_next_grid;
        local_next_grid = temp;
    }

    MPI_Gatherv(&local_next_grid[cols], local_rows * cols, MPI_CHAR, 
    next_grid, amounts, indexes, MPI_CHAR,
    0, MPI_COMM_WORLD);

    double end = MPI_Wtime();
    double time_taken = end - start;

    if (rank == 0)
    {
        printf("Parallel time: %f\n", time_taken);
        write_to_output_file(next_grid, rows, cols, "f_parallel_out.txt");
        free(grid);
    }

    free(local_grid);
    free(local_next_grid);
    MPI_Finalize();

    return 0;
}