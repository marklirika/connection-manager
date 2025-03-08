# Test Connection Manager (CM)

**CM** is a tool designed to monitor two network interfaces and establish a more efficient connection (using the primary interface `if0`) to a "Jetson" device via a "Satellite". It utilizes a special monitoring thread that logs network statuses and connection information.

# Installation
```bash
git clone https://github.com/marklirika/connection-manager connection-manager && cd connection-manager
source prepare.sh
make
```
Once built, you can run the program with the following commands:
```bash
sudo ./connection-manager
```
**Note:** It is important to use `sudo` to run these commands.

## Modes of Operation

CM works in two modes:
- **Mock Mode**
- **SSH Mode**

### Mock Mode

In **Mock Mode**, CM simulates a connection by pinging the interface it is connected to. The connection is considered lost when the ping fails. **Mock Mode** is enabled by default.

To create and control virtual `eth0/eth1` mock interfaces you can utilze
```bash
./interfaces <up|down> [0|1]
```
To turn of/off and also additionally if you want choose particular interface

### SSH Mode

In **SSH Mode**, CM actually connects to a SBC, such as the **Raspberry Pi 4 Model B**. However, it can connect to any device using any suitable connection method.

In **SSH Mode**, you can interact with the SBC by executing simple commands, creating files, writing content with `echo`, viewing the file system, and more. The limitation of this mode is that you will always start in the `~user` directory.

#### Example Commands:
```bash
echo "Hello World!"
touch robots.txt
echo "Walley" >> robots.txt
echo "Terminator" >> robots.txt
echo "ChatGPT" >> robots.txt
cat robots.txt
ls /
```

### SSH Mode Configuration
**SSH Mode** can be enabled in the configuration file `settings.conf` by setting `SSH:1`.

#### Configuration Description
The configuration file (settings.conf) stores all necessary settings for CM and can be adjusted as needed. Here's an example configuration:

## Configuration example:
```bash
ifname0: eth0             # Main interface
ifname1: eth1             # Secondary interface
SSH: 0                    # 0 = Mock Mode, 1 = SSH Mode
user: openhd              # Username for SSH
password: openhd          # Password for SSH
ip: 192.168.3.1           # Device IP address
port: 22                  # SSH Port
```
*You can also specify the configuration path and log file path from the command line using flags*

## Command-Line Options
Usage: cm [OPTION]

    OPTION:
        -h                  Show usage manual
        -c <conf_path>      Specify path to config file
        -l <log_path>       Specify path to log file

# Enjoy!