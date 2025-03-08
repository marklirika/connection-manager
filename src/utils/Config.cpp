#include "Config.h"

#include <string>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace utils {
    constexpr const char* USAGE = R"(
    CM is a small tool for checking, logging, testing, and establishing connections.

    Usage: cm [OPTION]

    OPTION:
        -h                  Show usage manual
        -c <conf_path>      Specify path to config file
        -l <log_path>       Specify path to log file
    )";

    ArgValues Config::getArgValues(int argc, char* argv[]) {
        std::string filepath = "";
        int opt;
        ArgValues argValues{"",""};

        while ((opt = getopt(argc, argv, "hc:l:")) != -1) {
            switch (opt) {
                case 'h':
                    std::cout << USAGE << std::endl;
                    return {"",""};
                case 'c':
                    argValues.configFilepath = optarg;
                    std::cout << optarg;
                    break;
                case 'l':
                    argValues.logFilepaht = optarg;
                    break;
                default:
                    std::cerr << USAGE << std::endl;
                    return {"",""};
            }
        }

        return argValues;
    }

    Config Config::getConfig(const std::string& filepath) {
        std::ifstream file(filepath);
        Config config;
        std::unordered_map<std::string, std::string> map;

        if (!file.is_open()) {
            throw std::runtime_error("Could not open the file " + filepath);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::string key, value;
            std::istringstream lineStream(line);
            if (std::getline(std::getline(lineStream, key, ':'), value)) {
                map[key] = value;
            }
        }

        file.close();

        config.ifname0 = map["ifname0"];
        config.ifname1 = map["ifname1"];
        config.isUsingSSH = (map["SSH"] == "1");

        if (config.isUsingSSH) {
            config.credentials.user = map["user"];
            config.credentials.password = map["password"];
            config.credentials.ip = map["ip"];
            config.credentials.port = std::stoi(map["port"]);
        }

        return config;
    }
}