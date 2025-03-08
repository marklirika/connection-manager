#pragma once

#include "MonitorThread.h"
#include "SSHManager.h"

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

    bool connection_check(const std::string& interface, const std::string& ip);

    /** 
     * @brief Simulates connecting to a device using mock data.
     * 
     * This method simulates the connection to a device by using a mock interface IP address.
     * The connection process is simulated by repeatedly pinging the device until a failure occurs.
     * 
     * @param interfaceIpAddr The IP address of the interface to be used for the mock connection.
     */
    void connectToDeviceMock(std::string& interfaceIpAddr);

    /* members */
    interface if0;
    interface if1;
    bool isUsingSSH;
    SSHManager sm;
    MonitorThread monitorThread;
};