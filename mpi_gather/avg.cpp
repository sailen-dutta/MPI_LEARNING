/* Program that computes the average of an array of elements in parallel using MPI_Scatter and MPI_Gather */

#include <iostream>
#include <vector>
#include <random>
#include <numeric>
#include <mpi.h>

std::vector<float> create_random_numbers(int num_elements){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    std::vector<float> rand_nums(num_elements);
    for (float& num : rand_nums){
        num = dis(gen);
    }
    return rand_nums;
}

float compute_average(const std::vector<float>& array){
    float sum = std::accumulate(array.begin(), array.end(), 0.0f);
    return sum / array.size();
}

int main(int argc, char** argv){

    if (argc != 2){
        std::cerr << "Usage: avg num_elements_per_proc \n";
        return 1;
    }

    int num_elements_per_proc = std::stoi(argv[1]);

    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Create a random array of elements on the root process
    std::vector<float> rand_nums;
    if (world_rank == 0){
        rand_nums = create_random_numbers(num_elements_per_proc * world_size);
        std::cout << "Random numbers generated on root process: " << std::endl;
        for (auto it = rand_nums.begin(); it != rand_nums.end(); ++it){
            std::cout << "rand_nums[" << std::distance(rand_nums.begin(), it) << "] = " << *it << std::endl;
        }
    }

     // For each process, create a buffer that will hold a subset of the entire array
    std::vector<float> sub_rand_nums(num_elements_per_proc);

    // Scatter the random numbers from the root process to all processes
    MPI_Scatter(rand_nums.data(), num_elements_per_proc, MPI_FLOAT, 
    sub_rand_nums.data(), num_elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Compute the average of your subset
    float sub_average = compute_average(sub_rand_nums);

    // Gather all partial averages down to the root process
    std::vector<float> sub_averages;
    if (world_rank == 0){
        sub_averages.resize(world_size);
    }

    MPI_Gather(&sub_average, 1, MPI_FLOAT, sub_averages.data(), 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Compute the total average on the root process
    if (world_rank == 0){
        float average = compute_average(sub_averages);
        std::cout << "Average of all elements is: "<< average << std::endl;

        // Compute the average across the original data for comparison
        float original_data_average = compute_average(rand_nums);
        std::cout << "Average of original data is: " << original_data_average << std::endl;
    }

    MPI_Finalize();
    return 0;
}
