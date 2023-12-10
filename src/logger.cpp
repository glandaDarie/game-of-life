#include <iostream> 
#include <unistd.h>
#include "../utils/logger.h"

Logger::Logger(const std::string& filename) {
    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        std::cerr << "Failed to get current working directory" << std::endl;
        return;
    }
    const std::string current_directory = std::string(cwd);
    if (current_directory.empty()) {
        std::cerr << "Error obtaining parent directory!" << std::endl;
        return;
    }

    const std::string absolute_path = current_directory + "/" + filename;

    try {
        logFile.open(absolute_path);
        if (!logFile.is_open()) {
            throw std::runtime_error("Error opening log file!");
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return;
    }

    if (!logFile.is_open()) {
        std::cerr << "Error opening log file!" << std::endl;
        return;
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!logFile.is_open()) {
        std::cerr << "Log file is not open!" << std::endl;
        return;
    }

    const std::time_t now = std::time(nullptr);
    logFile << "[" << getCurrentTime(now) << "] ";

    switch (level) {
        case LogLevel::INFO:
            logFile << "[INFO] ";
            break;
        case LogLevel::WARNING:
            logFile << "[WARNING] ";
            break;
        case LogLevel::ERROR:
            logFile << "[ERROR] ";
            break;
    }

    logFile << message << std::endl;
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::getCurrentTime(std::time_t timestamp) {
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
    return buffer;
}

std::string Logger::getAbsolutePath(std::string pwd, std::string filename) {
    const size_t lastSlash = pwd.find_last_of('/');
    return lastSlash != std::string::npos && lastSlash != 0
               ? pwd.substr(0, lastSlash) + "/" + filename
               : std::string();
}
