#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

char** allocate_grid(int *rows, int *cols)
{
    char** grid = malloc(sizeof(char*) * (*rows));

    for (int i = 0; i < *rows; i++)
    {
        grid[i] = malloc(sizeof(char) * (*cols));
    }

    return grid;
}

char** get_grid(char* filename, int* rows, int* cols)
{
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file");
        exit(-1);
    }

    fscanf(f, "%d%d", rows, cols);

    char** grid = allocate_grid(rows, cols);

    for (int i = 0; i < *rows; i++)
    {
        for (int j = 0; j < *cols; j++)
        {
            fscanf(f, " %c", &grid[i][j]);
        }
    }

    if (fclose(f) != 0)
    {
        fprintf(stderr, "Error opening file");
        exit(-1);
    }

    return grid;
}

void print_grid(char** grid, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
}

int is_bacteria(char** grid, int i, int j)
{
    if (grid[i][j] == 'X') return 1;
    return 0;
}

int get_nb_bacteria_around(char** grid, int src_i, int src_j, int rows, int cols)
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
                if (grid[actual_i][actual_j] == 'X') nb_bacteria++;
            }
        }
    }

    return nb_bacteria;
}

void bacterium_death(char** grid, int i, int j)
{
    grid[i][j] = '.';
}

void spawn_bacterium(char** grid, int i, int j)
{
    grid[i][j] = 'X';
}

void update_grid_point(char** grid, char** next_grid, int i, int j, int rows, int cols)
{
    int nb_bacteria = get_nb_bacteria_around(grid, i, j, rows, cols);
    if (is_bacteria(grid, i, j)) 
    {
        if (nb_bacteria < 2 || nb_bacteria > 3) bacterium_death(next_grid, i, j);
        else next_grid[i][j] = grid[i][j];
    }
    else if (!is_bacteria(grid, i, j)) 
    {
        if (nb_bacteria == 3) spawn_bacterium(next_grid, i, j);
        else next_grid[i][j] = grid[i][j];
    }
}

void free_grid(char** grid, int rows)
{
    for (int i = 0; i < rows; i++)
    {
        free(grid[i]);
    }
    
    free(grid);
}

void write_to_output_file(char** grid, int rows, int cols, char* filename)
{
    FILE *f = fopen(filename, "w");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file");
        exit(-1);
    }

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            fprintf(f, "%c ", grid[i][j]);
        }
        fprintf(f, "\n");
    }

    if (fclose(f) != 0)
    {
        fprintf(stderr, "Error opening file");
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

    // printf("Type the number of your chosen running mode: \n1 = NORMAL MODE \n2 = DEBUG MODE\n");
    // int mode = 0;
    // scanf("%d", &mode);

    // if (mode != 1 && mode != 2)
    // {
    //     printf("Invalid mode number!\n");
    //     return 0;
    // }

    int mode = 1;

    int rows = 0, cols = 0;
    char** grid = get_grid(filename, &rows, &cols);
    char** next_grid = allocate_grid(&rows, &cols);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < generations; i++)
    {
        if (mode == 2)
        {
            printf("====================\nGENERATION %d\n", i+1);
            print_grid(grid, rows, cols);
        }

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                update_grid_point(grid, next_grid, i, j, rows, cols);
            }
        }

        //swap between grids for memory management efficiency
        char** temp = grid;
        grid = next_grid;
        next_grid = temp;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    if (rank == 0)
    {
        printf("Serial time: %f\n", time_taken);
        write_to_output_file(next_grid, rows, cols, "out_parallel.txt");
    }

    free_grid(grid, rows);
    free_grid(next_grid, rows);

    MPI_Finalize();

    return 0;
}