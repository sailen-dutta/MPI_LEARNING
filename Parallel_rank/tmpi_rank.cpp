#include <iostream>
#include <mpi.h>
#include <vector>
#include <algorithm>
#include <memory>
#include "tmpi_rank.h"
#include <cstring>

// Class to hold the communicator rank and the corresponding number
class CommRankNumber{
    private:
        int comm_rank;
        union{
            float f;
            int i;
        } number;

    public:
        // Constructor to initialize the object
        CommRankNumber(int rank, const void* value, MPI_Datatype datatype): comm_rank(rank){
            if (datatype == MPI_FLOAT){
                std::memcpy(&number.f, value, sizeof(float));
            }
            else if(datatype == MPI_INT){
                std::memcpy(&number.i, value, sizeof(int));
            }
        }

        // Getter for the communicator rank
        int get_comm_rank() const{
            return comm_rank;
        }

        float get_float_value() const{
            return number.f;
        }

        float get_int_value() const{
            return number.i;
        }

        // Comparison function for sorting (float)
        static bool compare_float(const CommRankNumber& a, const CommRankNumber& b){
            return a.number.f < b.number.f;
        }

        // Comparison function for sorting (int)
        static bool compare_int(const CommRankNumber& a, const CommRankNumber& b){
            return a.number.i < b.number.i;
        }
};

std::vector<char> gather_numbers_to_root(const void* number, MPI_Datatype datatype, MPI_Comm comm){
    int comm_rank, comm_size;
    MPI_Comm_rank(comm, &comm_rank);
    MPI_Comm_size(comm, &comm_size);

    int datatype_size;
    MPI_Type_size(datatype, &datatype_size);

    std::vector<char> gathered_numbers;
    if(comm_rank == 0){
        gathered_numbers.resize(datatype_size * comm_size);
    }

    MPI_Gather(number, 1, datatype, gathered_numbers.data(), 1, datatype, 0, comm);

    return gathered_numbers;
}

std::vector<int> get_ranks(const std::vector<char>& gathered_numbers, int gathered_number_count, MPI_Datatype datatype){
    int datatype_size;
    MPI_Type_size(datatype, &datatype_size);

    std::vector<CommRankNumber> comm_rank_numbers;
    for(int i = 0; i < gathered_number_count; ++i){
        comm_rank_numbers.emplace_back(i, gathered_numbers.data() + (i*datatype_size), datatype);
    }

    if (datatype == MPI_FLOAT){
        std::sort(comm_rank_numbers.begin(), comm_rank_numbers.end(), CommRankNumber::compare_float);
    }
    else if (datatype == MPI_INT) {
        std::sort(comm_rank_numbers.begin(), comm_rank_numbers.end(), CommRankNumber::compare_int);
    }

    std::vector<int> ranks(gathered_number_count);
    for (int i = 0; i < gathered_number_count; ++i) {
        ranks[comm_rank_numbers[i].get_comm_rank()] = i; // Use get_comm_rank() to access comm_rank
    }

    return ranks;
}

int TMPI_Rank(void* send_data, void* recv_data, MPI_Datatype datatype, MPI_Comm comm) {
    if (datatype != MPI_INT && datatype != MPI_FLOAT) {
        return MPI_ERR_TYPE;
    }

    int comm_size, comm_rank;
    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &comm_rank);

    auto gathered_numbers = gather_numbers_to_root(send_data, datatype, comm);

    std::vector<int> ranks;
    if (comm_rank == 0) {
        ranks = get_ranks(gathered_numbers, comm_size, datatype);
    }

    MPI_Scatter(ranks.data(), 1, MPI_INT, recv_data, 1, MPI_INT, 0, comm);

    return MPI_SUCCESS;
}