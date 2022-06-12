#!/usr/bin/env python
from os import system

sizes = [1]
for _ in range(13):
   sizes.append(sizes[-1] * 2)
for _ in range(3):
   sizes.append(sizes[-1] + sizes[13])


cmd = 'mpiexec -machinefile ./allnodes -np 2 ./ping_pong.py'


for send_type in ['buff', 'sync']:
   for size in sizes:
      system(cmd + ' ' + send_type  + ' ' + str(size) +' >> res.txt')
