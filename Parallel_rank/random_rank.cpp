#include <iostream>
#include <mpi.h>
#include "tmpi_rank.h"
#include <random>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd() + world_rank); // Seed differently for each process
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float rand_num = dist(gen);
    int rank;
    TMPI_Rank(&rand_num, &rank, MPI_FLOAT, MPI_COMM_WORLD);

    // Synchronize output using MPI_Barrier
    for (int i = 0; i < world_size; ++i) {
        MPI_Barrier(MPI_COMM_WORLD); // Wait for all processes to reach this point
        if (i == world_rank) {
            std::cout << "Rank for " << rand_num << " on process " << world_rank << " - " << rank << std::endl;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
}