#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include "/usr/local/cxxopts/include/cxxopts.hpp"
#include "/usr/local/opt/libomp/include/omp.h"
#include "utils/constants.h"
#include "utils/logger.h"
#include "helpers/game_helpers.h"
#include <functional>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    cxxopts::Options options("Game of Life", "Parallelize the game of life using the OpenMP Framework");
    options.add_options()
        ("g,generations", "Number of generations", cxxopts::value<int>()->default_value(std::to_string(Constants::NUM_GENERATIONS)))
        ("r,rows", "Number of rows", cxxopts::value<int>()->default_value(std::to_string(Constants::BOARD_ROWS)))
        ("c,columns", "Number of columns", cxxopts::value<int>()->default_value(std::to_string(Constants::BOARD_COLUMNS)))
        ("s,chunk_size", "Chunk size", cxxopts::value<int>()->default_value(std::to_string(Constants::CHUNK_SIZE)))
        ("tl,test_leetcode", "Test Leetcode", cxxopts::value<bool>()->default_value("false"))
        ("disp,display_logging_trace", "Display logging trace", cxxopts::value<bool>()->default_value("false"))
        ("tr,trace", "Trace", cxxopts::value<bool>()->default_value("false"));

    const cxxopts::ParseResult result = options.parse(argc, argv);

    Constants::NUM_GENERATIONS = result["generations"].as<int>();
    Constants::BOARD_ROWS = result["rows"].as<int>();
    Constants::BOARD_COLUMNS = result["columns"].as<int>();
    Constants::CHUNK_SIZE = result["chunk_size"].as<int>();
    bool isTestLeetcode = result["test_leetcode"].as<bool>();
    bool displayLoggingTrace = result["display_logging_trace"].as<bool>();
    bool trace = result["trace"].as<bool>();

    Logger logger("logs.txt");
    logger.log(LogLevel::INFO, "NUM_GENERATIONS: " + std::to_string(Constants::NUM_GENERATIONS));
    logger.log(LogLevel::INFO, "BOARD_ROWS: " + std::to_string(Constants::BOARD_ROWS));
    logger.log(LogLevel::INFO, "BOARD_COLUMNS: " + std::to_string(Constants::BOARD_COLUMNS));
    logger.log(LogLevel::INFO, "CHUNK_SIZE: " + std::to_string(Constants::CHUNK_SIZE));
    logger.log(LogLevel::INFO, "DISPLAY_LOGGING_TRACE: " + std::to_string(displayLoggingTrace));

    std::function<std::vector<std::vector<int>>()> boardFn = [&]() -> std::vector<std::vector<int>> {
        return isTestLeetcode ? testLeetcode() : randomBoard();
    };

    std::vector<std::vector<int>> board = generateBoard(boardFn, "test");
    displayBoard(board, "Starting board");
    std::unordered_map<std::string, std::any> kwargs = {
        {"num_generations", Constants::NUM_GENERATIONS},
        {"broadcast", true},
        {"display", displayLoggingTrace},
        {"trace", trace}
    };
    runGenerations(board, &logger, kwargs);
    return 0;
}