#pragma once

#include <string>

namespace utils {
    constexpr const char *LOG_DEFAULT_FILEPATH = "/var/log/cm-log.txt";

    enum LogLevel {
        INFO,
        ERROR
    };

    class CMLogger {
    public:
        static void setFilepath(const std::string& path);
        static void log(LogLevel level, const std::string& message);

    private:
        CMLogger() = delete;
        CMLogger(const std::string& filepath);

        CMLogger(const CMLogger&) = delete;
        CMLogger& operator=(const CMLogger&) = delete;

        /* methods */
        
        /** 
         * @brief Returns a reference to the  singleton `CMLogger` instance.
         * 
         * This method ensures that there is only one instance of the `CMLogger` class.
         * 
         * @param filepath The path to log messages.
         * 
         * @return CMLogger& A reference to the singleton `CMLogger` instance.
         */
        static CMLogger& getInstance(const std::string& filepath);

        /** 
         * @brief Retrieves the current system time as a formatted string.
         * 
         * This method returns system time as a formatted string.
         * The format include the full date and time `till miliseconds.
         * 
         * @return std::string A string representation of the current system time.
         */
        static std::string currentTime();

        /* members */
        static std::string filepath;
    };
}