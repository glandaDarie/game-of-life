#include <iostream>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, numProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    std::cout << "Number of processes: " + std::to_string(numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << "Rank: " + std::to_string(rank) + ", num_processes: " + std::to_string(numProcs); 

    MPI_Finalize();
    return 0;
}