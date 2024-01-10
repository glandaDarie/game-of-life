#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "/usr/local/cxxopts/include/cxxopts.hpp"
#include <assert.h>
#include <unordered_map>
#include <any>
#include <cassert>
#include "/usr/local/opt/libomp/include/omp.h"
#include "../utils/constants.h"
#include "../utils/logger.h"
#include <functional>
#include <algorithm>
#include <chrono>
#include <mpi.h>

std::vector<std::vector<int>> randomBoard(float probabilityAliveThreshold = 0.3f) {
    std::vector<std::vector<int>> board(Constants::BOARD_ROWS, std::vector<int>(Constants::BOARD_COLUMNS, 0));
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            float probabilityAlive = static_cast<float>(rand()) / RAND_MAX;
            if(probabilityAlive < probabilityAliveThreshold) {
                board[i][j] = 1;
            }
        }
    }
    return board;
}

std::vector<std::vector<int>> testLeetcode() {
    std::vector<std::vector<int>> board = {
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 1},
        {0, 0, 0}
    };
    Constants::BOARD_ROWS = board.size();
    Constants::BOARD_COLUMNS = (Constants::BOARD_ROWS > 0) ? board[0].size() : 0;
    return board;
}

void displayBoard(const std::vector<std::vector<int>>& board, std::string message = "") {
    if(!message.empty()) {
        std::cout << message << std::endl;
    }
    for(const std::vector<int>& row : board) {
        for(const int cell : row) {
            std::cout << cell << " "; 
        }
        std::cout << std::endl;
    }
    std:: cout << std::endl;
}

int numberNeighbours(std::vector<std::vector<int>>& board, int i, int j) {
    int aliveNeighbours = 0;
    for (int k = std::max(i - 1, 0); k < std::min(i + 2, Constants::BOARD_ROWS); ++k) {
        for (int l = std::max(j - 1, 0); l < std::min(j + 2, Constants::BOARD_COLUMNS); ++l) {
            aliveNeighbours += board[k][l] & 1;
        }
    }
    return aliveNeighbours;
}

void calculateNextGenerationSyncronous(std::vector<std::vector<int>>& board, int rank, int numProcesses, Logger* logger = nullptr, const bool display = false) {
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            int aliveNeighbours = numberNeighbours(board, i, j);
            if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                if(display) {
                    logger->log(LogLevel::INFO, "Thread main (event loop),  board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                }
                board[i][j] |= 2; 
            }
        }
    }
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            if(display) {
                logger->log(LogLevel::INFO, "Thread main (event loop),  board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
            }
            board[i][j] >>= 1;
        }
    }
}

void ompCalculateNextGenerationParallelized1D(std::vector<std::vector<int>>& board, int rank, int numProcesses, Logger* logger = nullptr, const bool display = false) {
    #pragma omp parallel for collapse(1)
     for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        int threadID = omp_get_thread_num();
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            int aliveNeighbours = numberNeighbours(board, i, j);
            if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                if(display) {
                    logger->log(LogLevel::INFO, "Thread " + std::to_string(threadID) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                }
                board[i][j] |= 2; 
            }
        }
    }
    #pragma omp parallel for collapse(1)
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        int threadID = omp_get_thread_num();
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            if(display) {
                logger->log(LogLevel::INFO, "Thread " + std::to_string(threadID) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
            }
            board[i][j] >>= 1;
        }
    }
}

void ompCalculateNextGenerationParallelized2D(std::vector<std::vector<int>>& board, int rank, int numProcesses, Logger* logger = nullptr, const bool display = false) {
    int numChunksRows = (Constants::BOARD_ROWS + Constants::CHUNK_SIZE - 1) / Constants::CHUNK_SIZE;
    int numChunksCols = (Constants::BOARD_COLUMNS + Constants::CHUNK_SIZE - 1) / Constants::CHUNK_SIZE;

    #pragma omp parallel for collapse(2)
    for (int chunkRow = 0; chunkRow < numChunksRows; ++chunkRow) {
        for (int chunkCol = 0; chunkCol < numChunksCols; ++chunkCol) {
            int startRow = chunkRow * Constants::CHUNK_SIZE;
            int startCol = chunkCol * Constants::CHUNK_SIZE;
            int endRow = std::min(startRow + Constants::CHUNK_SIZE, Constants::BOARD_ROWS);
            int endCol = std::min(startCol + Constants::CHUNK_SIZE, Constants::BOARD_COLUMNS);
            for (int i = startRow; i < endRow; ++i) {
                for (int j = startCol; j < endCol; ++j) {
                    int aliveNeighbours = numberNeighbours(board, i, j);
                    if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                        if (display) {
                            int threadID = omp_get_thread_num();
                            logger->log(LogLevel::INFO, "Thread " + std::to_string(threadID) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                        }
                        board[i][j] |= 2; 
                    }
                }
            }
        }
    }

    #pragma omp parallel for collapse(2)
    for (int chunkRow = 0; chunkRow < numChunksRows; ++chunkRow) {
        for (int chunkCol = 0; chunkCol < numChunksCols; ++chunkCol) {
            int startRow = chunkRow * Constants::CHUNK_SIZE;
            int startCol = chunkCol * Constants::CHUNK_SIZE;
            int endRow = std::min(startRow + Constants::CHUNK_SIZE, Constants::BOARD_ROWS);
            int endCol = std::min(startCol + Constants::CHUNK_SIZE, Constants::BOARD_COLUMNS);
            for (int i = startRow; i < endRow; ++i) {
                for (int j = startCol; j < endCol; ++j) {
                    if (display) {
                        int threadID = omp_get_thread_num();
                        logger->log(LogLevel::INFO, "Thread " + std::to_string(threadID) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                    }
                    board[i][j] >>= 1;
                }
            }
        }
    }
}

void mpiCalculateNextGenerationParallelized2D(
    std::vector<std::vector<int>>& board, 
    int rank,
    int numProcesses,
    Logger* logger = nullptr, 
    const bool display = false
) {
    const int numRows = board.size();
    const int numCols = board[0].size();
    const int halfRows = numRows / 2; 

    const int startRow = (rank == 0) ? 0 : halfRows;
    const int endRow = (rank == 0) ? halfRows : numRows;

    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < numCols; ++j) {
            int aliveNeighbours = numberNeighbours(board, i, j);
            if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                board[i][j] |= 2;
                if (display) {
                    logger->log(LogLevel::INFO, "Rank " + std::to_string(rank) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                }
            }
        }
    }

    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < numCols; ++j) {
            if (display) {
                logger->log(LogLevel::INFO, "Rank " + std::to_string(rank) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
            }
            board[i][j] >>= 1;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    std::vector<int> sendBuffer((endRow - startRow) * numCols, 0);
    std::vector<int> receiveBuffer((endRow - startRow) * numCols, 0);

    int iteration = 0;
    for(int i = startRow; i < endRow; ++i) {
        for(int j = 0; j < numCols; ++j) {
            sendBuffer[iteration++] = board[i][j];
        }
    }

    if(rank == 0) {
        MPI_Recv(receiveBuffer.data(), (endRow - startRow) * numCols, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else if(rank == 1) {
        MPI_Send(sendBuffer.data(), (endRow - startRow) * numCols, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    iteration = 0;
    int offset = (rank == 0) ? halfRows : 0;
    int limit = (rank == 0) ? numRows : halfRows;
    for(int i = offset; i < limit; ++i) {
        for(int j = 0; j < numCols; ++j) {
            board[i][j] = receiveBuffer[iteration++];
        }
    }
}

void computeGameOfLife(
    void (*nextGenerationCallback)(std::vector<std::vector<int>>&, int, int, Logger*, bool),
    std::vector<std::vector<int>> board,
    int generations, 
    int rank, 
    int numProcesses,
    Logger* logger = nullptr,
    const bool display = false,
    std::string executionType = "Serial",
    bool trace = false) {
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    for (int generation = 0; generation < generations; ++generation) {
        nextGenerationCallback(board, rank, numProcesses, logger, display);
        if (generation == generations - 1) {
            if(rank == 0) { // this check is for MPI (this happened because of the code structure I did)
                std::cout << "Final board" << std::endl << std::endl;
                displayBoard(board); 
            }
            // std::cout << "Final board" << std::endl << std::endl;
            // displayBoard(board); 
        }
        if(trace) {
            displayBoard(board); 
            std::cout << std::endl;
        }
    }
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;
    if (logger != nullptr) {
        logger->log(LogLevel::INFO, " EXECUTION TIME: {" + executionType + "} -> " + std::to_string(seconds) + " seconds");
    }
}

void runGenerations(
    std::vector<std::vector<int>>& board,
    std::string algorithm,
    Logger* logger = nullptr,
    const std::unordered_map<std::string, std::any> kwargs = {}
    ) {
    int generations;
    int rank, numProcesses;
    bool trace, display, broadcast, serial, parallel1d, parallel2d;

    try {
        generations = std::any_cast<int>(kwargs.at("num_generations"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for generations: " << e.what() << std::endl;
        return;
    }

    try {
        broadcast = std::any_cast<bool>(kwargs.at("broadcast"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for broadcast: " << e.what() << std::endl;
        return;
    }

    try {
        display = std::any_cast<bool>(kwargs.at("display"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for display: " << e.what() << std::endl;
        return;
    }

    try {
        trace = std::any_cast<bool>(kwargs.at("trace"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for trace: " << e.what() << std::endl;
        return;
    }

    try {
        rank = std::any_cast<int>(kwargs.at("rank"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for rank: " << e.what() << std::endl;
        return;
    }

    try {
        numProcesses = std::any_cast<int>(kwargs.at("numProcesses"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught for rank: " << e.what() << std::endl;
        return;
    }

    if(!broadcast) {
        if(algorithm == "OMP") {
            try {
                serial = std::any_cast<bool>(kwargs.at("serial"));
                parallel1d = std::any_cast<bool>(kwargs.at("parallel_1d"));
                parallel2d = std::any_cast<bool>(kwargs.at("parallel_2d"));
            } catch(const std::bad_any_cast& e) {
                std::cerr << "Exception caught: " << e.what() << std::endl;
                return;
            }
            if(serial) {
                std::cout << "Synchronous version (OMP):" << std::endl;
                computeGameOfLife(calculateNextGenerationSyncronous, board, generations, rank, numProcesses, logger, display, "Serial - OMP", trace);
            }
            if(parallel1d) {
                std::cout << "\nParallelized 1D version (OMP):" << std::endl;
                computeGameOfLife(ompCalculateNextGenerationParallelized1D, board, generations, rank, numProcesses, logger, display, "Parallel 1D - OMP", trace);
            }
            if(parallel2d) {
                std::cout << "\nParallelized 2D version (OMP):" << std::endl;
                computeGameOfLife(ompCalculateNextGenerationParallelized2D, board, generations, rank, numProcesses, logger, display, "Parallel 2D - OMP", trace);
            }
        } else {
            if(rank == 0) {
                std::cout << "\nParallelized 2D version (MPI):" << std::endl;
            }
            computeGameOfLife(mpiCalculateNextGenerationParallelized2D, board, generations, rank, numProcesses, logger, display, "Parallel 2D - MPI", trace);
        }
    } else {
        if(algorithm == "OMP") {
            std::cout << "Synchronous version (OMP):" << std::endl;
            computeGameOfLife(calculateNextGenerationSyncronous, board, generations, rank, numProcesses, logger, display, "Serial - OMP", trace);

            std::cout << "\nParallelized 1D version (OMP):" << std::endl;
            computeGameOfLife(ompCalculateNextGenerationParallelized1D, board, generations, rank, numProcesses, logger, display, "Parallel 1D - OMP", trace);

            std::cout << "\nParallelized 2D version (OMP):" << std::endl;
            computeGameOfLife(ompCalculateNextGenerationParallelized2D, board, generations, rank, numProcesses, logger, display, "Parallel 2D - OMP", trace);
        }
        else {
            if(rank == 0) {
                std::cout << "\nParallelized 2D version (MPI):" << std::endl;
            }
            computeGameOfLife(mpiCalculateNextGenerationParallelized2D, board, generations, rank, numProcesses, logger, display, "Parallel 2D - MPI", trace);
        }
    }
}