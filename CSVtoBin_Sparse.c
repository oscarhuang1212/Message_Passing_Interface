// File:    CSVtoBin_Sparse.c
// Name:    Oscar Huang
// Desc:    Convert sparse csv file to binary file. 
//          Each node was recorded as (row, col, value) in the binary file
// Input:   CSV file
// Output:  Binary file of transposed matrix. The transposition is to mimic nodes random distributed in the binary file.
// Usage:   Compiled_program [input csv file location] [output bin file location]  
// Keypoints: Sparse binary file creating



#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "Startup_Sparse.h"

int main(int argc, char *argv[]){


    const char* input_file_name = argv[1];
    const char* output_file_name = argv[2];

    FILE* in_file =fopen(input_file_name, "r");
    FILE* out_file =fopen(output_file_name, "wb");

    char* reading_buffer = malloc(ncol*(sizeof(int)+sizeof(char)));
    char* pt;
    int buffer;

    for(int row=0; row<nrow; row++)
    {
        fgets(reading_buffer,ncol*(sizeof(int)+sizeof(char)),in_file);
        pt = strtok(reading_buffer,",");

        for(int col=0; col<ncol; col++)
        {
            if(atoi(pt)!=0)
                {
                    buffer = atoi(pt);
                        fwrite(&col,sizeof(int),1,out_file);//Using transpose to shuffle
                        fwrite(&row,sizeof(int),1,out_file);
                        fwrite(&buffer,sizeof(int),1,out_file);
                }
            pt=strtok(NULL,",");
        }
    }
    
    free(reading_buffer);
    free(pt);
    fclose(in_file);
    fclose(out_file);
    return 0;
}



