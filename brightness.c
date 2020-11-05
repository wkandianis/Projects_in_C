#include <stdio.h>
#include <stdlib.h>
#include "pnmrdr.h"

float compute(FILE *ptr);

/*  Function - BRIGHTNESS
 * Arguments: command line args 
 *    Return: int
 *   Purpose: computes the avg brightness of a pgm file 
 */
int main(int argc, char *argv[])
{
        if (argc == 1) {
                float avg = compute(stdin); 
                printf("%.3f\n", avg);
    		exit(EXIT_SUCCESS);
        } 
        
        if (argc == 2) {
                char *filename = argv[1];
                FILE *fptr; 
                fptr = fopen(filename, "rb"); 
    
                if (fptr == NULL) {
                  fprintf(stderr, "Failed to open file.\n");
                        exit(EXIT_FAILURE);
                }

                float avg = compute(fptr); 
                fclose(fptr);
                printf("%.3f\n", avg);
    exit(EXIT_SUCCESS);
        } 
        else if (argc > 2) {
                fprintf(stderr, "Too many arguments.\n");
    exit(EXIT_FAILURE);
        }
}

float compute(FILE *ptr) {

        Pnmrdr_T iptr       = Pnmrdr_new (ptr);
        Pnmrdr_mapdata sptr = Pnmrdr_data (iptr);

        unsigned int width  = sptr.width;  
        unsigned int height = sptr.height; 
        unsigned int denom  = sptr.denominator; 

        float total = height * width;
        float sum   = 0;
        int   i; 

        for (i = 0; i < total; i++) {
                float temp = Pnmrdr_get (iptr);
                temp = temp / denom;
                sum  = sum  + temp; 
        }
        free(iptr);
        float avg = sum / total; 
        return avg;
} 


