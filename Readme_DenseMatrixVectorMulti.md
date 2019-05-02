#Dense Matrix Vector Multiplication

## Description
Calculate the dense matrix vector multiplication.  Each processor input (load) assigned rows from the input file. For practicing purpose, MPI_Alltoallv is used to re-distribute data. After MPI_Alltoallv, each processor holds data with assigned columns. For each processor the corresponded rows in the vector file is loaded. The local multiplication results of processors were further reduced to processor 0.

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
Startup_Dense.h file should includes two parameters: nrow and ncol

eg. 
```
 #define nrow 100
 #define ncol 100
```
&nbsp;&nbsp;&nbsp;&nbsp;**nrow**: number of rows in the input matrix  
&nbsp;&nbsp;&nbsp;&nbsp;**ncol**: number of columnss in the input matrix


##Input files
###matrix.bin
Matrix file should include nrow * ncol intergers(4 bytes).

###vector.bin
Vector file should include nrow intergers(4 bytes).



##Procedure of the program

###Import Matrix File

Each processor import assigned rows in the matrix file.

eg.  
nrow = 100, and number of processor = 5.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 import row 0 to row 19 (20 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 import row 20 to row 39 (20 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 2 import row 40 to row 59 (20 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 3 import row 60 to row 79 (20 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 4 import row 80 to row 99 (20 rows)  
  
eg2.  
nrow = 100, and number of processor = 3.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 import row 0 to row 32 (33 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 import row 33 to row 65 (33 rows)  
&nbsp;&nbsp;&nbsp;&nbsp;processor 2 import row 66 to row 99 (34 rows)  


###Data re-distributed
For practice purpose, the data is re-distributed, and each processor holds assigned column of the input matrix.

eg.  
nrow = 100, ncol = 50, number of processor = 2.   
Before re-distributed:    
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 holds row 0 to row 49.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 holds row 50 to row 99.  

After re-distributed:  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 holds column 0 to column 24.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 holds column 25 to column 49.  

###Import Vector File
Each processor import partial data from vector file.

eg.  
processor 0 holds column 0 to column 24 of the matrix.  
processor 1 holds column 25 to column 49 of the matrix.
  
&nbsp;&nbsp;&nbsp;&nbsp;processor 0 import element 0 to element 24 from the vector file.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 import element 25 to element 49 from the vector file.  


###Mutiplication and reduce
Each processor calculate the local_result, and use MPI_Reduce to get the global result in processor 0. 


eg.  
nrow = 100, ncol = 50 , number of processor = 2  

&nbsp;&nbsp;&nbsp;&nbsp;processor 0 local result has size 100, which has the result of the column 0 to column 24 of input matrix mutiply with  row 0 to row 24 of input vector.  
&nbsp;&nbsp;&nbsp;&nbsp;processor 1 local result has size 100, which has the result of the column 25 to column 49 of input matrix mutiply with  row 25 to row 49 of input vector.

processor 0 has the global result which is the sum of the local result of the processor 0 and the local result of the processor 1.
