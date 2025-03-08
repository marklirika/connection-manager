#pragma once

#include <thread>
#include <string>
#include <atomic>
#include <libssh2.h>

#include "Config.h"

struct interface {
    std::string ifname;
    std::atomic<bool> status;
};

class ConnectionManager {
public:
    ConnectionManager() = delete;
    ConnectionManager(utils::Config config);
    /** 
     * @brief Runs the main logic of the CM.
     * 
     * This method is responsible for starting the connection process, managing interfaces,
     * and ensuring the connection state is handled properly.
     */
    void run();

private:
    struct MonitorThread {

        /* methods */

        /** 
         * @brief Starts the monitoring of network interfaces.
         * 
         * This method initiates the monitoring of two network interfaces (`if0` and `if1`) by creating 
         * a separate thread that continuously checks the network status of both interfaces.
         * 
         * @param if0 The first network interface to be monitored.
         * @param if1 The second network interface to be monitored.
         */
        void start(interface& if0, interface& if1);

        /** 
         * @brief Monitors the network status of the given interfaces.
         * 
         * This method runs in a separate thread and continuously checks the availability of both 
         * network interfaces (`if0` and `if1`), updating the connection status accordingly.
         * 
         * @param if0 The first network interface to be monitored.
         * @param if1 The second network interface to be monitored.
         */
        void monitorNetworkStatus(interface& if0, interface& if1);

        /** 
         * @brief Checks if the network is available on the given interface.
         * 
         * This method checks the network status of the specified interface to determine if the network
         * is accessible and operational.
         * 
         * @param ifname The name of the interface.
         * 
         * @return bool `true` if the network is available, `false` otherwise.
         */
        bool isNetworkAvailable(const std::string& ifname);

        /* members */
        std::atomic<bool> isConnectionEstablished;
        std::thread thread{};
    };

    /* methods */

    /** 
     * @brief Selects the available network interface.
     * 
     * This method selects an available network interface by evaluating the status of the
     * monitored interfaces and determining which one is online.
     * 
     * @return std::string The name of the available interface (e.g., "eth0", "wlp0s20f3").
     */
    std::string selectAvailableInterface();

    /** 
     * @brief Resolves the IP address associated with a given network interface.
     * 
     * This method resolves and returns the IP address for the specified network interface.
     * 
     * @param interface The name of the network interface (e.g., "eth0", "wlp0s20f3").
     * 
     * @return std::string The IP address of the interface.
     */
    std::string resolveIPbyIF(const std::string& interface);

    /** 
     * @brief Simulates connecting to a device using mock data.
     * 
     * This method simulates the connection to a device by using a mock interface IP address.
     * The connection process is simulated by repeatedly pinging the device until a failure occurs.
     * 
     * @param interfaceIpAddr The IP address of the interface to be used for the mock connection.
     */
    void connectToDeviceMock(std::string& interfaceIpAddr);

    /** 
     * @brief Authenticates the SSH session with the provided credentials.
     * 
     * This method performs authentication on an SSH session using the provided credentials.
     * 
     * @param session The SSH session object to authenticate.
     * @param credentials The SSH credentials required for authentication.
     * 
     * @return int Returns a status code indicating the result of the authentication process.
     *         A value of `0` indicates success, and non-zero values indicate an error.
     */
    int authenticate(LIBSSH2_SESSION *session, CredentialsSSH& credentials);

    /** 
     * @brief Enters the SSH session using the provided credentials.
     * 
     * This method establishes an SSH connection and starts an interactive session using the
     * provided credentials.
     * 
     * @param session The SSH session object.
     * @param credentials The credentials for the SSH login.
     */
    void enterSSH(LIBSSH2_SESSION *session, CredentialsSSH credentials);

    /** 
     * @brief Connects to a device over SSH.
     * 
     * This method connects to a device via SSH using the provided SSH credentials, performing 
     * any necessary initialization for the SSH connection.
     * 
     * @param credentials The SSH credentials needed to establish the connection.
     */
    void connectToDeviceSSH(CredentialsSSH& credentials);

    /* members */
    interface if0;
    interface if1;
    bool isUsingSSH;
    CredentialsSSH credentials;
    MonitorThread monitorThread;
};