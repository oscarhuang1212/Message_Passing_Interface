#Sparse Matrix Vector Multiplication

## Description
Calculate the sparse matrix vector multiplication.  Each processor input (load) assigned number of points from the input matrix file. For each processor, the number of points in each row is counted and saved in row_npoints. The row_npoints is reduced and broadcasted by MPI_Reduceall to all processor. The nodes from the input matrix is re-distributed based on the row of the node. The vector is loaded to every processor, and the local result is calculated in each processor. The local results are gathered by MPI_Gatherv for the final result.

##Usage   
Compiled_program [binary matrix file location] [binary vector file location]  

eg.
```
a.out Bin/M1.bin Bin/V1.bin
```
&nbsp;&nbsp;&nbsp;&nbsp;**a.out**: Compiled file name  
&nbsp;&nbsp;&nbsp;&nbsp;**Bin/M1.bin**: location of binary matrix file  
&nbsp;&nbsp;&nbsp;&nbsp;**Bin/V1.bin**: location of binary vector file  


##Header files
###Startup_Dense.h
Startup_Dense.h file should includes two parameters: nrow, ncol, and npoints

eg. 
```
#define nrow 100
#define ncol 100
#define npoints 999
```
&nbsp;&nbsp;&nbsp;&nbsp;**nrow**: number of rows in the input matrix  
&nbsp;&nbsp;&nbsp;&nbsp;**ncol**: number of columnss in the input matrix
&nbsp;&nbsp;&nbsp;&nbsp;**npoints**: number of nodes in the input matrix


##Input files
###matrix.bin
Input matrix include npoints nodes, each nodes is represents as three integers: row, column, and value (3 * 4 bytes)

###vector.bin
Vector file should include nrow intergers(4 bytes).


##Structure 
###node
The node structure represent each node in the sparse matrix. The node with value *value* is at location row: *row*, and column: *col*.

```
struct node
{
    int row;
    int col;
    int value;
};
```


##Procedure of the program

###Import Matrix File

Each processor import assigned points in the matrix file.

eg.  
npoint = 100, and number of processor = 5.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 import node 0 to node 19 (20 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 import node 20 to node 39 (20 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 2 import node 40 to node 59 (20 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 3 import node 60 to node 79 (20 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 4 import node 80 to node 99 (20 nodes)  
  
eg2.  
npoint = 100, and number of processor = 3.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 import node 0 to node 32 (33 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 import node 33 to node 65 (33 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 2 import node 66 to node 99 (34 nodes)  


###Data re-distributed
Based on the number of nodes in each rows, the data is re-distrubuted for load balanced. 

eg.  
number of processors = 2  
row 0 to 9 has 50 nodes  
row 10 to 99 has 50 nodes  
After re-distributed:    
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 holds row 0 to row 9. (50 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 holds row 10 to row 99. (50 nodes)  

eg2.  
number of processors = 2  
row 0 to 9 has 45 points  
row 10 has 10 points  
row 99 has 45 points  
After re-distributed:    
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 holds row 0 to row 10. (55 nodes)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 holds row 11 to row 99. (45 nodes)  

###Import Vector File
Import whole vector to each processor


###Mutiplication and gather
Each processor calculate the local_result, and use MPI_Gatherv to get the global result in processor 0. 


eg.  
number of processors = 2  
processor 0 holds row 0 to row 9. (50 nodes)  
processor 1 holds row 10 to row 99. (50 nodes)  

After multiplication:  
&nbsp;&nbsp;&nbsp;&nbsp;  processor 0 holes local result with size 10 (row 0 to row 9)
&nbsp;&nbsp;&nbsp;&nbsp;  processor 1 holes local result with size 90 (row 10 to row 99)

processor 0 will gather the local result from all processors (10 integer from processor 0, and 90 integer from processor 1), and get the global result with size 100.
