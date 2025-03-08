#include "SSHManager.h"
#include "CMLogger.h"

#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

constexpr int TIMEOUT = 5;

SSHManager::SSHManager() {
    int res = libssh2_init(0);
    if (res != 0) {
        throw std::runtime_error("Failed to initialize libssh2: " + std::to_string(res));
    }
}

SSHManager::~SSHManager() {
    if (!this->session) {
        libssh2_session_free(session);
    }

    libssh2_exit();
}

SSHManager::SSHManager(CredentialsSSH credentials) : credentials{credentials} {
    int res = libssh2_init(0);
    if (res != 0) {
        throw std::runtime_error("Failed to initialize libssh2: " + std::to_string(res));
    }

}

bool SSHManager::waitWithTimeout(int seconds) {
    fd_set fdset;
    struct timeval timeout;

    FD_ZERO(&fdset);
    FD_SET(socketfd, &fdset);

    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;

    int ret = select(socketfd + 1, &fdset, nullptr, nullptr, &timeout);
    return ret > 0;
}

int SSHManager::authenticate() {
    int res{0};
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        utils::CMLogger::log(utils::ERROR, "Failed to create socket");
        return socketfd;
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
        utils::CMLogger::log(utils::ERROR, "Invalid IP address");
        close(socketfd);
        return res;
    }

    res = connect(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if (res != 0) {
        utils::CMLogger::log(utils::ERROR, "Failed to connect to device: " + std::string(strerror(errno)));
        close(socketfd);
        return res;
    }

    res = libssh2_session_handshake(session, socketfd);
    if (res) {
        utils::CMLogger::log(utils::ERROR, "Failed to establish SSH connection: " + std::to_string(res));
        close(socketfd);
        return res;
    }

    res = libssh2_userauth_password(session, credentials.user.c_str(), credentials.password.c_str());
    if (res) {
        utils::CMLogger::log(utils::ERROR, "Authentication failed: " + std::to_string(res));
        close(socketfd);
        return res;
    }

    return res;
}

void SSHManager::enterSSH() {
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

        if (!waitWithTimeout(2 * TIMEOUT)) {
            throw std::runtime_error("Failed to execute command: " + std::to_string(res));
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

void SSHManager::connectToDeviceSSH(MonitorThread& monitorThread) {
    session = libssh2_session_init();
    if (!session) {
        throw std::runtime_error("Failed to create SSH session");
    }

    int res = authenticate();
    if (res == -1) {
        throw std::runtime_error("Failed to connect device: " + std::to_string(res));
    }

    utils::CMLogger::log(utils::INFO, "Successfully connected to device!");

    std::cout << "Successfully connected to device!" << std::endl;
    monitorThread.isConnectionEstablished.store(true);
    enterSSH();

    close(socketfd);
    utils::CMLogger::log(utils::INFO, "Session of connection ended!");

    libssh2_session_free(session);
}
