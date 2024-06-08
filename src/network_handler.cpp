#include "network_handler.h"
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>  
#include <fcntl.h>
#include <cassert>


extern zmq::context_t context;

NetworkHandler::NetworkHandler(std::vector<Peer>& peers, 
    Peer my_info, Peer cleint_info)
    : my_info(my_info), peer_identities(peers), client_info(cleint_info) {

    recieve_socket = zmq::socket_t(context, ZMQ_ROUTER); // create a recieving ROUTER socket
    recieve_socket.bind("tcp://" + static_cast<std::string>(my_info)); // bind to our own info
    const std::string handshake_str = "hi!"; // dummy msg to send to peers for handshake
    zmq::message_t handshake(handshake_str.begin(), handshake_str.end()); // conver to sendable strnig

    std::string identity = static_cast<std::string>(my_info); // who are we?
    for (uint32_t i = 0; i < peers.size(); i++) {
        send_sockets.emplace_back(context, ZMQ_DEALER); // create a DEALER socket for sending rpc to peer
        peer_to_index[peers[i]] = i; // maps peer to index
        send_sockets[i].set(zmq::sockopt::routing_id, identity); // sets identity to our selves "ip:port"
        send_sockets[i].connect("tcp://" + static_cast<std::string>(peers[i])); // connects to peer
        send_sockets[i].send(handshake, zmq::send_flags::dontwait); // sends both identity and message
    }
    
    zmq::message_t _;
    for (uint32_t i = 0; i < peers.size(); i++)  {
        for (uint32_t j = 0; j < 2; j++) { // recieve both identity and message
            std::cout << "recieving " << i << " " << j << std::endl;
            if (!recieve_socket.recv(_, zmq::recv_flags::none)) // blocking recieve
                std::cerr << "failed to recieve from peer " << peer_identities[i] << std::endl; // probably won't ever happen
        }
    }
    std::cout << "recieved handshake" << std::endl << std::flush;
  }

void NetworkHandler::connect_client() {
    client_socket = zmq::socket_t(context, ZMQ_SUB); // create a recieving SUBSCRIBE socket
    client_socket.connect("tcp://" + static_cast<std::string>(client_info)); // connect to client
    client_socket.set(zmq::sockopt::subscribe, ""); // subscribe to all messages
}

void NetworkHandler::disconnect_client() {
    client_socket.close();
}

void NetworkHandler::send_rpc(json& rpc, Peer peer) {
    assert(peer_to_index.contains(peer));
    send_rpc(rpc, peer_to_index[peer]);
}

void NetworkHandler::send_rpc(json& rpc, uint32_t peer_idx) { // async send
    printf("thread %d sending to %d\n", std::this_thread::get_id(), peer_idx);
    assert(peer_idx < (uint32_t)peer_identities.size());
    std::string json_str = rpc.dump(); // turn json to string
    zmq::message_t msg(json_str.begin(), json_str.end()); // string to sendable messege
    send_sockets[peer_idx].send(msg, zmq::send_flags::dontwait); // send message async
}

bool NetworkHandler::recieve_rpc(Peer& sender, json& received_rpc) {
    // printf("trying to recieve\n");
    try {
        zmq::message_t identity; // recieve identity
        zmq::message_t message; // recieve message
        // recieve both identity and message -> nonblocking recieve for identity, blocking recieve for message (guarentees atomicity)
        auto recieved_identity = recieve_socket.recv(identity, zmq::recv_flags::dontwait);  
        if (!recieved_identity.has_value()) {
            // printf("failed\n");
            return false;
        }
        printf("thread %d, before\n", std::this_thread::get_id());
        auto recieved_message = recieve_socket.recv(message, zmq::recv_flags::none);
        printf("thread %d, after\n", std::this_thread::get_id());
        if (recieved_identity.has_value() && recieved_message.has_value()) {
            printf(" got %d %d\n", recieved_identity.value(), recieved_message.value());
        }
        // if ((recieved_identity.has_value() && !recieved_message.has_value()) || (!recieved_identity.has_value() && recieved_message.has_value())) {
            
        // }
        // if (recieve_socket.recv(identity, zmq::recv_flags::dontwait) && recieve_socket.recv(message, zmq::recv_flags::none)) {
        if (recieved_identity.value() && recieved_message.value()) {
            std::string identity_str(static_cast<char*>(identity.data()), identity.size()); // convvert the identity to a string
            std::string msg_str(static_cast<char*>(message.data()), message.size()); // convert the msg to a string
            sender = Peer(identity_str); // set sender to the identity that sent u the msg
            received_rpc = json::parse(msg_str); // set the received rpc to the msg recieved
            return true;
        } else {
            return false;
        }
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ error on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on ZMQ error
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on JSON parsing error
    } catch (const std::exception& e) {
        std::cerr << "Standard exception on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on any other standard exception
    }
}

bool NetworkHandler::recieve_client_rpc(json& received_rpc) {
    try {
        zmq::message_t message; // recieve message
        // recieve both identity and message -> nonblocking recieve for identity, blocking recieve for message (guarentees atomicity)
        if (client_socket.recv(message, zmq::recv_flags::dontwait)) {
            std::string msg_str(static_cast<char*>(message.data()), message.size()); // convert the msg to a string
            received_rpc = json::parse(msg_str); // set the received rpc to the msg recieved
            return true;
        } else {
            return false;
        }
    } catch (const zmq::error_t& e) {
        std::cerr << "ZMQ error on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on ZMQ error
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on JSON parsing error
    } catch (const std::exception& e) {
        std::cerr << "Standard exception on recieve_rpc: " << e.what() << std::endl;
        return false; // Return false on any other standard exception
    }
}

uint32_t NetworkHandler::get_num_peers() {
    return peer_identities.size();
}

uint32_t NetworkHandler::get_peer_idx(Peer& peer) {
    return peer_to_index[peer];
}