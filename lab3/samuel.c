#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_RAND 100000000
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void fill_array(int *arr, int size) {
  int i;

  #pragma omp parallel private(i)
  {
    unsigned int seed = omp_get_thread_num();

    #pragma omp for schedule(guided, 1) 
    for(i=0; i<size; i++) {
      arr[i] = rand_r(&seed) % MAX_RAND;
    }
  }
}

void maybe_alloc_bigger_bucket(int **buckets, int *buckets_sizes, int *last_in_bucket, int bucket) {
  if (last_in_bucket[bucket] >= buckets_sizes[bucket]) {
    int *new_bucket = (int*)calloc(2 * buckets_sizes[bucket], sizeof(int));
    buckets_sizes[bucket] *= 2;

    int i;
    for(i=0; i<last_in_bucket[bucket]; i++) {
      new_bucket[i] = buckets[bucket][i];
    }

    int *old_bucket = buckets[bucket];
    buckets[bucket] = new_bucket;
    free(old_bucket);
  }
}

void sort(int *arr, int array_size, int **buckets, int *buckets_sizes, int *last_in_bucket, int threads, int bucket_range, int thread_buckets) {
  int i, j;

  #pragma omp parallel private(i, j)
  {
    int thread_index = omp_get_thread_num();
    int thread_first_bucket = thread_buckets * thread_index;

    for(i=0; i<array_size; i++) {
      int index = (array_size / threads * thread_index + i) % array_size;

      int val = arr[index];
      int bucket = val / bucket_range;

      if (bucket >= thread_first_bucket && bucket < thread_first_bucket + thread_buckets) {
        // makes bucket two times bigger if it is too small
        maybe_alloc_bigger_bucket(buckets, buckets_sizes, last_in_bucket, bucket);

        buckets[bucket][last_in_bucket[bucket]] = val;

        j = last_in_bucket[bucket] - 1;
        while(j >= 0 && buckets[bucket][j] > val){
            buckets[bucket][j + 1] = buckets[bucket][j];
            j--;
        }
        buckets[bucket][j + 1] = val;

        last_in_bucket[bucket]++;
      }
    }
  }
}

void calculate_values_per_thread(int *values_numbers, int *last_in_bucket, int *bucket_first_index, int buckets_number, int thread_buckets) {
  int i;

  #pragma omp parallel private(i)
  {
    int thread_index = omp_get_thread_num();
    int upper_bound = thread_buckets * (thread_index + 1);

    if(upper_bound > buckets_number) {
      upper_bound = buckets_number;
    }

    for(i=thread_buckets * thread_index + 1; i < upper_bound; i++) {
      bucket_first_index[i] = bucket_first_index[i-1] + last_in_bucket[i-1];
    }

    values_numbers[thread_index] = bucket_first_index[upper_bound-1] + last_in_bucket[upper_bound-1];
  }
}

void put_back_to_array(int *arr, int array_size, int **buckets, int *values_numbers, int *last_in_bucket, int thread_buckets, int buckets_number) {
  int i, j;

  #pragma omp parallel private(i, j)
  {
    int thread_index = omp_get_thread_num();
    int thread_first_bucket = thread_buckets * thread_index;

    int place_in_array = 0;
    for(i=0; i<thread_index; i++) {
      place_in_array += values_numbers[i];
    }

    for(i=thread_first_bucket; i<thread_first_bucket + thread_buckets && i<buckets_number; i++) {
      for(j=0; j<last_in_bucket[i]; j++) {
        arr[place_in_array] = buckets[i][j];
        place_in_array++;
      }
    }
  }
}

int check_if_sorted(int *arr, int size) {
  int i;
  int sorted = 1;

  for(i=1; i<size; i++) {
    if(arr[i] < arr[i-1]) {
      sorted = 0;
      break;
    }
  }

  return sorted;
}
  FILE *file;

// int main (int argc, char **argv) {
int run(int array_size, int threads) {
  int i, buckets_number;
//   int array_size, threads;
  
  // parameters
//   buckets_number = atoi(argv[2]);

//   array_size = atoi(argv[1]);
//   threads = atoi(argv[2]);

  buckets_number = MAX(array_size / 10, 1);

  // shared variables
  int bucket_size = 2 * array_size / buckets_number;
  int bucket_range = (MAX_RAND + buckets_number - 1) / buckets_number;
  int thread_buckets = (buckets_number + threads - 1) / threads;

  // arrays
  int *arr = (int*)calloc(array_size, sizeof(int));

  int **buckets = (int**)calloc(buckets_number, sizeof(int*));
  int *buckets_sizes = (int*)calloc(buckets_number, sizeof(int));
  for(i=0; i<buckets_number; i++) {
    buckets[i] = (int*)calloc(bucket_size, sizeof(int));
    buckets_sizes[i] = bucket_size;
  }

  int *last_in_bucket = (int*)calloc(buckets_number, sizeof(int));
  int *bucket_first_index = (int*)calloc(buckets_number, sizeof(int));
  int *values_numbers = (int*)calloc(threads, sizeof(int));

  // time measurement
  double time_start, time_after_draw, time_after_sort, time_end;

  // file for results

  // setting number of threads
  omp_set_num_threads(threads);

  // start time of the calculations
  time_start = omp_get_wtime();

  // putting random values to the array
  fill_array(arr, array_size);

  // time after filling the array
  time_after_draw = omp_get_wtime();

  // Putting values to buckets. Used insertion sort, so buckets are always sorted.
  sort(arr, array_size, buckets, buckets_sizes, last_in_bucket, threads, bucket_range, thread_buckets);

  // timestamp after sorting
  time_after_sort = omp_get_wtime();

  // calculating number of numbers in all thread's buckets and put them to `values_numbers` array
  calculate_values_per_thread(values_numbers, last_in_bucket, bucket_first_index, buckets_number, thread_buckets);
  
  // using `values_numbers` to determine start index in the array and putting values from buckets to the array
  put_back_to_array(arr, array_size, buckets, values_numbers, last_in_bucket, thread_buckets, buckets_number);

  // time of the end of algorithm
  time_end = omp_get_wtime();

  // checking if the array is sorted
  int sorted = check_if_sorted(arr, array_size);
  if (sorted == 0)
    exit(EXIT_FAILURE);

  // writing measurements to the file
//   fprintf(file, "%d %d %f %f %f %f", threads, array_size, time_after_draw - time_start, time_after_sort - time_after_draw, time_end - time_after_sort, time_end - time_start);
  printf("%d %d %f %f %f %f\n", threads, array_size, time_after_draw - time_start, time_after_sort - time_after_draw, time_end - time_after_sort, time_end - time_start);
  
  // writing special symbol at the end of invalid measurements
//   if(sorted == 0) {
//     fprintf(file, " X\n");
//   } else {
//     fprintf(file, "\n");
//   }

  // closing result file

  // freeing memory
  free(arr);

  for(i=0; i<buckets_number; i++) {
    free(buckets[i]);
  }

  free(buckets);
  free(buckets_sizes);

  free(last_in_bucket);
  free(bucket_first_index);
  free(values_numbers);

  return 0;
}

int main() {
    int res =0;

    const int repeat = 1;
    const int max_thread_count = 10;
    const int max_array_size = 1000000000;
    // const int max_array_size = 1000000;
    const int min_array_size = 1000;
    const int array_size_step = 100;

    // file = fopen("results_samuel.csv", "a+");
    // fprintf(file, "thread_count, array_size, random_gen, put_to_buckets, write_to_array, all\n");
    printf("thread_count, array_size, random_gen, put_to_buckets, write_to_array, all\n");

    for (int n = 0; n<repeat ; n++)
        for(int thread_count = 1; thread_count <= max_thread_count; thread_count++)
            for(int array_size = min_array_size; array_size <= max_array_size; array_size*=array_size_step) 
                res += run(array_size, thread_count);

    // fclose(file);
    return res;
}