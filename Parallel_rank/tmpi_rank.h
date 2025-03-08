#pragma once
#include <mpi.h>

int TMPI_Rank(void* send_data, void* recv_data, MPI_Datatype datatype, MPI_Comm comm);