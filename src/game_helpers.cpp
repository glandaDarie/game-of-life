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

void generateBoard(std::vector<std::vector<int>>& board, float probability_alive_threshold = 0.3) {
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < Constants::BOARD_ROWS; ++i) {
        for (int j = 0; j < Constants::BOARD_COLUMNS; ++j) {
            float probability_alive = static_cast<float>(rand()) / RAND_MAX;
            if(probability_alive < probability_alive_threshold) {
                board[i][j] = 1;
            }
        }
    }
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
}

void calculateNextGenerationParallelized(std::vector<std::vector<int>>& board) {
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

void runGenerations(
    std::vector<std::vector<int>>& board,
    Logger* logger = nullptr,
    const std::unordered_map<std::string, std::any> kwargs = {}
    ) {

    if(logger != nullptr) {
        // add the metrics information to the logger
        logger->log(LogLevel::INFO, "metric information here");
    }

    assert(kwargs.find("num_generations") == kwargs.end());
    int generations = std::any_cast<int>(kwargs.at("num_generations"));
    for (int generation = 0; generation < generations; ++generation) {
            calculateNextGenerationParallelized(board);
            displayBoard(board);
            // Uncomment the line below to print each generation (may slow down the process for larger sizes)
            // printMatrix(matrix);
    }
}