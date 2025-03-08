#include "ConnectionManager.h"
#include "CMLogger.h"

/* std */
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

constexpr int TIMEOUT = 5;

ConnectionManager::ConnectionManager(utils::Config config) 
    : if0{config.ifname0}, if1{config.ifname1}, isUsingSSH{config.isUsingSSH}, sm{config.credentials}
{
    utils::CMLogger::log(utils::INFO, "Initializing CM...");
    monitorThread.start(if0, if1);
}

std::string ConnectionManager::selectAvailableInterface() {
    if (if0.status) {
        utils::CMLogger::log(utils::INFO, if0.ifname + " had been chosen to connect to.");
        return if0.ifname;
    } else if (if1.status) {
        utils::CMLogger::log(utils::INFO, if1.ifname + " had been chosen to connect to.");
        return if1.ifname;
    }

    return "";
}

std::string ConnectionManager::resolveIPbyIF(const std::string& ifname) {
    struct ifreq ifr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockfd == -1) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return "";
    }

    std::strncpy(ifr.ifr_name, ifname.c_str(), IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFADDR,& ifr) == -1) {
        std::cerr << "Error getting IP address for interface " << ifname << ": " << strerror(errno) << std::endl;
        close(sockfd);
        return "";
    }

    sockaddr_in *ipaddr = (sockaddr_in*)&ifr.ifr_addr;
    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,& ipaddr->sin_addr, ip_str, INET_ADDRSTRLEN);

    close(sockfd);

    return std::string(ip_str);
}

void ConnectionManager::connectToDeviceMock(std::string& interfaceIpAddr) {
    bool isConnected = true;

    while (isConnected) {
        std::string command = "ping -c 1 " + interfaceIpAddr;

        int result = std::system(command.c_str());

        if (result != 0) {
            isConnected = false;
            std::cout << "Connection lost to " << interfaceIpAddr << std::endl;
        } else {
            monitorThread.isConnectionEstablished.store(true);
            std::cout << "Ping successful to " << interfaceIpAddr << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
    }
}

bool ConnectionManager::connection_check(const std::string& interface, const std::string& ip) {
    std::string command = "ping -I " + interface + " -c 1 " + ip + " > /dev/null 2>&1";

    int result = std::system(command.c_str());

    return result == 0;
}

void ConnectionManager::run() {
    while (true) {
        std::string selectedInterface = selectAvailableInterface();
        try {
            if (!selectedInterface.empty()) {
                utils::CMLogger::log(utils::INFO, "Interface found, connecting to device...");

                if (isUsingSSH) {
                    utils::CMLogger::log(utils::INFO, "Establishing connectio via SSH");
                    std::string ip = sm.getCredentials().ip;
                    if (!connection_check(selectedInterface, ip)) {
                        throw std::runtime_error("No direct connection found between " + 
                            selectedInterface + " and " + ip);
                    }
                    sm.connectToDeviceSSH(monitorThread);
                }
                else {
                    utils::CMLogger::log(utils::INFO, "Establishing connectio via Mock");
                    std::string ip = resolveIPbyIF(selectedInterface);
                    connectToDeviceMock(ip);
                }

                monitorThread.isConnectionEstablished.store(false);
                utils::CMLogger::log(utils::INFO, "Connection session ended");
            }
            else {
                utils::CMLogger::log(utils::INFO, "No interfaces are unavailable. Retrying...");
            }
            std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
        }
        catch (const std::runtime_error& e) {
            utils::CMLogger::log(utils::ERROR, std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT / 2));
        }
    }
}