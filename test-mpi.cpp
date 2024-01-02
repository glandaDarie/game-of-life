#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << "Rank: " + std::to_string(rank) + ", num_processes: " + std::to_string(num_procs); 

    MPI_Finalize();
    return 0;
}