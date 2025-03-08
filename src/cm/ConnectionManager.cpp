#include "ConnectionManager.h"
#include "CMLogger.h"

/* std */
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <cstring>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

constexpr int TIMEOUT = 5;

ConnectionManager::ConnectionManager(utils::Config config) 
    : if0{config.ifname0}, if1{config.ifname1}, isUsingSSH{config.isUsingSSH}, credentials{config.credentials}
{
    utils::CMLogger::log(utils::INFO, "Initializing CM...");
    monitorThread.start(if0, if1);
}

void ConnectionManager::MonitorThread::start(interface& if0, interface& if1) {
    utils::CMLogger::log(utils::INFO, "Starting interface monitor thread...");
    thread = std::thread(&MonitorThread::monitorNetworkStatus, this, std::ref(if0), std::ref(if1));
    thread.detach();
}

void ConnectionManager::MonitorThread::monitorNetworkStatus(interface& if0, interface& if1) {
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

bool ConnectionManager::MonitorThread::isNetworkAvailable(const std::string& ifname) {
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

int ConnectionManager::authenticate(LIBSSH2_SESSION *session, CredentialsSSH& credentials) {
    int res{0};
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return sock;
    }
    
    utils::CMLogger::log(utils::INFO, 
        "Authenticating with user: " + credentials.user + 
        " at IP: " + credentials.ip + 
        " on port: " + std::to_string(credentials.port));

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(credentials.port);
    res = inet_pton(AF_INET, credentials.ip.c_str(), &sockaddr.sin_addr);
    if (res <= 0) {
        std::cerr << "Invalid IP address" << std::endl;
        close(sock);
        return res;
    }

    res = connect(sock, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (res != 0) {
        std::cerr << "Failed to connect to device: " << strerror(errno) << std::endl;
        close(sock);
        return res;
    }

    res = libssh2_session_handshake(session, sock);
    if (res) {
        std::cerr << "Failed to establish SSH connection: " << res << std::endl;
        close(sock);
        return res;
    }

    res = libssh2_userauth_password(session, credentials.user.c_str(), credentials.password.c_str());
    if (res) {
        std::cerr << "Authentication failed: " << res << std::endl;
        close(sock);
        return res;
    }

    return res;
}

void ConnectionManager::enterSSH(LIBSSH2_SESSION *session, CredentialsSSH credentials) {
    LIBSSH2_CHANNEL *channel;

    while (true) {
        int res;
        std::string userInput;
        std::array<char, 8192> buffer;
        int nbytes;

        channel = libssh2_channel_open_session(session);
        if (!channel) {
            throw std::runtime_error("Failed to open channel");
        }

        std::cout << credentials.user << "@" << credentials.password << ":";
        std::getline(std::cin, userInput);

        if (userInput == "exit") {
            std::cout << "Device shell exited..." << std::endl;
            break;
        }

        res = libssh2_channel_exec(channel, userInput.c_str());
        if (res) {
            throw std::runtime_error("Failed to execute command: " + std::to_string(res));
        }

        while ((nbytes = libssh2_channel_read(channel, buffer.data(), buffer.size())) > 0) {
            std::cout.write(buffer.data(), nbytes);
        }
        if (nbytes < 0) {
            throw std::runtime_error("Error reading channel: " + std::to_string(nbytes) +
                                     ", " + std::to_string(libssh2_session_last_error(session, nullptr, nullptr, 0)));
        }

        res = libssh2_channel_wait_eof(channel);
        if (res < 0) {
            throw std::runtime_error("Error waiting for EOF: " + std::to_string(res));
        }

        res = libssh2_channel_wait_closed(channel);
        if (res < 0) {
            throw std::runtime_error("Error waiting for channel close: " + std::to_string(res));
        }

        libssh2_channel_free(channel);
    }
    
    if (channel) libssh2_channel_free(channel);
}

void ConnectionManager::connectToDeviceSSH(CredentialsSSH& credentials) {
    int res = libssh2_init(0);
    if (res != 0) {
        throw std::runtime_error("Failed to initialize libssh2: " + std::to_string(res));
    }

    LIBSSH2_SESSION *session = libssh2_session_init();
    if (!session) {
        throw std::runtime_error("Failed to create SSH session: " + std::to_string(res));
    }

    res = authenticate(session, credentials);
    if (res) {
        throw std::runtime_error("Failed to connect device: " + std::to_string(res));
    }

    utils::CMLogger::log(utils::INFO, "Successfully connected to device!");

    std::cout << "Successfully connected to device!" << std::endl;
    monitorThread.isConnectionEstablished.store(true);
    enterSSH(session, credentials);

    utils::CMLogger::log(utils::INFO, "Session of connection ended!");

    libssh2_session_free(session);
    libssh2_exit();
}

void ConnectionManager::run() {
    while (true) {
        std::string selectedInterface = selectAvailableInterface();
        try {
            if (!selectedInterface.empty()) {
                utils::CMLogger::log(utils::INFO, "Interface found, connecting to device...");
                
                if (isUsingSSH) {
                    utils::CMLogger::log(utils::INFO, "Establishing connectio via SSH");
                    connectToDeviceSSH(credentials);
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
                std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT));
            }
        }
        catch (const std::runtime_error& e) {
            utils::CMLogger::log(utils::ERROR, std::string(e.what()));
        }
    }
}