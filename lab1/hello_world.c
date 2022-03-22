#include <stdio.h>
#include <mpi.h>

int main (int argc, char * argv[])
{
  int rank, size;

  MPI_Init (&argc, &argv);  /* starts MPI */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);  /* get current process id */
  MPI_Comm_size (MPI_COMM_WORLD, &size);  /* get number of processes */
  char hostname[MPI_MAX_PROCESSOR_NAME];
  int len = MPI_MAX_PROCESSOR_NAME;
  MPI_Get_processor_name(hostname, &len);
  printf( "Hello world from process %d of %d, at node %s\n", rank, size, hostname);
  MPI_Finalize();
  return 0;
}
