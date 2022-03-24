#!/usr/bin/env python
from mpi4py import MPI
import numpy as np
import os
from sys import argv
import random 


N_POINTS = int(argv[1])


def get_message():
   return os.urandom(MSG_SIZE)
   

def point():
    return (random.random(), random.random())


def point_in_circle(point):
    x, y = point
    return (x * x + y * y) < 1
        

def node(comm):
    res = 0
    node_points = N_POINTS / comm.Get_size()
    i = 0
    while i < node_points:
        if point_in_circle(point()):
            res += 1
        i+=1
    return res



if __name__ == '__main__':
    ROOT_NODE = 0

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()

    comm.Barrier()
    time=MPI.Wtime()

    node_res = node(comm)

    global_sum = 0
    global_sum = comm.reduce(node_res, global_sum, op=MPI.SUM, root=ROOT_NODE)

   
    time=MPI.Wtime()-time

    if rank == ROOT_NODE:
        print global_sum, ';', N_POINTS, ';', 4.0 * float(global_sum)/float(N_POINTS), ';', time 

    MPI.Finalize()
