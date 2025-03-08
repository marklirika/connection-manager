#include "ConnectionManager.h"
#include "CMLogger.h"
#include "Config.h"

#include <stdexcept>
#include <iostream>

int main(int argc, char* argv[]) {
    std::string configFilepath{utils::DEFAULT_CONFIG_PATH};
    std::string logFilepath{utils::LOG_DEFAULT_FILEPATH};

    auto argValues = utils::Config::getArgValues(argc, argv);
    std::cout << argValues.configFilepath;

    if (!argValues.configFilepath.empty()) {
        configFilepath = argValues.configFilepath;
    }

    if (!argValues.logFilepaht.empty()) {
        logFilepath = argValues.logFilepaht;
    }
    try {
        auto config = utils::Config::getConfig(configFilepath);

        utils::CMLogger::setFilepath(logFilepath);

        ConnectionManager cm{config};
        cm.run();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
    }  
    return 0;
}
