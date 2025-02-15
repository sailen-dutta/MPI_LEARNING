// Example of checking the MPI_Status object from an MPI_Recv call

#include <iostream>
#include <ctime>    // For time
#include <cstdlib>  // For srand
#include <mpi.h>

int main(int argc, char** argv){
    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != 2){
        std::cerr << "Must use two processes for this example! \n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const int MAX_NUMBERS = 100;
    int numbers[MAX_NUMBERS];
    int number_amount;
    
    if (rank == 0){
        // Pick a random amount of integers to send to process one
        std::srand(std::time(nullptr));
        number_amount = (rand()/float(RAND_MAX)) * MAX_NUMBERS;

        // Send the amount of integers to process one
        MPI_Send(numbers, number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);
        std::cout << "Process 0 sent " << number_amount << " numbers to process 1 \n";
    }
    else if (rank == 1){
        MPI_Status status;
        // Receive at most MAX_NUMBERS from process zero
        MPI_Recv(numbers, MAX_NUMBERS, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        // After receiving the message, check the status to determine how many numbers were actually received
        MPI_Get_count(&status, MPI_INT, &number_amount);
        // Print off the amount of numbers, and also print additional information in the status object
        std::cout << "Process 1 received " << number_amount << " numbers from 0. ";
        std::cout << "Message source = " << status.MPI_SOURCE << ", ";
        std::cout << "Message tag = " << status.MPI_TAG << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

}