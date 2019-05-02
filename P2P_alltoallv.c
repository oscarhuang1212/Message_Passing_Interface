// File:    P2P_alltoallv.c
// Name:    Oscar Huang
// Desc:    Using non-blocking point to point operations (MPI_Isend and MPI_Irecv) to perform MPI_Alltoallv function.
//
//          The program is designed for 4 processors.
//          The sending buffer:
//              Processor 0: 0 1 2 3 4 5 6 7 8 9 10 
//              Processor 1: 100 101 102 103 104  105 106 107 108 109 110
//              Processor 2: 200 201 202 203 204
//              Processor 3: 300 301 302 303 304 305 306 307 308 309 310 311 312 313
//
//          The sending count:
//              Processor 0: 4 2 3 2
//              Processor 1: 2 4 1 4 
//              Processor 2: 1 1 1 2
//              Processor 3: 5 3 2 4
//
// Keypoints:   Non-blocking point to point operations 



#include "stdio.h"
#include "stdlib.h"
#include <mpi.h>



int main(int argc, char *argv[]){


    int p, rank;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int* send_count = malloc(p*sizeof(int));
    int* recv_count = malloc(p*sizeof(int));

    MPI_Request* req_count = malloc(p*sizeof(MPI_Request));
    MPI_Status* status_count = malloc(p*sizeof(MPI_Status));
    
    MPI_Request* req = malloc(p*sizeof(MPI_Request));
    MPI_Status* status = malloc(p*sizeof(MPI_Status));


//Initiate the sending count
{
    if(rank==0)
        {
            send_count[0]=4;
            send_count[1]=2;
            send_count[2]=3;
            send_count[3]=2;
        }
    else if(rank==1)
        {
            send_count[0]=2;
            send_count[1]=4;
            send_count[2]=1;
            send_count[3]=4;
        }
    else if(rank==2)
        {
            send_count[0]=1;
            send_count[1]=1;
            send_count[2]=1;
            send_count[3]=2;
        }
    else 
        {
            send_count[0]=5;
            send_count[1]=3;
            send_count[2]=2;
            send_count[3]=4;
        }
}


// Alltoall: Sending count --> Recv count
    for(int i=0; i<p; i++)
        {
            MPI_Isend(&send_count[i],1,MPI_INT,i,0,MPI_COMM_WORLD,&req_count[i]);
            MPI_Irecv(&recv_count[i],1,MPI_INT,i,0,MPI_COMM_WORLD,&req_count[i]);
        }
    MPI_Waitall(p, req_count, status_count);



    int send_buffer_size=0;
    int recv_buffer_size=0;

    int* sdispls = calloc(p,sizeof(int));
    int* rdispls = calloc(p,sizeof(int));


    //Calculate the send/recv buffer size and displacements 
    for(int i=0; i<p; i++)
    {
        send_buffer_size+=send_count[i];
        recv_buffer_size+=recv_count[i];

        if(i>0)
        {
            sdispls[i] = sdispls[i-1]+send_count[i-1];
            rdispls[i] = rdispls[i-1]+recv_count[i-1];
        }
    }

    int* send_buffer = malloc(send_buffer_size*sizeof(int));
    int* recv_buffer = malloc(recv_buffer_size*sizeof(int));
    

    //Initiate the value in the sending buffer
    for(int i=0; i<send_buffer_size; i++)
        send_buffer[i] = rank*100+i;

    //Alltoallv
    for(int i=0; i<p; i++)
        {
            MPI_Isend(&send_buffer[sdispls[i]],send_count[i],MPI_INT,i,0,MPI_COMM_WORLD,&req[i]);
            MPI_Irecv(&recv_buffer[rdispls[i]],recv_count[i],MPI_INT,i,0,MPI_COMM_WORLD,&req[i]);
        }

    MPI_Waitall(p, req, status);



    printf("Processor %d recv: ",rank);
    for(int i=0; i<recv_buffer_size; i++)
        printf("%d ", recv_buffer[i]);
    printf("\n");
    

    free(send_count);
    free(recv_count);

    free(req_count);
    free(status_count);

    free(req);
    free(status);

    free(sdispls);
    free(rdispls);

    free(send_buffer);
    free(recv_buffer);

    MPI_Finalize();

    return 0;


}