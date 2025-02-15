// Example using MPI_Send and MPI_Recv to pass a message around in a ring.

#include <iostream>
#include <mpi.h>
#define m_int MPI_INT  

int main(int argc, char** argv){
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    int rank, size;
    // Get rank and size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int token;
    // Receive from the lower process and send to the higher process. Take care
    // of the special case when you are the first process to prevent deadlock.
    if (rank != 0){
        MPI_Recv(&token, 1, m_int, rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received " << token << " from process " << rank-1 << std::endl;
    }
    else{
        // Set the token's value if you are process 0
        token = -1;
    }

    MPI_Send(&token, 1, m_int, (rank+1)%size, 0, MPI_COMM_WORLD);

    if (rank == 0){
        MPI_Recv(&token, 1, m_int, size-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << " received " << token << " from process " << size-1 << std::endl;
    }

    MPI_Finalize();
   
}