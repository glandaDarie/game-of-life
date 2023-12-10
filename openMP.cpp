#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <cxxopts.hpp>
#include "/usr/local/opt/libomp/include/omp.h"
#include "utils/constants.h"
#include "utils/logger.h"
#include "helpers/game_helpers.h"

int main(int argc, char* argv[]) {
    cxxopts::Options options("Game of Life", "Parallelize the game of life using the OpenMP Framework");
    options.add_options()
        ("g,generations", "Number of generations", cxxopts::value<int>()->default_value(std::to_string(Constants::NUM_GENERATIONS)))
        ("r,rows", "Number of rows", cxxopts::value<int>()->default_value(std::to_string(Constants::BOARD_ROWS)))
        ("c,columns", "Number of columns", cxxopts::value<int>()->default_value(std::to_string(Constants::BOARD_COLUMNS)))
        ("s,chunksize", "Chunk size", cxxopts::value<int>()->default_value(std::to_string(Constants::CHUNK_SIZE)));

    const cxxopts::ParseResult result = options.parse(argc, argv);

    Constants::NUM_GENERATIONS = result["generations"].as<int>();
    Constants::BOARD_ROWS = result["rows"].as<int>();
    Constants::BOARD_COLUMNS = result["columns"].as<int>();
    Constants::CHUNK_SIZE = result["chunksize"].as<int>();

    Logger logger("logfile.txt");
    logger.log(LogLevel::INFO, "NUM_GENERATIONS: " + std::to_string(Constants::NUM_GENERATIONS));
    logger.log(LogLevel::INFO, "BOARD_ROWS: " + std::to_string(Constants::BOARD_ROWS));
    logger.log(LogLevel::INFO, "BOARD_COLUMNS: " + std::to_string(Constants::BOARD_COLUMNS));
    logger.log(LogLevel::INFO, "CHUNK_SIZE: " + std::to_string(Constants::CHUNK_SIZE));
    
    std::vector<std::vector<int>> board(Constants::BOARD_ROWS, std::vector<int>(Constants::BOARD_COLUMNS, 0));

    std::unordered_map<std::string, std::any> kwargs = {
        {"num_generations", Constants::NUM_GENERATIONS},
    };

    generateBoard(board);
    displayBoard(board);
    runGenerations(board, &logger, kwargs);
    return 0;
}