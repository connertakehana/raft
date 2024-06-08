#include "client_network_handler.h"
#include <string>
#include <zmq.hpp>

zmq::context_t context(1);

ClientNetworkHandler::ClientNetworkHandler(Peer client_info)
  : client_info(client_info) {
    publisher = zmq::socket_t(context, ZMQ_PUB);
    publisher.bind("tcp://" + static_cast<std::string>(client_info));
}

void ClientNetworkHandler::send_rpc(json& rpc) {
    std::string json_str = rpc.dump(); // turn json to string
    zmq::message_t msg(json_str.begin(), json_str.end()); // string to sendable messege
    publisher.send(msg, zmq::send_flags::none); // send message
}