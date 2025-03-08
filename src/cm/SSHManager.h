#pragma once

#include "Config.h"
#include "MonitorThread.h"
#include <libssh2.h>

class SSHManager {
public:

    /* methods */

    SSHManager();

    ~SSHManager();

    SSHManager(CredentialsSSH credentials);

    const CredentialsSSH& getCredentials() { return credentials; }

    /** 
     * @brief Connects to a device over SSH.
     * 
     * This method connects to a device via SSH using the provided SSH credentials, performing 
     * any necessary initialization for the SSH connection.
     * 
     * @param credentials The SSH credentials needed to establish the connection.
     */
    void connectToDeviceSSH(MonitorThread& monitorThread);
private:

    /* methods */

    /**
     * @brief Waits for data on a socket with a specified timeout in seconds.
     * 
     * This function blocks the calling thread, waiting for data to be available on the specified socket. 
     * 
     * @param socketfd The file descriptor of the socket to wait on.
     * @param seconds The timeout period in seconds to wait for data.
     * 
     * @return `true` if data is available within the timeout, `false` otherwise.
     */
    bool waitWithTimeout(int seconds);

    /** 
     * @brief Authenticates the SSH session with the provided credentials.
     * 
     * This method performs authentication on an SSH session using the provided credentials.
     * 
     * @param session The SSH session object to authenticate.
     * @param credentials The SSH credentials required for authentication.
     * 
     * @return int Returns 0 on succes indicating the result of the authentication process.
     *         err_code on error
     */
    int authenticate();

    /** 
     * @brief Enters the SSH session using the provided credentials.
     * 
     * This method establishes an SSH connection and starts an interactive session using the
     * provided credentials.
     * 
     * @param session The SSH session object.
     * @param socketfd The socket fille descriptor, ssh connection established with
     * @param credentials The credentials for the SSH login.
     */
    void enterSSH();

    /* members */
    int socketfd;
    LIBSSH2_SESSION *session;
    CredentialsSSH credentials;
};