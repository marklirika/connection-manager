#pragma once

#include <thread>
#include <string>
#include <atomic>

struct interface {
    std::string ifname;
    std::atomic<bool> status;
};

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
