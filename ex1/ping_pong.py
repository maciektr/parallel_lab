#!/usr/bin/env python
from mpi4py import MPI
import numpy as np
import os
from sys import argv


MSG_N = 1000

if argv[1] == 'buff':
   BUFF_SIZE = 84000
else:
   BUFF_SIZE = 0

MSG_SIZE = int(argv[2])

FIRST_NODE = 0
SECOND_NODE = 1

def send(comm, *args, **kwargs):
   if BUFF_SIZE > 0:
      comm.bsend(*args, **kwargs)
   else:
      comm.ssend(*args, **kwargs)


def attach_buffer():
   if BUFF_SIZE > 0:
      buf = np.empty((BUFF_SIZE * MSG_N,))
      MPI.Attach_buffer(buf)


def detach_buffer():
   if BUFF_SIZE > 0:
      MPI.Detach_buffer()


def get_message():
   return os.urandom(MSG_SIZE)
   

def first_node(comm):
   data = get_message()
   for i in range(MSG_N):
      send(comm, data, dest=SECOND_NODE)
      data_rec = comm.recv(source=SECOND_NODE)
      assert data == data_rec


def second_node(comm):
   for i in range(MSG_N):
      data = comm.recv(source=FIRST_NODE)
      send(comm, data, dest=FIRST_NODE)


if __name__ == '__main__':
   comm_world = MPI.COMM_WORLD
   rank = comm_world.Get_rank()

   attach_buffer()
   comm_world.Barrier()
   time=MPI.Wtime()

   if rank == FIRST_NODE:
      first_node(comm_world)
   elif rank == SECOND_NODE:
      second_node(comm_world)
   else:
      print "Expected only two nodes"
   
   time=MPI.Wtime()-time
   detach_buffer()

   if rank == FIRST_NODE:
      capacity = float(MSG_SIZE*MSG_N*8*2)/time
      print 'buffered' if BUFF_SIZE > 0 else 'sync', ';', MSG_SIZE, ';', MSG_N, ';', capacity

   MPI.Finalize()
