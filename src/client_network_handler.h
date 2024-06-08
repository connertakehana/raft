#pragma once
#include "network_handler.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
struct Peer;

class ClientNetworkHandler {
private:
    Peer client_info;
    zmq::socket_t publisher;
public:
    ClientNetworkHandler(Peer client_info);
    void send_rpc(json& rpc);
};