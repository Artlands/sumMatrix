#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<mpi.h>

#define ROW 10000
#define COLUMN 10000

int main(int argc, char *argv[]){
    int procid, numprocs;
    int sub_sum[procid];
    int sum = 0;
    clock_t begin, end;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Request reqs[ROW/numprocs];

    if(procid == 0){ 

         //Generate random numbers, stored in array[i][j]
         int array[ROW][COLUMN];
	 int i, j;
         srand(time(NULL));
         printf("\nStart generating...\n");
         for(i = 0; i < ROW; i++){
             for(j = 0; j < COLUMN; j++){
                 array[i][j] = rand()%10; 
                 // printf("%d\t", array[i][j]); 
             }
         }
         printf("Finish generating!\n");

         //MPI_Isend, send data to different processors,Each chunk
         //of data are divided into ROW/numprocs times to send
         begin = clock();
         for(i = 1; i < numprocs; i++){ 
             for(j = 0; j < ROW/numprocs; j++)
             	MPI_Isend(array[i*ROW/numprocs + j], COLUMN, 
               		  MPI_INT, i, j, MPI_COMM_WORLD, &reqs[j]);
         }
 
	 //Sum the first part of data, store result in sub_sum[0]
         for(i = 0; i < ROW/numprocs; i++)
            for(j = 0; j < COLUMN; j++)
                sub_sum[procid] += array[i][j];
         printf("\nThe subset sum in process 0 is %d\n", sub_sum[procid]);

     } else {

	int i,j;
        int sub_array[ROW/numprocs][COLUMN];
	int flag = 0;
        MPI_Status status;

	//MPI_Irecv, each processor receive each row of data by ROW/numprocs times
        for(i = 0; i < ROW/numprocs; i++){
            MPI_Irecv(sub_array[i], COLUMN,
                      MPI_INT, MPI_ANY_SOURCE, i, MPI_COMM_WORLD, &reqs[i]);
        }

        //Do while loop until all rows in this slave are received 
        //Check every row, if it has received the data, do sum calculation,
        //else go to check next row
        while(!flag){ 
	    for(i = 0; i < ROW/numprocs; i++){
	        MPI_Test(&reqs[i], &flag, &status);
	        if(flag){
		    for(j = 0; j < COLUMN; j++)
		        sub_sum[procid] += sub_array[i][j];
	        }
            } 
	}
        printf("\nThe subset sum in process %d is: %d\n",procid, sub_sum[procid]);

	//MPI_Send sub sum to processor 0
        MPI_Send(&sub_sum[procid], 1, MPI_INT, 0, 99, MPI_COMM_WORLD);
     }
    
    if(procid == 0){ 

        MPI_Status status;
        int* sub_sum_buf = (int*)malloc(sizeof(int) * 1) ;
	int i;
	//MPI_Recv data from other processors
        for(i = 1; i != numprocs; ++i){
            MPI_Recv(sub_sum_buf, 1, MPI_INT,
                    MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &status);
            sum += *sub_sum_buf;
        }

        end = clock();
        double time_spent = (double)(end - begin)/CLOCKS_PER_SEC;
        printf("\nThe total calculation time is %f\n", time_spent);
        printf("\nThe total sum of numbers is %d\n", sum + sub_sum[0]);
        }

    MPI_Finalize();

    return 0;
}
