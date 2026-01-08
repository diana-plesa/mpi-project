#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) 
{
    if (argc != 3) 
    {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        return 1;
    }

    FILE *f1 = fopen(argv[1], "r");
    FILE *f2 = fopen(argv[2], "r");

    if (!f1 || !f2) 
    {
        fprintf(stderr, "Error opening file");
        exit(-1);
    }

    int ch1, ch2;
    int match = 1;

    while (1) 
    {
        ch1 = fgetc(f1);
        ch2 = fgetc(f2);
        
        if (ch1 != ch2) 
        {
            match = 0;
            break;
        }

        if (ch1 == EOF) break;
    }

    if (match) printf("Verification successful: files are identical\n");
    else printf("Verification failed: files differ\n");

    if (fclose(f1) != 0 || fclose(f2) != 0)
    {
        fprintf(stderr, "Error closing file");
        exit(-1);
    }
    return 0;
}