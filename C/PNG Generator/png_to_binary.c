#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *itoa(unsigned long n);
int main(int argc, char **argv)
{
    FILE *image = fopen("download.png", "r");
    FILE *binary_file = fopen("png_to_binary2.txt", "w");
    unsigned char character = fgetc(image);
    int i = 0;
    while (i < 250)
    {
        unsigned char *numString = (char *)calloc(30, sizeof(char));
        unsigned char c = character;
        sprintf(numString, "%u ", c);
        fputs(numString, binary_file);
        character = fgetc(image);
        free(numString);
        i++;
    }
    fclose(image);
    fclose(binary_file);
    return 0;
}