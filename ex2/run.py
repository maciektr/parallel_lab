#!/usr/bin/env python
from os import system

def cmd(n_processes, size):
   return 'mpiexec -machinefile ./allnodes -np '+str(n_processes)+' ./pi.py ' + str(size) + ' >> res.txt'


sizes = [1000000, 5000000, 10000000, 20000000, 40000000, 80000000, 160000000, 640000000,  1280000000]
for size in sizes:
   for n in range(1,13):
      system('echo n ' +str(n)+' >> res.txt')
      system(cmd(n,size))
