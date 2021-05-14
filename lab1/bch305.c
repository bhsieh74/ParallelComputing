// must compile with -std=c99 -Wall -o checkdiv 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]){
  
	unsigned int x, n;
	//unsigned int i; //loop index
	FILE * fp; //for creating the output file
	char filename[100]=""; // the file name

	clock_t start_p1, end_p1, start_p3, end_p3;


	/////////////////////////////////////////
	// start of part 1

	start_p1 = clock();
	// Check that the input from the user is correct.
	if(argc != 3){

    		printf("usage:  ./checkdiv N x\n");
    		printf("N: the upper bound of the range [2,N]\n");
    		printf("x: divisor\n");
    		exit(1);
	}  

	n = (unsigned int)atoi(argv[1]); 
	x = (unsigned int)atoi(argv[2]);
 
	int comm_sz;
	int my_rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	// Process 0 must send the x and n to each process.
	// Other processes must, after receiving the variables, calculate their own range.
	if (my_rank == 0) {
		for (int dest = 1; dest < comm_sz; dest++) {
			MPI_Send(&x, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
			MPI_Send(&n, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
		}
	}
	else {
		MPI_Recv(&x, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//printf("The value of x for process %d is %d\n", my_rank, n);
	}
	

	end_p1 = clock();

	//end of part 1
	/////////////////////////////////////////


	/////////////////////////////////////////
	//start of part 2
	// The main computation part starts here
  	double MPI_Wtime(void);
	double begin, finish;
	begin = MPI_Wtime();
	int start;
	int end;
	int nums = n - 1;
	if (nums % comm_sz == 0) {
		
		int range = nums / comm_sz;
		start = 2 + (range * my_rank);
		end = 2 + (range * my_rank) + (range - 1);
		//printf("%d", start);
		//printf("%d\n", end);
	}
	else {
		int remainder = nums % comm_sz;
		int range = nums / comm_sz;
		if (my_rank == 0) {
			start = 2;
			end = 2 + (range * my_rank) + (range);
			//printf("%d %d in\n", start, end);
		}
		else if ((my_rank < remainder) && (my_rank != 0)) {
			start = 2 + (range * my_rank) + (my_rank);
			end = 2 + (range * my_rank) + range + (my_rank);
			//printf("%d %d middle \n", start, end);
		}
		else {
			start = 2 + (range * my_rank) + remainder;
			end = 2 + (range * my_rank) + remainder + range - 1;
			//printf("%d %d out \n", start, end);
		}
	}	
	int* temparray;
	temparray = malloc(n * sizeof(int));
	int l;
	int *rbuf;
	rbuf = malloc(n * sizeof(int));
	for (l = 0; l < n; l++) {
		temparray[l] = 0;
	}
	int k;
	for (k = 0; k < n; k++) {
		rbuf[k] = 0;
	}

	int i;
	int counter = 0;
	if (my_rank != 0) {
		for (i = start; i <= end; i++) {
			if (i % x == 0) {
				temparray[counter] = i;
				counter = counter + 1;
			}
		}
		MPI_Gather(temparray, (n/comm_sz), MPI_INT, rbuf, (n/comm_sz), MPI_INT, 0, MPI_COMM_WORLD);
		
	}
	else {
		for (i = start; i <= end; i++) {
			if (i % x == 0) {
				temparray[counter] = i;
				counter = counter + 1;
			}
		}
		MPI_Gather(temparray, n/comm_sz, MPI_INT, rbuf, (n/comm_sz), MPI_INT, 0, MPI_COMM_WORLD);
		
		

	}
	finish = MPI_Wtime();
	double max = 0;
	double elapsed_p2 = finish - begin;
	MPI_Reduce(&elapsed_p2, &max, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	

	// end of the main compuation part
	//end of part 2
	/////////////////////////////////////////


	/////////////////////////////////////////
	//start of part 3
	//forming the filename
	if (my_rank == 0) {

		start_p3 = clock();

		strcpy(filename, argv[1]);
		strcat(filename, ".txt");

		if( !(fp = fopen(filename,"w+t"))) {
    			printf("Cannot create file %s\n", filename);
 		   	exit(1);
		}
	
		for (i = 0; i <= n; i++)
    			if (rbuf[i] != 0) {
       		 		fprintf(fp,"%d ", rbuf[i]);
			}

		fclose(fp);

		end_p3 = clock();
		double part1 = ((double)(end_p1 - start_p1)) / CLOCKS_PER_SEC;
		double part3 = ((double)(end_p3 - start_p3)) / CLOCKS_PER_SEC;
		printf("The time elapsed for part 1 is %f seconds.\n", part1);
		printf("The time elapsed for part 2 is %f seconds.\n", max);
		printf("The time elapsed for part 3 is %f seconds.\n", part3);
	}
	//end of part 3
	MPI_Finalize();
	/////////////////////////////////////////
	
	
	return 0;
}
