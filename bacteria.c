#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define G 5

char** get_grid(char* filename, int* rows, int* cols)
{
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        fprintf(stderr, "Error opening file");
        exit(-1);
    }

    fscanf(f, "%d%d", rows, cols);

    char** grid = malloc(sizeof(char*) * (*rows));

    for (int i = 0; i < *rows; i++)
    {
        grid[i] = malloc(sizeof(char) * (*cols));
    }

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

void point_reaction(char** grid, int i, int j, int rows, int cols)
{
    int nb_bacteria = get_nb_bacteria_around(grid, i, j, rows, cols);
    if (is_bacteria(grid, i, j) && (nb_bacteria < 2 || nb_bacteria > 3)) bacterium_death(grid, i, j);
    else if (!is_bacteria(grid, i, j) && nb_bacteria == 3) spawn_bacterium(grid, i, j);
}

int main(int argc, char** argv)
{
    printf("Type the number of your chosen running mode: \n1 = NORMAL MODE \n2 = DEBUG MODE\n");
    int mode = 0;
    scanf("%d", &mode);

    if (mode != 1 && mode != 2)
    {
        printf("Invalid mode number!\n");
        return 0;
    }

    printf("Type the desired grid size: 10, 1000, 2000, 3000, 5000\n");
    int grid_size = 0;
    scanf("%d", &grid_size);

    if (grid_size != 10 && grid_size != 1000 && grid_size != 2000 && grid_size != 3000 && grid_size != 5000)
    {
        printf("Invalid grid size!\n");
        return 0;   
    }

    char file_name[20];
    if (grid_size == 10) strcpy(file_name, "bacteria10.txt");
    else if (grid_size == 1000) strcpy(file_name, "bacteria1000.txt");
    else if (grid_size == 2000) strcpy(file_name, "bacteria2000.txt");
    else if (grid_size == 3000) strcpy(file_name, "bacteria3000.txt");
    else if (grid_size == 5000) strcpy(file_name, "bacteria5000.txt");

    int rows = 0, cols = 0;
    char** grid = get_grid(file_name, &rows, &cols);
    print_grid(grid, rows, cols);

    for (int i = 0; i < rows; i++)
    {
        free(grid[i]);
    }
    
    free(grid);

    return 0;
}