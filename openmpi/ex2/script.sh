#!/bin/bash -l
#SBATCH --nodes 1
#SBATCH --ntasks 12
#SBATCH --time=01:00:00
#SBATCH --partition=plgrid
#SBATCH --account=plgmpr22
#SBATCH --sockets-per-node=2

module add plgrid/tools/openmpi
module spider mpi4py
module add plgrid/libs/python-mpi4py/3.0.0-python-2.7

for points in 1000000 36000000 1280000000 ; do
        for n in 1 2 3 4 5 6 7 8 9 10 11 12 ; do
                echo n $n >> res.txt
                mpiexec -np 12 ./pi.py $points >> res.txt
        done
done
