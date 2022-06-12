#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_RAND 1000;

int main (int argc, char **argv) {
  int i, threads, chunk_size, array_size;
  
  chunk_size = atoi(argv[1]);
  array_size = atoi(argv[2]);
  threads = atoi(argv[3]);

  omp_set_num_threads(threads);

  FILE *file;
  file = fopen("results.csv", "a");

  double time_start = omp_get_wtime();

  int *arr = (int*)calloc(array_size, sizeof(int));

  #pragma omp parallel shared(chunk_size, array_size) private(i) 
  {
    unsigned int seed = omp_get_thread_num();

    #pragma omp for schedule(guided, chunk_size) 
    for(i=0; i<array_size; i++) {
      arr[i] = rand_r(&seed) % MAX_RAND;
    }
  }

  double time = omp_get_wtime() - time_start;

  fprintf(file, "guided,%d,%d,%d,%f\n", chunk_size, array_size, threads, time);
  fclose(file);

  return 0;
}