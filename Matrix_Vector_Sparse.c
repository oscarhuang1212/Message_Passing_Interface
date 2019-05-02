// File:    Matrix_Vector_Sparse.c
// Name:    Oscar Huang
// Desc:    Calculate the multiplication between sparse matrix and dense vector.
//          Each processor read npoints/#processor points.
//          Calculate number of points in each row.
//          Assign rows to each processor. (Each processor holds different number of rows, but similar number of points)
//          Using MPI_Alltoallv to exchance data. Each processor recev the points within assigned rows.
//          Mutiply the nodes with the corresponded elements in the vector, and save the results in local_result
//              Size of local_result: local_nrow
//          Using MPI_Gatherv to collect the local_results from each processor to root processor (processor 0)
// Input:   Binary file of sparse matrix and dense vector.
//          Nodes' format in sparse matrix: (row, column, value)
// Usage:   Compiled_program [binary matrix file location] [binary vector file location]  
// Keypoints:   Binary sparse matrix / MPI_Type_create_struct/ MPI_Allreduce / MPI_AlltoAll
//              MPI_Alltoallv / MPI_Gatherv


#include "stdio.h"
#include "stdlib.h"
#include <mpi.h>
#include "math.h"
#include "Startup_Sparse.h"


//Create a structure for sparse node 
struct node  
{
    int row;
    int col;
    int value;
    
};

int main(int argc, char *argv[]){

    FILE* matrix_file = fopen(argv[1],"r");
    FILE* vector_file = fopen(argv[2], "r");
    
    if (matrix_file==NULL || vector_file==NULL)
    {
        printf("Error reading file\n");
        return 0;
    }

    int rank, p;
    int* row_npoints = calloc(nrow,sizeof(int));
    int* row_npoints_acc = calloc(nrow,sizeof(int));
    int* result = malloc(nrow*sizeof(int));


    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //create MPI_Type: MPI_myNode for sending nodes
    MPI_Datatype MPI_myNode;//myNode
    MPI_Datatype T[1] = {MPI_INT};
    int block[] = {3};
    MPI_Aint disp[]={0};       
    MPI_Type_create_struct(1,block,disp,T, &MPI_myNode);    
    MPI_Type_commit(&MPI_myNode); 
     
    /*
    Practicing create type with different type:
    
    To create a type with {int, double[5], char[2]}:
    
    MPI_Datatype MPI_myNode;
    MPI_Datatype T[3] = {MPI_INT, MPI_DOUBLE, MPI_CHAR};
    int block[3] = {1,5,2};
    
    MPI_Aint disp[3];
    disp[0] = 0;
    disp[1] = disp[0] + sizeof(int)*block[0];
    disp[2] = disp[1] + sizeof(double)*block[1];
    
    MPI_Type_create_struct(3,block,disp,T, &MPI_myNode);    
    MPI_Type_commit(&MPI_myNode); 
    */

    //calculate the number of points to read for each processor
    int local_section_size;   
    local_section_size= floor((rank+1)*npoints/p)-floor(rank*npoints/p); 

    //read information from input file
    struct node* local_input = calloc(local_section_size,(sizeof(struct node)));
    fseek(matrix_file,(int)floor(rank*npoints/p)*sizeof(struct node),SEEK_SET);
    fread(local_input, sizeof(struct node),local_section_size,matrix_file);
    fclose(matrix_file);
    

    //calculate number of points in each row
    for(int i=0; i<local_section_size; i++) 
        row_npoints[local_input[i].row]++;

    //calculate the accumulated number of points in each row
    row_npoints_acc[0]=row_npoints[0];
    
    for(int i=1; i<nrow; i++)
        row_npoints_acc[i]=row_npoints_acc[i-1]+row_npoints[i]; 

    MPI_Allreduce(MPI_IN_PLACE,row_npoints_acc,nrow,MPI_INT,MPI_SUM,MPI_COMM_WORLD);


    //calculate the number of rows each processor should process. For each processor, the number of points processed will be similar.
    int* local_nrow_acc = calloc(p,sizeof(int)); 

    int c = 0;
    for(int j=0; j<nrow; j++)
        if(row_npoints_acc[j]<=(float)(c+1)*npoints/p)
            local_nrow_acc[c]= j;
        else
            c++;
        

    //calculate the sending_count for MPI_Alltoallv
    int* input_sending_count = calloc(p,sizeof(int)); 

    for(int i=0; i<local_section_size; i++)
    {
        if(local_input[i].row<=local_nrow_acc[0])
                input_sending_count[0]++;
    
        else
            for(int j=1; j<p; j++)
                    if(local_input[i].row<=local_nrow_acc[j] && local_input[i].row>local_nrow_acc[j-1])
                            input_sending_count[j]++;
    }

    //calculate the sending_displs for MPI_Alltoallv
    int* input_sending_count_acc = calloc(p, sizeof(int)); 

    for(int i=1; i<p; i++)
        input_sending_count_acc[i]=input_sending_count_acc[i-1]+input_sending_count[i-1];


    struct node* input_sorted = calloc(local_section_size,sizeof(struct node));
    int* sorting_count = calloc(p,sizeof(int));

    //sort the local_input nodes depends on the row
    for(int i=0; i<local_section_size; i++) 
    {
        if(local_input[i].row<=local_nrow_acc[0])
            {
                input_sorted[sorting_count[0]]=local_input[i];
                sorting_count[0]++;
            }
        else
            {
                for(int j=1; j<p; j++)
                    if(local_input[i].row<=local_nrow_acc[j] && local_input[i].row>local_nrow_acc[j-1])
                        {
                            input_sorted[input_sending_count_acc[j]+sorting_count[j]]=local_input[i];
                            sorting_count[j]++;
                        }
            }
    }
    
    free(sorting_count);

  
    //Calculate the recv buffer size // Could be optimized by avoiding this
    int* recv_size = malloc(p*sizeof(int)); 
    MPI_Allreduce(input_sending_count,recv_size,p,MPI_INT,MPI_SUM,MPI_COMM_WORLD); 


    //Calculate the recv count for the MPI_Alltoallv
    int* recv_count = malloc(p*sizeof(int)); 
    MPI_Alltoall(input_sending_count,1,MPI_INT,recv_count,1,MPI_INT,MPI_COMM_WORLD);

    //Calculate the recv displs for the MPI_Alltoallv
    int* recv_count_acc = calloc(p,sizeof(int)); 
    for(int i=1; i<p;i++)
        recv_count_acc[i]=recv_count_acc[i-1]+recv_count[i-1];

    struct node* recv = malloc(recv_size[rank]*sizeof(struct node));


    //sending and receiving nodes between processor
    MPI_Alltoallv(input_sorted,input_sending_count,input_sending_count_acc,MPI_myNode,recv,recv_count,recv_count_acc,MPI_myNode,MPI_COMM_WORLD);


    //import vector file to all processors
    int* vector_input=malloc(nrow*sizeof(int)); 
    fread(vector_input,sizeof(int),ncol,vector_file);
    fclose(vector_file);

    int* local_nrow =  calloc(p,sizeof(int));
    
    //calculate sending count and recvdispls 
    for(int i=p-1; i>0; i--) 
    {
        local_nrow[i] = local_nrow_acc[i] -local_nrow_acc[i-1];
        local_nrow_acc[i]=local_nrow_acc[i-1]+1;
    }
    local_nrow[0]=local_nrow_acc[0];
    local_nrow_acc[0]=0;


    int* local_result = calloc(local_nrow[rank],sizeof(int)); 
    
    //Multiplication
    for(int i=0; i<recv_size[rank]; i++)
        local_result[recv[i].row-local_nrow_acc[rank]] += recv[i].value*vector_input[recv[i].col]; 

    
    //Gathering local result to processor 0
    MPI_Gatherv(local_result,local_nrow[rank],MPI_INT,result,local_nrow,local_nrow_acc,MPI_INT,0,MPI_COMM_WORLD);


    if(rank==0)
        for(int i=0; i<nrow; i++)
        {
            printf("%d ", result[i]);
            if(i%10==9)
                printf("\n");

        }

    MPI_Finalize();

    free(local_input);
    free(row_npoints);
    free(row_npoints_acc);
    free(local_nrow_acc);
    free(input_sending_count);
    free(input_sending_count_acc);
    free(input_sorted);
    free(recv);
    free(recv_count_acc);
    free(recv_count);
    free(recv_size);
    free(vector_input);
    free(local_result);
    free(result);
    return 0;
}