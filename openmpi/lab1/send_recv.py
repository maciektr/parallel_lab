#!/usr/bin/env python
from mpi4py import MPI
import numpy as np

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

buf = np.empty((1024,))
MPI.Attach_buffer(buf)

if rank == 0:
   data = ('abc',123)
   comm.bsend(data, dest=1)
elif rank == 1:
   data = comm.recv(source=0)
   print data
else:
   print "Expected only two nodes"
