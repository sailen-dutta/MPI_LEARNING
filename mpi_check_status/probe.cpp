#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <mpi.h>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != 2){
        std::cerr << "2 processes are required for this example \n";
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        const int MAX_NUMBERS = 100;
        std::vector<int> numbers(MAX_NUMBERS);

        // Seed the random number generator
        std::srand(std::time(nullptr));

        // Pick a random number of integers to send
        int number_amount = static_cast<int>((std::rand() / static_cast<float>(RAND_MAX)) * MAX_NUMBERS);

        // Send the numbers to process 1
        MPI_Send(numbers.data(), number_amount, MPI_INT, 1, 0, MPI_COMM_WORLD);

        std::cout << "Process 0 sent " << number_amount << " numbers to Process 1\n";
    }
    else if (rank == 1){
        MPI_Status status;

        // Probe for the incoming message to get its size
        MPI_Probe(0, 0, MPI_COMM_WORLD, &status);

        // Get the count of integers in the message
        int number_amount;
        MPI_Get_count(&status, MPI_INT, &number_amount);

        // Allocate a buffer to hold the incoming numbers
        auto number_buf = std::make_unique<int[]>(number_amount);

        // Receive the message
        MPI_Recv(number_buf.get(), number_amount, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        // MPI_Recv expects a raw pointer (void*) as the first argument . 
        //Since number_buf is a std::unique_ptr<int[]>, you cannot pass it directly to MPI_Recv.
        // The get() method is used to extract the raw pointer from the std::unique_ptr.

        std::cout << "Process 1 dynamically received " << number_amount << " numbers from Process 0\n";
    }
    MPI_Finalize();
    return 0;
}