#include "../src/server_handler.h"
#include "../src/client_network_handler.h"

#include <vector>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>  
#include <fcntl.h>
#include <cassert>

void test_runner(std::string server_info_path) {
    ServerHandler server_handler(server_info_path);
    server_handler.main();
}

void test_client(std::string server_info_path) {
    std::ifstream server_info_file(server_info_path);
    if (!server_info_file.is_open())
        throw std::runtime_error("Could not open server info file at " + server_info_path);

    Peer client_info;
    std::string line;
    while (std::getline(server_info_file, line)) {
        std::istringstream iss(line);
        std::string ip;
        std::string port;
        char marker;
        iss >> ip >> port >> marker;
        if (marker == 'C') {
            // Line contains an IP, port, and a marker
            client_info = {ip, port};
            break;
        }
    }
    server_info_file.close();
    ClientNetworkHandler client_network_handler(client_info);

    while (true) {
        uint64_t key, value;
        std::cin >> key >> value;
        ClientRequest client_req = ClientRequest(key, value, 0);
        json rpc = client_req.serialize();
        client_network_handler.send_rpc(rpc);
    }
}

int main() {
    std::vector<std::string> init_files = {"../server_init/s1_init.txt", "../server_init/s2_init.txt"};
    std::thread server1(test_runner, init_files[0]);
    std::thread server2(test_runner, init_files[1]);
    std::thread client(test_client, init_files[1]);
    server1.join();
    server2.join();
    client.join();
    return 0;
}