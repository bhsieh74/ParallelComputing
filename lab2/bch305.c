#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <omp.h>

int main(int argc, char *argv[]) {
	//initializing bins and threads and the filename
	unsigned int bins, threads;
	char filename[100] = "";
	//casting as integer
	bins = (unsigned int)atoi(argv[1]);
	threads = (unsigned int)atoi(argv[2]);
	strcpy(filename, argv[3]);
	
	int size;

	//standard File I/O
	FILE *fp;
	fp = fopen(filename, "r");

	//the first number of the text file is the number of floats, so we will extract this number
	//to use to initialize our array of floats
	if (fp != NULL) {
		fscanf(fp, "%d", &size);
	}
	float nums[size];
	//reading in the floats from the textfile into our array of floats
	int temp = 0;
	while (!feof(fp)) {
		fscanf(fp, "%f", &nums[temp]);
		temp = temp + 1;
	}
	//printing for debugging
	//printf("%d\n", bins);
	//printf("%d\n", threads);
	//printf("%s\n", filename);
	//printf("%d\n", size);
	
	//initializing our histogram
	int* histogram;
	histogram = malloc(bins * sizeof(int));
	//setting all values to 0 to prevent garbage values
	for (int i = 0; i < bins; i++) {
		histogram[i] = 0;
	}
	//getting the intervals 
	float divide = 20.00 / bins;

	//creating multiple threads based on the command line input
	#pragma omp parallel num_threads(threads)
	{	
		//each thread will have its own local histogram and we will initialize it to all zeroes to prevent 
		//garbage values
		int local_histogram[threads][bins];
		for (int i = 0; i < threads; i++) {
			for (int j = 0; j < bins; j++) {
				local_histogram[i][j] = 0;
			}
		}

		//getting the id of the current thread
		int id = omp_get_thread_num();
		//splitting the for loop iterations between all of the threads
		#pragma omp for
		for (int i = 0; i < size; i++) {
			float tempnum = nums[i];
			//creating the intervals
			int boolean = 1;
			float lower = 0;
			float upper = divide;
			int bin = 0;
			//while the current number does not fall into the current bin, we will increment
			//the bin by the interval until it falls into a bin
			while (boolean) {
				
				if ((tempnum >= lower) && (tempnum < upper)) {
					local_histogram[id][bin]++;
					boolean = 0;
				}
				else {
					bin++;
					lower = lower + divide;
					upper = upper + divide;
				}
			}
		}
		//merging the local histograms into the final histogram 
		//using atomic keyword for  critical section
		for (int i = 0; i<bins; i++) {
			for (int t = 0; t < threads; t++) {
				#pragma omp atomic
				histogram[i] += local_histogram[t][i];
			
			}
		}
	}
	//printing the bins and their values
	for (int i = 0; i < bins; i++) {
		printf("Bin #%d: %d\n", i, histogram[i]);
	}

	return 0;


}
