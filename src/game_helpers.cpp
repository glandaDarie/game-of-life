#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cxxopts.hpp>
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

void calculateNextGenerationSyncronous(std::vector<std::vector<int>>& board, Logger* logger = nullptr) {
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            int aliveNeighbours = numberNeighbours(board, i, j);
            if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                if(logger != nullptr) {
                    logger->log(LogLevel::INFO, "Thread main (event loop),  board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
                }
                board[i][j] |= 2; 
            }
        }
    }
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            if(logger != nullptr) {
                logger->log(LogLevel::INFO, "Thread main (event loop),  board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
            }
            board[i][j] >>= 1;
        }
    }
}

void calculateNextGenerationParallelized1D(std::vector<std::vector<int>>& board, Logger* logger = nullptr) {
    #pragma omp parallel for collapse(1)
     for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        int threadID = omp_get_thread_num();
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            int aliveNeighbours = numberNeighbours(board, i, j);
            if ((aliveNeighbours == 4 && board[i][j]) || aliveNeighbours == 3) {
                if(logger != nullptr) {
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
            if(logger != nullptr) {
                logger->log(LogLevel::INFO, "Thread " + std::to_string(threadID) + ": board element: [" + std::to_string(i) + "][" + std::to_string(j) + "]");
            }
            board[i][j] >>= 1;
        }
    }
}

void calculateNextGenerationParallelized2D(std::vector<std::vector<int>>& board, Logger* logger = nullptr) {
    #pragma omp parallel for collapse(2)
    for (int chunkRow = 0; chunkRow < 2; ++chunkRow) {
        for (int chunkCol = 0; chunkCol < 2; ++chunkCol) {
            int startRow = chunkRow * Constants::CHUNK_SIZE;
            int startCol = chunkCol * Constants::CHUNK_SIZE;
            for (int i = startRow; i < startRow + Constants::CHUNK_SIZE; ++i) {
                for (int j = startCol; j < startCol + Constants::CHUNK_SIZE; ++j) {
                    int liveNeighbors = 0;
                    for (int x = -1; x <= 1; ++x) {
                        for (int y = -1; y <= 1; ++y) {
                            if (x == 0 && y == 0) {
                                continue;
                            }; 
                            int neighborX = i + x; 
                            int neighborY = j + y;
                            if (neighborX >= 0 && neighborX < Constants::BOARD_ROWS && neighborY >= 0 && neighborY < Constants::BOARD_COLUMNS) {
                                liveNeighbors += board[neighborX][neighborY];
                            }
                        }
                    }
                    if (board[i][j] == 1 && (liveNeighbors < 2 || liveNeighbors > 3)) {
                        board[i][j] = 0;
                    } else if (board[i][j] == 0 && liveNeighbors == 3) {
                        board[i][j] = 1;
                    }
                }
            }
        }
    }
}

void computeGameOfLife(
    void (*nextGenerationCallback)(std::vector<std::vector<int>>&, Logger*),
    std::vector<std::vector<int>> board,
    int generations, 
    Logger* logger = nullptr,
    std::string executionType = "Serial",
    bool trace = false) {
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    for (int generation = 0; generation < generations; ++generation) {
        if (generation == generations - 1) {
            std::cout << "Final board" << std::endl << std::endl;
            displayBoard(board); 
        }
        nextGenerationCallback(board, nullptr);
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
    Logger* logger = nullptr,
    const std::unordered_map<std::string, std::any> kwargs = {}
    ) {
    std::__1::chrono::steady_clock::time_point start;
    std::__1::chrono::steady_clock::time_point end;
    double seconds;
    int generations;
    bool broadcast, serial, parallel1d, parallel2d;

    try {
        generations = std::any_cast<int>(kwargs.at("num_generations"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return;
    }

    try {
        broadcast = std::any_cast<bool>(kwargs.at("broadcast"));
    } catch(const std::bad_any_cast& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return;
    }

    if(!broadcast) {
        try {
            serial = std::any_cast<bool>(kwargs.at("serial"));
            parallel1d = std::any_cast<bool>(kwargs.at("parallel_1d"));
            parallel2d = std::any_cast<bool>(kwargs.at("parallel_2d"));
        } catch(const std::bad_any_cast& e) {
            std::cerr << "Exception caught: " << e.what() << std::endl;
            return;
        }

        if(serial) {
            std::cout << "Synchronous version:" << std::endl;
            computeGameOfLife(calculateNextGenerationSyncronous, board, generations, logger);
        }
        if(parallel1d) {
            std::cout << "\nParallelized 1D version:" << std::endl;
            computeGameOfLife(calculateNextGenerationParallelized1D, board, generations, logger, "Parallel 1D");
        }
        if(parallel2d) {
            std::cout << "\nParallelized 2D version:" << std::endl;
            computeGameOfLife(calculateNextGenerationParallelized2D, board, generations, logger, "Parallel 2D");
        }
    } else {
        std::cout << "Synchronous version:" << std::endl;
        computeGameOfLife(calculateNextGenerationSyncronous, board, generations, logger);

        std::cout << "\nParallelized 1D version:" << std::endl;
        computeGameOfLife(calculateNextGenerationParallelized1D, board, generations, logger, "Parallel 1D");

        // std::cout << "\nParallelized 2D version:" << std::endl;
        // computeGameOfLife(calculateNextGenerationParallelized2D, board, generations, logger, "Parallel 2D");
    }
}