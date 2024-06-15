# Raft: Very very minial version of raft

Minimal implementation of raft

## Getting Started

### Prerequisites

Install:

- ZeroMQ
- OpenSSL

### Building the Project

Build the project:

```bash
make
```

## Running the Project

### Manual Inputs

You can pass in manual client inputs and with:

```bash
./raft_test1
```

### Setting Up Configuration Files

When running the testing files, you need to create a `.txt` file for each server. These files must follow a specific format and include details for your server, peers, and the client.

Each line in the `.txt` file should follow this structure:

```
xxx.xxx.xxx.xxx PORT_NUMBER S
xxx.xxx.xxx.xxx PORT_NUMBER E
xxx.xxx.xxx.xxx PORT_NUMBER E
xxx.xxx.xxx.xxx PORT_NUMBER C
```

- **Your Server**: Specify your own IP address and port, followed by the character `S`.
- **Peers**: Specify the IP addresses and ports of peer servers, followed by the character `E`.
- **Client**: Specify the IP address and port of the client, followed by the character `C`.

For example, if your server's IP is `192.168.1.1` on port `8080`, two peers are on `192.168.1.2` and `192.168.1.3` on ports `8081` and `8082` respectively, and the client is on `192.168.1.4` on port `8083`, the file would look like this:

```
192.168.1.1 8080 S
192.168.1.2 8081 E
192.168.1.3 8082 E
192.168.1.4 8083 C
```
