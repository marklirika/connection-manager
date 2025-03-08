#include "MonitorThread.h"
#include "CMLogger.h"

/* std */
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

constexpr int TIMEOUT = 5;

void MonitorThread::start(interface& if0, interface& if1) {
    utils::CMLogger::log(utils::INFO, "Starting interface monitor thread...");
    thread = std::thread(&MonitorThread::monitorNetworkStatus, this, std::ref(if0), std::ref(if1));
    thread.detach();
}

void MonitorThread::monitorNetworkStatus(interface& if0, interface& if1) {
    while (true) {
        if0.status.store(isNetworkAvailable(if0.ifname));
        if1.status.store(isNetworkAvailable(if1.ifname));

        if (if0.status) {
            utils::CMLogger::log(utils::INFO, if0.ifname + " is online.");
        }
        else {
            utils::CMLogger::log(utils::INFO, if0.ifname + " is offline.");
        }

        if (if1.status) {
            utils::CMLogger::log(utils::INFO, if1.ifname + " is online.");
        }
        else {
            utils::CMLogger::log(utils::INFO, if1.ifname + " is offline.");
        }

        if (isConnectionEstablished.load()) {
            utils::CMLogger::log(utils::INFO, "Connection to device established");
        }
        else {
            utils::CMLogger::log(utils::INFO, "Connection to device is not established");
        }

        std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
    }
}

bool MonitorThread::isNetworkAvailable(const std::string& ifname) {
    ifreq ifr{};
    bool res{ false };

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        utils::CMLogger::log(utils::INFO, "Socket creation failed: " + std::string(strerror(errno)));
        return res;
    }

    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);

    res = (ioctl(sockfd, SIOCGIFFLAGS,& ifr) != -1);

    close(sockfd);
    return res;
}

