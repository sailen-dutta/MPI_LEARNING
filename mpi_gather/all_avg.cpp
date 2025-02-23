#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <mpi.h>
#include <numeric>

// Creates a vector of random numbers. Each number has a value from 0 - 1
std::vector<float> create_rand_nums(int num_elements){
    std::vector<float> rand_nums(num_elements);
    for (auto& num : rand_nums){
        num = static_cast<float> (rand()) / RAND_MAX;
    }
    return rand_nums;
}

// Computes the average of a vector of numbers
float compute_average(std::vector<float>& array){
    float sum = std::accumulate(array.begin(), array.end(), 0.0f);
    return sum / array.size();
}

int main(int argc, char** argv){
    if (argc != 2){
        std::cerr << "Usage: avg num_elements_per_proc\n";
        return 1;
    }

    int num_elements_per_proc = std::stoi(argv[1]);

    std::srand(std::time(nullptr));

    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    std::vector<float> rand_nums;
    if(world_rank == 0){
        rand_nums = create_rand_nums(num_elements_per_proc * world_size);
    }

    // For each process, create a vector that will hold a subset of the entire array
    std::vector<float> sub_rand_nums(num_elements_per_proc);

    // Scatter the random numbers from the root process to all processes in the MPI world
    MPI_Scatter(rand_nums.data(), num_elements_per_proc, MPI_FLOAT, sub_rand_nums.data(), num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Compute the average of your subset
    float sub_avg = compute_average(sub_rand_nums);

    // Gather all partial averages down to all the processes
    std::vector<float> sub_avgs(world_size);
    MPI_Allgather(&sub_avg, 1, MPI_FLOAT, sub_avgs.data(), 1, MPI_FLOAT, MPI_COMM_WORLD);

    float avgs = compute_average(sub_avgs);

    std::cout << "Avg of all elements from proc " << world_rank << " is " << avgs << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}