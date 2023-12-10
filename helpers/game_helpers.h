#ifndef GAME_HELPERS_H
#define GAME_HELPERS_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <any>
#include "../utils/logger.h"

void generateBoard(std::vector<std::vector<int>>& board, float probability_alive_threshold = 0.3);
void displayBoard(const std::vector<std::vector<int>>& board, std::string message = "");
void calculateNextGenerationParallelized(std::vector<std::vector<int>>& board);
void runGenerations(
    std::vector<std::vector<int>>& board,
    Logger* logger = nullptr,
    const std::unordered_map<std::string, std::any> kwargs = {}
);

#endif 