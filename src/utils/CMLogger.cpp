#include "CMLogger.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <stdexcept>

namespace utils {
    std::string CMLogger::filepath = LOG_DEFAULT_FILEPATH;

    CMLogger::CMLogger(const std::string& filepath) {
        this->filepath = filepath;
    }
    
    void CMLogger::setFilepath(const std::string& filepath) {
        std::ofstream logFile(filepath, std::ios::app);
        if (!logFile) {
            throw std::runtime_error("Error creating log file: " + filepath);
        }
        logFile.close();

        getInstance(filepath);
    }

    CMLogger& CMLogger::getInstance(const std::string& filepath) {
        static CMLogger instance(filepath);
        return instance;
    }

    std::string CMLogger::currentTime() {
        char buffer[100];
        auto now = std::chrono::system_clock::now();
        auto time_point = std::chrono::system_clock::to_time_t(now);

        auto time_info = std::localtime(&time_point);
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
        
        return std::string(buffer);
    }

    void CMLogger::log(LogLevel level, const std::string& message) {
        std::string prefix;
        std::string timeStamp = currentTime();

        switch (level) {
            case INFO:
                prefix = "[INFO]";
                break;
            case ERROR:
                prefix = "[ERROR]";
                break;
        }

        std::string logMessage = timeStamp + " " + prefix + " " + message;

        std::ofstream logFile(filepath, std::ios::app);
        if (logFile.is_open()) {
            logFile << logMessage << std::endl;
        } else {
            throw std::runtime_error("Error opening log file: " + filepath);
        }
    }
}