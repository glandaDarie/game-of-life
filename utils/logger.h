#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <ctime>
#include <stdexcept>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    Logger(const std::string& filename);
    void log(LogLevel level, const std::string& message);
    ~Logger();

private:
    std::string getCurrentTime(std::time_t timestamp);
    std::string getAbsolutePath(std::string pwd, std::string filename);
    std::ofstream logFile;
};

#endif
