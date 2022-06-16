#include<stdio.h>
#include<stdlib.h>
#include "gputimer.h"

// #define N 2048
#define BLOCK_SIZE 32 

__global__ void matrix_transpose_naive(int *input, int *output, int N) {

	int indexX = threadIdx.x + blockIdx.x * blockDim.x;
	int indexY = threadIdx.y + blockIdx.y * blockDim.y;
	int index = indexY * N + indexX;
	int transposedIndex = indexX * N + indexY;

    // this has discoalesced global memory store  
	output[transposedIndex] = input[index];

	// this has discoalesced global memore load
	// output[index] = input[transposedIndex];
}

__global__ void matrix_transpose_shared(int *input, int *output, int N) {

	__shared__ int sharedMemory [BLOCK_SIZE] [BLOCK_SIZE];

	// global index	
	int indexX = threadIdx.x + blockIdx.x * blockDim.x;
	int indexY = threadIdx.y + blockIdx.y * blockDim.y;

	// transposed global memory index
	int tindexX = threadIdx.x + blockIdx.y * blockDim.x;
	int tindexY = threadIdx.y + blockIdx.x * blockDim.y;

	// local index
	int localIndexX = threadIdx.x;
	int localIndexY = threadIdx.y;

	int index = indexY * N + indexX;
	int transposedIndex = tindexY * N + tindexX;

	// reading from global memory in coalesed manner and performing tanspose in shared memory
	sharedMemory[localIndexX][localIndexY] = input[index];

	__syncthreads();

	// writing into global memory in coalesed fashion via transposed data in shared memory
	output[transposedIndex] = sharedMemory[localIndexY][localIndexX];
}

//basically just fills the array with index.
void fill_array(int *data, int N) {
	for(int idx=0;idx<(N*N);idx++)
		data[idx] = idx;
}

void print_output(int *a, int *b, int N) {
	printf("\n Original Matrix::\n");
	for(int idx=0;idx<(N*N);idx++) {
		if(idx%N == 0)
			printf("\n");
		printf(" %d ",  a[idx]);
	}
	printf("\n Transposed Matrix::\n");
	for(int idx=0;idx<(N*N);idx++) {
		if(idx%N == 0)
			printf("\n");
		printf(" %d ",  b[idx]);
	}
}

int main(int argc, char *argv[]) {
	// argv = N ; Type ; Grid size ; Block size
	int N =0;

    if (argc > 1) {
        N = atoi(argv[1]);
    } else {
        printf("Please pass number of elements as command line argument.\n");
        exit(EXIT_FAILURE);
    }

    bool shared = false;
    if (argc > 2 && strcmp(argv[2], "shared") == 0){
		shared = true;
    }

	int the_block_size = BLOCK_SIZE;
	int the_grid_size = N / BLOCK_SIZE;

	if (argc > 3) 
        the_grid_size = atoi(argv[4]);

	if (argc > 4) 
        the_block_size = atoi(argv[3]);

	int *a, *b;
    int *d_a, *d_b; // device copies of a, b, c

	int size = N * N *sizeof(int);

	// Alloc space for host copies of a, b, c and setup input values
	a = (int *)malloc(size); fill_array(a, N);
	b = (int *)malloc(size);

	GpuTimer timer;
    timer.Start();
	
	// Alloc space for device copies of a, b, c
	cudaMalloc((void **)&d_a, size);
	cudaMalloc((void **)&d_b, size);

	// Copy inputs to device
	cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

	dim3 blockSize(the_block_size,the_block_size,1);
	dim3 gridSize(the_grid_size,the_grid_size,1);

	if(!shared)
	matrix_transpose_naive<<<gridSize,blockSize>>>(d_a,d_b, N);
	
	if(shared)
	matrix_transpose_shared<<<gridSize,blockSize>>>(d_a,d_b, N);

	// Copy result back to host
	cudaMemcpy(b, d_b, size, cudaMemcpyDeviceToHost);
	// print_output(a,b);

    cudaFree(d_a);
	cudaFree(d_b); 

    timer.Stop();
	float timer_result = timer.Elapsed();

	// terminate memories
	free(a);
	free(b);

    // printf("size;time;type;grid_size;block_size\n");
    printf("%d, %f, %s, %d, %d\n", N, timer_result, (shared ? "shared" : "naive"), the_grid_size, the_block_size);

	return 0;
}
