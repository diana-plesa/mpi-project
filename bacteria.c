#include <stdio.h>
#include <stdlib.h>

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

int main()
{
    int rows = 0, cols = 0;
    char** grid = get_grid("bacteria.txt", &rows, &cols);
    print_grid(grid, rows, cols);
    return 0;
}