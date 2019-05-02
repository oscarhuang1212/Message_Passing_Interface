// File:    P2P_alltoall.c
// Name:    Oscar Huang
// Desc:    Using non-blocking point to point operations (MPI_Isend and MPI_Irecv) to perform MPI_Alltoall function.
// Usage:   Compiled_program [sending_buffer_size]   
//          If the sending_buffer_size was not indicated, the default value will be 5
// Keypoints:   Non-blocking point to point operations

#include "stdio.h"
#include "stdlib.h"
#include <mpi.h>


int main(int argc, char *argv[]){


    int p, rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);


    int sending_size;    
    if(argv[1]==NULL)
        sending_size = 5;
    else
        sending_size = atoi(argv[1]);  

    int buffer_size = p*sending_size;


    int* send_buffer = malloc(buffer_size*sizeof(int));
    int* recv_buffer = malloc(buffer_size*sizeof(int));

    MPI_Request* req = malloc(p*sizeof(MPI_Request));
    MPI_Status* status = malloc(p*sizeof(MPI_Status));

    //initail the sending buffer
    for(int i =0; i<buffer_size; i++)      
        send_buffer[i] = rank*buffer_size+i;

    //using isend and irecv to performce MPI_Alltoall function
    for(int i=0; i<p; i++)
    {
        MPI_Isend(&send_buffer[i*sending_size],sending_size,MPI_INT,i,0,MPI_COMM_WORLD,&req[i]);
        MPI_Irecv(&recv_buffer[i*sending_size],sending_size,MPI_INT,i,0,MPI_COMM_WORLD,&req[i]);
    }
    MPI_Waitall(p,req,status);



    //print the sending_buffer
    /*
        printf("Processor #%d sending_buffer: ",rank);

        for(int i=0; i<buffer_size; i++)
            printf("%d ",send_buffer[i]);
        
        printf("\n");    
    */  

   //print the recv_buffer
    printf("Processor #%d recv_buffer: ",rank);

    for(int i=0; i<buffer_size; i++)
        printf("%d ",recv_buffer[i]);
    
    printf("\n");


    free(req);
    free(status);
    free(send_buffer);
    free(recv_buffer);

    MPI_Finalize();

    return 0;


}