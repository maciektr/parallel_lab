#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define WRITE_FILE_IN true
// #define WRITE_FILE_OUT true
const int initial_bucket_size_mul = 2;

#define MAX_RAND 60000
typedef unsigned long int COUNT;
typedef unsigned short int VAL;

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
int bucket_rewrites_count = 0;

VAL *rewrite_bucket(VAL *bucket, int current_size, int new_size) {
    VAL *new_bucket = calloc(new_size, sizeof(VAL));
    memcpy(new_bucket, bucket, current_size * sizeof(*bucket));
    free(bucket);
    bucket_rewrites_count++;
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

void random_array(VAL *array, int array_size, int thread_count){
  int i;
  #pragma omp parallel num_threads(thread_count) private(i)
  {
    unsigned int seed = (omp_get_thread_num()+1) * omp_get_wtime();
    for(i=omp_get_thread_num();i<array_size;i+=thread_count)
      array[i] = rand_r(&seed) % MAX_RAND;
  }
}

void dump_array(VAL *array, int array_size, FILE *file){
  for(int i=0; i<array_size; i++) 
    fprintf(file, "%d\n", array[i]);
}


struct Buckets {
  int buckets_count;
  VAL **buckets;
  int *bucket_sizes;
  int *last_in_bucket;
  omp_lock_t *bucket_locks;
  int *bucket_offset;
};

void free_buckets(struct Buckets buckets) {
  for (int i = 0; i< buckets.buckets_count; i++){
    free(buckets.buckets[i]);
    omp_destroy_lock(&(buckets.bucket_locks[i]));
  }
  free(buckets.buckets);
  free(buckets.bucket_sizes);
  free(buckets.last_in_bucket);
  free(buckets.bucket_locks);
  free(buckets.bucket_offset);
}

void put_values_to_buckets(VAL *array, int array_size, int thread_count, struct Buckets buckets){
  int i;
  int buckets_count = buckets.buckets_count;
  int bucket_size = (MAX_RAND + buckets_count - 1) / buckets_count;
  #pragma omp parallel num_threads(thread_count) private(i) shared(bucket_rewrites_count)
  {
    for(i=omp_get_thread_num(); i<array_size; i+=thread_count) {
      int bucket = array[i] / bucket_size;
      omp_set_lock(&(buckets.bucket_locks[bucket]));
      append(&buckets.buckets[bucket], &(buckets.bucket_sizes[bucket]), &(buckets.last_in_bucket[bucket]), array[i]);
      omp_unset_lock(&(buckets.bucket_locks[bucket]));
    }
  }

}

void create_buckets(struct Buckets *buckets, int array_size, int buckets_count) {
  buckets->buckets_count = buckets_count;
  buckets->buckets = (VAL**)calloc(buckets_count, sizeof(VAL*));
  buckets->bucket_sizes = (int *)calloc(buckets_count, sizeof(int));
  buckets->bucket_locks = (omp_lock_t *)calloc(buckets_count, sizeof(omp_lock_t));
  buckets->last_in_bucket = (int*)calloc(buckets_count, sizeof(int));
  
  int initial_bucket_size = array_size * initial_bucket_size_mul / buckets_count;
  for(int i=0; i<buckets_count; i++) {
    buckets->bucket_sizes[i] = initial_bucket_size;
    buckets->buckets[i] = (VAL*)calloc(initial_bucket_size, sizeof(VAL));
    omp_init_lock(&(buckets->bucket_locks[i]));
  }
  buckets->bucket_offset = (int*)calloc(buckets_count, sizeof(int));
}

void calculate_offsets(struct Buckets *buckets, int array_size, int thread_count) {
  int i;
  int *bucket_offset_part = (int*)calloc(buckets->buckets_count, sizeof(int));
  #pragma omp parallel num_threads(thread_count) private(i)
  {
    for (i=omp_get_thread_num(); i<buckets->buckets_count; i+=thread_count){
      bucket_offset_part[i] = buckets->last_in_bucket[i];
      if ((i - thread_count) >= 0) bucket_offset_part[i] += bucket_offset_part[i-thread_count];
    }
  }
  #pragma omp parallel num_threads(thread_count) private(i)
  {
     for (i=omp_get_thread_num(); i<buckets->buckets_count; i+=thread_count){
      buckets->bucket_offset[i] = 0;
      for(int k = 0; k < thread_count ; k++) {
        if ( i - k >= 0) buckets->bucket_offset[i] += bucket_offset_part[i - k];
      }
      buckets->bucket_offset[i] -= buckets->last_in_bucket[i];
    }
  }
  free(bucket_offset_part);
}

void put_to_array(VAL* array, struct Buckets *buckets, int thread_count){
  int i;
  #pragma omp parallel num_threads(thread_count) private(i)
  {
    for(i=omp_get_thread_num(); i<buckets->buckets_count; i+=thread_count)
      write_bucket_to_array(buckets->buckets[i], buckets->bucket_offset[i], buckets->last_in_bucket[i], array);
  }

}

void check_order(VAL* array, int array_size){
    for (int i=1; i<array_size; i++){
    if(array[i - 1] > array[i]){
      printf("ERROR: The array is not sorted. %d preceeding %d\n", array[i - 1], array[i]);
      exit(EXIT_FAILURE);
    }
  }
}

void sort_buckets(struct Buckets *buckets, int thread_count){
  int i;
  #pragma omp parallel num_threads(thread_count) private(i)
  {
    for (i=omp_get_thread_num(); i<buckets->buckets_count; i+=thread_count)
      sort_bucket(buckets->buckets[i], buckets->last_in_bucket[i]);
  }
}

void measure(int array_size, int nt) {
  int buckets_num = MAX(array_size / 10, 1);

  // creating array to sort
  VAL *arr = (VAL*)calloc(array_size, sizeof(VAL));
  VAL *array = arr;

  // creating data structures
  struct Buckets buckets_store;
  create_buckets(&buckets_store, array_size, buckets_num);

  // Write random values to array
  double time_random_gen = omp_get_wtime();
  random_array(array, array_size, nt);
  time_random_gen = omp_get_wtime() - time_random_gen;

  // Log arr values to file
  #if (defined(WRITE_FILE_IN) && WRITE_FILE_IN == true) || (defined(WRITE_FILE_OUT) && WRITE_FILE_OUT == true)
  char file_name[30];
  sprintf(file_name, "test_%d_%d.csv", array_size, nt);
  FILE *file = fopen(file_name, "w+");
  // FILE *file = fopen("test.csv", "w+");
  #endif

  #if (defined(WRITE_FILE_IN) && WRITE_FILE_IN == true)
  // fprintf(file, "before\n");
  dump_array(array, array_size, file);
  #endif

  // putting values to buckets
  double time_put_to_buckets = omp_get_wtime();
  put_values_to_buckets(array, array_size, nt, buckets_store);
  time_put_to_buckets = omp_get_wtime() - time_put_to_buckets;

  // Sort in buckets
  double time_sort_buckets = omp_get_wtime();
  sort_buckets(&buckets_store, nt);
  time_sort_buckets = omp_get_wtime() - time_sort_buckets;

  // Calculate offsets
  double time_bucket_offsets = omp_get_wtime();
  calculate_offsets(&buckets_store, array_size, nt);
  time_bucket_offsets = omp_get_wtime() - time_bucket_offsets;

  // putting values to original array
  double time_write_to_array = omp_get_wtime();
  put_to_array(array, &buckets_store, nt);
  time_write_to_array = omp_get_wtime()- time_write_to_array;

  // Write resulting array to file
  #if (defined(WRITE_FILE_OUT) && WRITE_FILE_OUT == true)
  fprintf(file, "\nafter\n");
  dump_array(arr, array_size, file);
  #endif

  #if (defined(WRITE_FILE_IN) && WRITE_FILE_IN == true) || (defined(WRITE_FILE_OUT) && WRITE_FILE_OUT == true)
  fclose(file);  
  #endif

  // Check if array is sorted
  check_order(array, array_size);

  printf("%d; %f; %f; %f; %f; %f; %d\n",array_size, time_random_gen, time_put_to_buckets, time_sort_buckets, time_bucket_offsets, time_write_to_array, bucket_rewrites_count );
  bucket_rewrites_count = 0;

  // Free memory
  free(arr);
  free_buckets(buckets_store);
} 

int main(int argc, char **argv) {
  // printf("nt; random_gen; put_to_buckets; sort_buckets; bucket_offsets; write_to_array; rewrite_count\n");
  const int max_thread_count = 8;

  for(int thread_count = 8; thread_count <= max_thread_count; thread_count++)
    for(int array_size = 100000000; array_size <= 1000000000; array_size*=10) 
      measure(array_size, thread_count);

  return 0;
}