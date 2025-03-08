#pragma once

#include <string>

struct CredentialsSSH {
    std::string user;
    std::string password;
    std::string ip;
    int port;
};

namespace utils {
    constexpr const char* DEFAULT_CONFIG_PATH = "settings.conf";

    struct ArgValues {
        std::string logFilepaht;
        std::string configFilepath;
    };

    struct Config {

        /* methods */

        /** 
         * @brief Parses command-line arguments and returns the parsed values.
         * 
         * This method processes the arguments passed to the program and returns them as a structured
         * set of values.
         * 
         * @param argc The number of command-line arguments.
         * @param argv The array of command-line arguments.
         * 
         * @return ArgValues A structured object containing the parsed arguments.
         */
        static ArgValues getArgValues(int argc, char* argv[]);

        /** 
         * @brief Reads and parses a configuration file to return a `Config` object.
         * 
         * This method loads the configuration from the file specified by the given file path.
         * 
         * @param filepath The path to config.
         * 
         * @return `Config` object populated from the file.
         */
        static Config getConfig(const std::string& filepath);

        /* members */
        std::string ifname0;
        std::string ifname1;

        /* SSH */
        bool isUsingSSH;
        CredentialsSSH credentials;
    };
}