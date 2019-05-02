// File:    Matrix_Vector_ReadbyRow.c
// Name:    Oscar Huang
// Desc:    Calculate the matrix vector multiplication. 
//          Each processor (input) load assigned rows from the matrix file. 
//              (Data size in each processor: local_nrow * ncol)
//
//          To practice data communication between processors, the data was re-distributed by MPI_Alltoall.
//          Each processor holds assigned columns after the re-distribution.          
//              (Data size in each processor: nrow * local_ncol)
//
//          Multiply the data with the imported vector (partial).
//              Local imported vector size: locl_ncol
//              Local result size: nrow
//          Use MPI_reduce to sum all the local result.
// Input:   Binary file of dense matrix and vector.
// Usage:   Compiled_program [binary matrix file location] [binary vector file location]  
// Keypoints: Binary file handling / MPI_Alltoallv / MPI_Reduce

#include "stdio.h"
#include "stdlib.h"
#include <mpi.h>
#include "math.h"
#include "Startup_Dense.h"


void unpacked(int* input, int* local_nrow, int* local_ncol, int p, int rank);
int main(int argc, char *argv[]){


    FILE* matrix_file = fopen(argv[1],"r");
    FILE* vector_file = fopen(argv[2], "r");

    int rank, p;
    int* result = malloc(nrow*sizeof(int));


    if (matrix_file==NULL || vector_file==NULL)
    {
        printf("Error reading file\n");
        return 0;
    }

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    double row_section_size = (double)nrow/p; 
    double col_section_size = (double)ncol/p;

    // number of rows and column assigned to each processor
    int* local_nrow=malloc(p*sizeof(int)); 
    int* local_ncol=malloc(p*sizeof(int));

    for(int i=0; i<p; i++)
    {
        local_nrow[i]=(int)(floor((i+1)*row_section_size)-floor(i*row_section_size));
        local_ncol[i]=(int)(floor((i+1)*col_section_size)-floor(i*col_section_size));

    }

    // Load the rows each processor assigned from the binary matrix file
    int* matrix_input = malloc(local_nrow[rank]*ncol*sizeof(int)); 
    fseek(matrix_file,(int)floor(rank*row_section_size)*sizeof(int)*ncol,SEEK_SET);
    fread(matrix_input,sizeof(int),local_nrow[rank]*ncol,matrix_file);
    fclose(matrix_file);

    // Load the corresponding elements from the binary vector file to each processor
    int* vector_input=malloc(local_ncol[rank]*sizeof(int)); 
    fseek(vector_file,(int)floor(rank*row_section_size)*sizeof(int),SEEK_SET);
    fread(vector_input,sizeof(int),local_nrow[rank],vector_file); 
    fclose(vector_file);


    int* sending_buffer = malloc(local_nrow[rank]*ncol*sizeof(int));
    int* receiving_buffer = malloc(local_ncol[rank]*nrow*sizeof(int));


    //Packing the data from MPI_Alltoallv
    for(int i=0; i<ncol; i++) 
        for(int j=0; j<local_nrow[rank];j++)
            sending_buffer[i*local_nrow[rank]+j] = matrix_input[i+j*ncol]; 
    
    free(matrix_input);


    int* send_count = malloc(p*sizeof(int));
    int* receive_count = malloc(p*sizeof(int));
    int* sdispls = calloc(p,sizeof(int));
    int* rdispls = calloc(p,sizeof(int));

    //calculate the sending/receive count and the displs
    for(int i=0; i<p; i++) 
    {
        send_count[i] = local_nrow[rank]*local_ncol[i];
        receive_count[i] = local_nrow[i]*local_ncol[rank];

        if(i>0)
        {
            sdispls[i]=sdispls[i-1]+send_count[i-1];
            rdispls[i]=rdispls[i-1]+receive_count[i-1];
        }
    }


    MPI_Alltoallv(sending_buffer,send_count,sdispls,MPI_INT,receiving_buffer,receive_count,rdispls,MPI_INT,MPI_COMM_WORLD);

    free(sdispls);
    free(rdispls);

    //Unpack the receiving data. The matrix is separated by coulumn now.
    unpacked(receiving_buffer,local_nrow,local_ncol,p,rank); 


    int* local_result = calloc(nrow,sizeof(int));

    //matrix vector multiplication. Each processor holds partial result.
    for(int i=0; i<local_ncol[rank];i++)
        for(int j=0; j<nrow;j++)
            local_result[j]+=receiving_buffer[i*nrow+j]*vector_input[i]; 
    
    free(vector_input);
    free(local_nrow);
    free(local_ncol);
    free(sending_buffer);
    free(receiving_buffer);


    MPI_Reduce(local_result,result, nrow, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    free(local_result);
    
    if(rank==0)
    {
            for(int i=0; i<nrow; i++)
                printf("%d ",result[i]);
        printf("\n");
    }

    MPI_Finalize();  


  return 0;
}



void unpacked(int* input, int* local_nrow, int* local_ncol, int p, int rank)
{
    int* buffer = malloc(local_ncol[rank]*nrow*sizeof(int));

    int e=0;
    int r=0;

    for(int i=0; i<p; i++)
    {
        for(int j=0; j<local_ncol[rank];j++)
            for(int k=0; k<local_ncol[i];k++)
            {                    
                buffer[j*nrow+r+k]= input[e];
                e++;
            }
        r+= local_nrow[i];
    }        

    for(int i=0; i< local_ncol[rank]*nrow; i++)
        input[i]=buffer[i];

    free(buffer);
}