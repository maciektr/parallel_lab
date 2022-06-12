#!/bin/bash -l

for array_size in 1000000 10000000 100000000
do
  for threads in {1..4}
  do
    for i in {1..10}
    do
      ./schedule_static $array_size $threads

      ./schedule_dynamic 1 $array_size $threads
      ./schedule_dynamic 1000 $array_size $threads

      chunk_size=`expr $array_size / $threads`
      ./schedule_dynamic $chunk_size $array_size $threads

      ./schedule_guided 1 $array_size $threads
    done
  done
done
