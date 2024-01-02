#ifndef GAME_HELPERS_H
#define GAME_HELPERS_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include <any>
#include "../utils/logger.h"
#include <string>

template<typename... Args>
std::vector<std::vector<int>> generateBoard(
    std::function<std::vector<std::vector<int>>(Args...)> boardCallback, 
    std::string boardInfo,
    Args&&... args) {
    return boardInfo == "test"
        ? boardCallback() 
        : boardCallback(std::forward<Args>(args)...);
}
std::vector<std::vector<int>> randomBoard(float probabilityAliveThreshold = 0.3f);
void displayBoard(const std::vector<std::vector<int>>& board, std::string message = "");
std::vector<std::vector<int>> testLeetcode();
int numberNeighbours(std::vector<std::vector<int>>& board, int i, int j);
void runGenerations(
    std::vector<std::vector<int>>& board,
    std::string algorithm,
    Logger* logger = nullptr,
    const std::unordered_map<std::string, std::any> kwargs = {}
);

#endif 