// File:    CSVtoBin.c
// Name:    Oscar Huang
// Desc:    Convert csv file to binary file.
// Input:   CSV file 
// Output:  Binary file 
// Usage:   Compiled_program [input csv file location] [output bin file location]  
// Keypoints: Binary file creating

#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "Startup_Dense.h"


int main(int argc, char *argv[]){

    const char* input_file_name = argv[1];
    const char* output_file_name = argv[2];
    
    FILE* in_file =fopen(input_file_name, "r");
    FILE* out_file =fopen(output_file_name, "wb");

    char reading_buffer[128];
    int int_buffer[128];
    char* pt;

    int value;

    for(int i = 0; i < nrow; i++)
    {
        for(int j = 0; j < ncol;j++){
            fscanf(in_file, "%d,", &value);
            fwrite(&value,sizeof(int),1,out_file);
        }

    }

    fclose(in_file);
    fclose(out_file);
    return 0;

}


