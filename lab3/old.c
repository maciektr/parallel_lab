#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

// Uruchomic z pomiarem rewrite_buckets

#define MAX_RAND 60000
// #define MAX_RAND LONG_MAX
typedef unsigned long int COUNT;
typedef unsigned short int VAL;
// typedef unsigned long int VAL;
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// #define WRITE_FILE true
const int max_nt = 10;
const int initial_bucket_size_mul = 2;


VAL *rewrite_bucket(VAL *bucket, int current_size, int new_size) {
    VAL *new_bucket = calloc(new_size, sizeof(VAL));
    memcpy(new_bucket, bucket, current_size * sizeof(*bucket));
    free(bucket);
    return new_bucket;
}

void append(VAL **bucket, int *bucket_size, int *items_count, VAL value) {
    if ((*items_count) + 5 >= (*bucket_size)){
        *bucket = rewrite_bucket(*bucket, *bucket_size, (*bucket_size) * 2);
        *bucket_size = (*bucket_size) * 2;
     }
    (*bucket)[*items_count] = value;
    (*items_count)++;
}

void sort_bucket(VAL *bucket, int bucket_size) {
  int i = 0;
  for (i=1;i<bucket_size; i++) {
        VAL val = bucket[i];

        int j = i - 1;
        while(j >= 0 && bucket[j] > val){
            bucket[j + 1] = bucket[j];
            j--;
        }
        bucket[j + 1] = val;
  }
}

void write_bucket_to_array(VAL *bucket, int bucket_offset, int bucket_size, VAL *arr){
    int place_in_array = bucket_offset;
    for(int j=0; j<bucket_size; j++) {
      arr[place_in_array] = bucket[j];
      place_in_array++;
    }

}

int main(int argc, char **argv) {
  int i, array_size;

  // array_size = atoi(argv[1]);
  array_size = 1000000000;
  // printf("nt; random_gen; put_to_buckets; sort_buckets; bucket_offsets; write_to_array\n");
  for(int nt = 1; nt <= max_nt; nt++){

//   for(array_size = 1000; array_size <= 1000000000; array_size*=10) {
    // const int nt = 1;
  int buckets_num = MAX(array_size / 10, 1);

  // creating array to sort
  VAL *arr = (VAL*)calloc(array_size, sizeof(VAL));

  // creating data structures
  VAL **buckets = (VAL**)calloc(buckets_num, sizeof(VAL*));
  int *bucket_sizes = (int *)calloc(buckets_num, sizeof(int));
  int initial_bucket_size = array_size * initial_bucket_size_mul / buckets_num;
  for(i=0; i<buckets_num; i++) {
    bucket_sizes[i] = initial_bucket_size;
    buckets[i] = (VAL*)calloc(initial_bucket_size, sizeof(VAL));
  }
  int *last_in_bucket = (int*)calloc(buckets_num, sizeof(int));
  int *bucket_offset = (int*)calloc(buckets_num, sizeof(int));
  int *bucket_offset_part = (int*)calloc(buckets_num, sizeof(int));
  omp_lock_t *bucket_lock = (omp_lock_t *)calloc(buckets_num, sizeof(omp_lock_t));
  for (i=0; i<buckets_num; i++)
    omp_init_lock(&(bucket_lock[i]));


  // Write random values to array
  double time_random_gen = omp_get_wtime();
  #pragma omp parallel num_threads(nt) private(i)
  {
    unsigned int seed = (omp_get_thread_num()+1) * omp_get_wtime();
    for(i=omp_get_thread_num();i<array_size;i+=nt)
      arr[i] = rand_r(&seed) % MAX_RAND;
  }
  time_random_gen = omp_get_wtime() - time_random_gen;

  // Log arr values to file
  #ifdef WRITE_FILE
  #if (WRITE_FILE == true)
  FILE *file;
  file = fopen("test.csv", "w+");

  fprintf(file, "before\n");
  for(i=0; i<array_size; i++) {
    fprintf(file, "%d\n", arr[i]);
  }
  #endif
  #endif

  // putting values to buckets
  double time_put_to_buckets = omp_get_wtime();
  int bucket_size = (MAX_RAND + buckets_num - 1) / buckets_num;
  #pragma omp parallel num_threads(nt) private(i)
  {
    for(i=omp_get_thread_num(); i<array_size; i+=nt) {
      int bucket = arr[i] / bucket_size;
      omp_set_lock(&(bucket_lock[bucket]));
      // rewrites bucket if full
      append(&buckets[bucket], &(bucket_sizes[bucket]), &(last_in_bucket[bucket]), arr[i]);
      omp_unset_lock(&(bucket_lock[bucket]));
    }
  }
  time_put_to_buckets = omp_get_wtime() - time_put_to_buckets;

  // Sort in buckets
  double time_sort_buckets = omp_get_wtime();
  #pragma omp parallel num_threads(nt) private(i)
  {
    for (i=omp_get_thread_num(); i<buckets_num; i+=nt)
      sort_bucket(buckets[i], last_in_bucket[i]);
  }
  time_sort_buckets = omp_get_wtime() - time_sort_buckets;

  // Calculate offsets
  double time_bucket_offsets = omp_get_wtime();
  #pragma omp parallel num_threads(nt) private(i)
  {
    for (i=omp_get_thread_num(); i<buckets_num; i+=nt){
      bucket_offset_part[i] = last_in_bucket[i];
      if ((i -nt) >= 0) bucket_offset_part[i] += bucket_offset_part[i-nt];
    }
  }
  #pragma omp parallel num_threads(nt) private(i)
  {
     for (i=omp_get_thread_num(); i<buckets_num; i+=nt){
      bucket_offset[i] = 0;
      for(int k = 0; k < nt ; k++) {
        if ( i - k >= 0) bucket_offset[i] += bucket_offset_part[i - k];
      }
      bucket_offset[i] -= last_in_bucket[i];
    }
  }
  time_bucket_offsets = omp_get_wtime() - time_bucket_offsets;

  // putting values to original array
  double time_write_to_array = omp_get_wtime();
  #pragma omp parallel num_threads(nt) private(i)
  {
    for(i=omp_get_thread_num(); i<buckets_num; i+=nt)
      write_bucket_to_array(buckets[i], bucket_offset[i], last_in_bucket[i], arr);
  }
  time_write_to_array = omp_get_wtime()- time_write_to_array;

  // Write resulting array to file
  #ifdef WRITE_FILE
  #if (WRITE_FILE == true)
  fprintf(file, "\nafter\n");
  for(i=0; i<array_size; i++) {
    fprintf(file, "%d\n", arr[i]);
  }
  fclose(file);
  #endif
  #endif

  // Check if array is sorted
  for (i=1; i<array_size; i++){
    if(arr[i - 1] > arr[i]){
      printf("ERROR: The array is not sorted. %d preceeding %d\n", arr[i - 1], arr[i]);
      return 1;
    }
  }
  printf("%d; %f; %f; %f; %f; %f\n",array_size, time_random_gen, time_put_to_buckets, time_sort_buckets, time_bucket_offsets, time_write_to_array );


  // Free memory
  free(arr);
  free(bucket_sizes);
  free(bucket_offset);
  free(last_in_bucket);
  free(bucket_offset_part);
  for(i=0;i<buckets_num;i++) {
    free(buckets[i]);
    omp_destroy_lock(&(bucket_lock[i]));
  }
  free(buckets);
  free(bucket_lock);
  }

  return 0;
}