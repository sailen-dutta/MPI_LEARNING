// MPI_Send, MPI_Recv example. Communicates the number -1 from process 0
// to process 1.

#include <iostream>
#include <mpi.h>

int main(int argc, char** argv){
    
    // Initialize MPI environment
    MPI_Init(NULL, NULL);

    // Get rank and size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // We are assuming atleast 2 processes for this task
    if (world_size < 2) {
        std::cerr << "World size must be greater than 1 for " << argv[0] << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);   // terminates all processes in MPI_COMM_WORLD and returns an error code of 1 to the environment.
    } 

    double number = 0;
    if (world_rank == 0){
    // If we are rank 0, set the number to a double and send it to process 1
        number = -1.715;
        MPI_Send(
      /* data         = */ &number, 
      /* count        = */ 1, 
      /* datatype     = */ MPI_DOUBLE, 
      /* destination  = */ 1, 
      /* tag          = */ 0, 
      /* communicator = */ MPI_COMM_WORLD);
    }
    else if (world_rank == 1) {
        MPI_Recv(
      /* data         = */ &number, 
      /* count        = */ 1, 
      /* datatype     = */ MPI_DOUBLE, 
      /* source       = */ 0, 
      /* tag          = */ 0, 
      /* communicator = */ MPI_COMM_WORLD, 
      /* status       = */ MPI_STATUS_IGNORE);
      std::cout << "Process 1 received number " << number << " from process 0" << std::endl;
    }

    MPI_Finalize();

}