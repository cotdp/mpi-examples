#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
   char idstr[32], buff[128];
   int numprocs, rank, namelen, i;
   char processor_name[MPI_MAX_PROCESSOR_NAME];

   MPI_Status stat;
   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Get_processor_name(processor_name, &namelen);

   // Based on example from https://wiki.inf.ed.ac.uk/pub/ANC/ComputationalResources/slides.pdf
   if (rank == 0) {
      // This is the rank-0 copy of the process
      printf("We have %d processors\n", numprocs);
      // Send each process a "Hello ... " string
      for(i = 1; i < numprocs; i++) {
         sprintf(buff, "Hello %d... ", i);
         MPI_Send(buff, 128, MPI_CHAR, i, 0, MPI_COMM_WORLD);
      }
      // Go into a blocking-receive for each servant process
      for(i = 1; i < numprocs; i++) {
         MPI_Recv(buff, 128, MPI_CHAR, i, 0, MPI_COMM_WORLD, &stat);
         printf("%s\n", buff);
      }
   } else {
      // Go into a blocking-receive waiting
      MPI_Recv(buff, 128, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);
      // Append our identity onto the received string
      sprintf(idstr, "Processor %d ", rank);
      strcat(buff, idstr);
      strcat(buff, "reporting!");
      // Send the string back to the rank-0 process
      MPI_Send(buff, 128, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();

   return 0;
}

