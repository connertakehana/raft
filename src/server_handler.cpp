#include "server_handler.h"
#include "network_handler.h"

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
#include <thread>


ServerHandler::ServerHandler(std::string server_info_path) {
    std::ifstream server_info_file(server_info_path);
    if (!server_info_file.is_open())
        throw std::runtime_error("Could not open server info file at " + server_info_path);

    Peer server_info, client_info;
    std::vector<Peer> peers;
    std::string line;
    while (std::getline(server_info_file, line)) {
        std::istringstream iss(line);
        std::string ip;
        std::string port;
        char marker;
        iss >> ip >> port >> marker;
        if (marker == 'S') {
            // Line contains an IP, port, and a marker
            server_info = {ip, port};
            printf("%d, my server: %s\n", std::this_thread::get_id(), static_cast<std::string>(server_info).c_str());
        } else if (marker == 'C') {
            // Line contains only an IP and port
            client_info = {ip, port};
            printf("%d, my client: %s\n", std::this_thread::get_id(), static_cast<std::string>(client_info).c_str());
        } else {
            peers.emplace_back(ip, port);
            printf("%d, peer: %s\n", std::this_thread::get_id(), static_cast<std::string>(peers.back()).c_str());
        }
    }
    server_info_file.close();
    NetworkHandler* network_handler = new NetworkHandler(peers, server_info, client_info);
    server = std::make_shared<Follower>(server_info, network_handler);
    
}

void ServerHandler::main() {
    printf("%d got to top of main\n", std::this_thread::get_id());
    while (true) {
        printf("%d, running first main\n", std::this_thread::get_id());
        SERVER_TYPE next_state = server->main();
        std::string next_state_str = next_state == SERVER_TYPE::FOLLOWER ? "follower" : (next_state == SERVER_TYPE::CANDIDATE ? "candidate" : "leader");
        printf("%d, going to state: %s\n", std::this_thread::get_id(), next_state_str.c_str());
        switch (next_state) {
            case SERVER_TYPE::FOLLOWER:
                server = std::make_shared<Follower>(server->get_state());
                break;
            case SERVER_TYPE::CANDIDATE:
                server = std::make_shared<Candidate>(server->get_state());
                break;
            case SERVER_TYPE::LEADER:
                server = std::make_shared<Leader>(server->get_state());
                break;
            default:
                std::cerr << "Unknown server type" << std::endl;
                break;
        }
    }
    
    
}

