#pragma once
#include <iostream>
#include <string>
#include <set>
#include <thread>
#include <map>
#include <nlohmann/json.hpp>
#include <zmq.hpp>

using json = nlohmann::json;

struct Peer {
    std::string ip;
    std::string port;

    Peer()
      : ip(""), port("") {}

    Peer(std::string ip, std::string port)
      : ip(ip), port(port) {}

    Peer(std::string ip_port)
      : ip(ip_port.substr(0, ip_port.find(':'))),
      port(ip_port.substr(ip_port.find(':') + 1)) {}
    
    bool operator<(const Peer& other) const {
        return (ip + port) < (other.ip + other.port);
    }
    bool operator==(const Peer& other) const {
        return ip == other.ip && port == other.port;
    }
    operator std::string() const { 
        return ip + ":" + port;
    }

    friend std::ostream& operator<< (std::ostream& os, const Peer& p) {
        os << static_cast<std::string>(p);
        return os;
    }
};

class NetworkHandler {
private:
    Peer my_info; // nodes ip:port
    std::map<Peer, uint32_t> peer_to_index; // Peer to index
    std::vector<Peer> peer_identities; // index to peer
    zmq::socket_t recieve_socket; // recieve socket for receiving rpcs -> everyone sends to this single reciever
    std::vector<zmq::socket_t> send_sockets; // send sockets for sending rpc -> each peer gets their own socket to send to
    
    Peer client_info; // client ip:port
    zmq::socket_t client_socket; // recieve socket for receiving from cleint

    void connection_setup(Peer& other, uint32_t index);
public:
    NetworkHandler(std::vector<Peer>& peers, Peer my_info, Peer cleint_info);

    void send_rpc(json& rpc, uint32_t peer_id);
    void send_rpc(json& rpc, Peer peer);
    bool recieve_rpc(Peer& sender, json& received_rpc);

    void connect_client();
    void disconnect_client();
    bool recieve_client_rpc(json& received_rpc);

    uint32_t get_num_peers();
    uint32_t get_peer_idx(Peer& peer);

    Peer get_my_info() { return my_info; }
    
};