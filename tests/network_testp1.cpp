#include "../src/network_handler.h"
#include "../src/server.h"
#include "../src/message.h"
#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <cassert>



using json = nlohmann::json;
//network_testp1 and network_testp2 communicate
// std::string MY_IP = "10.31.41.49";
// std::string MY_PORT = "4204";

// std::string PEER_IP = "10.31.41.49";
// std::string PEER_PORT = "4205";

std::string MY_IP = "10.34.148.91";
std::string MY_PORT = "4206";

std::string PEER_IP = "10.34.148.91";
std::string PEER_PORT = "4207";

// int main() {

//     Server server1;
//     AppendEntriesMessage msg(server1, 1, 1, server1.get_logs(), 1);
//     std::vector<Peer> peers;
//     Peer p2 = {PEER_IP, PEER_PORT};
//     peers.push_back(p2);
//     NetworkHandler net_handler(peers, MY_IP, MY_PORT);
//     json msg_serial = msg.serialize();
//     int32_t ret = net_handler.send_rpc(msg_serial, 0);
//     printf("Success: %d\n", ret);
//     return 0;
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PUSH);
//     socket.bind("tcp://*:5555");  // Bind to a local port

//     std::string msg = "Hello, World!";
//     zmq::message_t message(msg.begin(), msg.end());
//     std::cout << "Sending message..." << std::endl;
//     socket.send(message, zmq::send_flags::none);
//     socket.send(message, zmq::send_flags::none);
//     socket.send(message, zmq::send_flags::none);

//     return 0;
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PUSH);
//     socket.bind("tcp://*:5555");  // Bind to a local port

//     std::string msg = "Hello, World!";
//     zmq::message_t message(msg.begin(), msg.end());
//     std::cout << "Sending message..." << std::endl;
//     socket.send(message, zmq::send_flags::dontwait);
//     // socket.send(message, zmq::send_flags::none);
//     // socket.send(message, zmq::send_flags::none);

//     return 0;
// }


// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_ROUTER);

//     // Use a unique name for the socket identity configuration
//     std::string socketIdentity = "Router1";
//     socket.set(zmq::sockopt::routing_id, socketIdentity);

//     // Bind to an actual IP address
//     socket.bind("tcp://0.0.0.0:5555");  // Replace 192.168.1.100 with the actual IP address

//     zmq::message_t receivedIdentity;
//     zmq::message_t receivedMessage;

//     while (true) {
//         if (socket.recv(receivedIdentity) && socket.recv(receivedMessage)) {
//             std::cout << "Received: " << receivedMessage.to_string() << std::endl;

//             // Echo the message back to the sender
//             socket.send(receivedIdentity, zmq::send_flags::sndmore);
//             socket.send(receivedMessage, zmq::send_flags::none);
//         }
//     }

//     return 0;
// }

// int main () {
//     // Context
//     zmq::context_t context(1);

//     // Socket to send messages
//     zmq::socket_t send_socket(context, ZMQ_DEALER);
//     send_socket.connect("tcp://localhost:5555");

//     // Socket to receive messages
//     zmq::socket_t recieve_socket(context, ZMQ_ROUTER);
//     recieve_socket.bind("tcp://*:5555");

//     // Send a message
//     zmq::message_t request(5);
//     memcpy(request.data(), "Hello", 5);
//     send_socket.send(request, zmq::send_flags::dontwait);

//     // Receive a message
//     zmq::message_t reply;
//     recieve_socket.recv(reply);  // This is blocking

//     std::cout << "Received: " << reply.to_string() << std::endl;

//     return 0;
// }

// void peer(const std::string& receive_port, const std::string& send_port) {
//     zmq::context_t context(1);

//     // Router socket for receiving messages
//     zmq::socket_t receiver(context, ZMQ_ROUTER);
//     std::string bind_address = "tcp://*:" + receive_port;
//     receiver.bind(bind_address);

//     // Set a receive timeout
//     receiver.setsockopt(ZMQ_RCVTIMEO, 5000);

//     // Dealer socket for sending messages
//     zmq::socket_t sender(context, ZMQ_DEALER);
//     std::string connect_address = "tcp://localhost:" + send_port;
//     sender.connect(connect_address);

//     // Loop for a fixed number of attempts
//     for (int attempts = 0; attempts < 10; ++attempts) {
//         zmq::message_t identity;
//         zmq::message_t request;
//         if (receiver.recv(identity, zmq::recv_flags::none) && receiver.recv(request, zmq::recv_flags::none)) {
//             std::string received_msg = request.to_string();
//             std::cout << "Received: " << received_msg << " on port " << receive_port << std::endl;

//             std::string response_str = "Echo from " + receive_port + ": " + received_msg;
//             zmq::message_t reply(response_str.size());
//             memcpy(reply.data(), response_str.data(), response_str.size());

//             receiver.send(identity, zmq::send_flags::sndmore);
//             receiver.send(reply, zmq::send_flags::none);
//         }

//         // Send a message randomly
//         if (rand() % 2 == 0) {
//             std::string msg = "Hello from " + receive_port;
//             zmq::message_t message(msg.size());
//             memcpy(message.data(), msg.data(), msg.size());
//             sender.send(message, zmq::send_flags::none);
//             std::this_thread::sleep_for(std::chrono::milliseconds(500));
//         }
//     }
// }

// int main() {
//     std::thread peer1(peer, "5555", "5556");
//     std::thread peer2(peer, "5556", "5555");

//     peer1.join();
//     peer2.join();

//     return 0;
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PUSH);
//     socket.bind("tcp://*:5555");  // Bind to a local port
//     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
//     std::string msg = "Hello, World!";
//     zmq::message_t message(msg.begin(), msg.end());
//     std::cout << "Sending message..." << std::endl;
//     socket.send(message, zmq::send_flags::dontwait);
//     // while (true);
//     return 0;
// }


// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PAIR);
//     socket.bind("tcp://*:5555");  // Bind to a local port
//     std::string msg_to_send = "Hello, Peer B from A!";
//     zmq::message_t message(msg_to_send.begin(), msg_to_send.end());
//     std::cout << "Sending message..." << std::endl;
//     socket.send(message, zmq::send_flags::none);
//     for (int i = 0; i < 10; i++) {
//         zmq::message_t received_message;
//         socket.recv(received_message, zmq::recv_flags::none);
//         std::string msg_received(received_message.data<char>(), received_message.size());
//         std::cout << "received: " << msg_received << std::endl;
//     }
//     // while (true);
//     return 0;
// }


// int main() { //PEER A
//     zmq::context_t context(1);
//     zmq::socket_t reciever(context, ZMQ_ROUTER);
//     reciever.bind("tcp://*:5555"); // PPER A's INFO

//     zmq::socket_t b_sender(context, ZMQ_DEALER);
//     std::string identity = "A";
//     b_sender.set(zmq::sockopt::routing_id, identity);

    
//     b_sender.connect("tcp://localhost:5556"); // B
//     std::cout << "got here!" << std::endl;
//     for (int i = 0; i < 10; i++) {
//         std::string msg_str = "Hi from A";
//         zmq::message_t msg(msg_str.begin(), msg_str.end());
//         b_sender.send(msg, zmq::send_flags::dontwait);
//         std::cout << i << std::endl;
//     }
//     std::cout << "exited" << std::endl;
//     // b_sender.send(msg, zmq::send_flags::none);
//     // b_sender.send(msg, zmq::send_flags::none);
//     // b_sender.send(msg, zmq::send_flags::none);
//     return 0;
// }


// int main() { //PEER A
//     zmq::context_t context(1);
//     zmq::socket_t reciever(context, ZMQ_ROUTER);
//     reciever.bind("tcp://10.34.104.44:5555"); // PPER A's INFO

//     zmq::socket_t b_sender(context, ZMQ_DEALER);
//     std::string identity = "A";
//     b_sender.set(zmq::sockopt::routing_id, identity);

    
//     b_sender.connect("tcp://10.34.104.44:5556"); // B
//     std::cout << "got here!" << std::endl;
//     for (int i = 0; i < 10; i++) {
//         std::string msg_str = "Hi from A";
//         zmq::message_t msg(msg_str.begin(), msg_str.end());
//         b_sender.send(msg, zmq::send_flags::dontwait);
//         std::cout << i << std::endl;
//     }
//     std::cout << "exited" << std::endl;
//     // b_sender.send(msg, zmq::send_flags::none);
//     // b_sender.send(msg, zmq::send_flags::none);
//     // b_sender.send(msg, zmq::send_flags::none);
//     return 0;
// }


int main() {
    std::vector<Peer> peers;
    Peer p2 = {PEER_IP, PEER_PORT};
    peers.push_back(p2);
    NetworkHandler net_handler(peers, MY_IP, MY_PORT);
    json jsonObject;
    jsonObject["message"] = "hello";
    for (int i = 0; i < 10; i++) {
        net_handler.send_rpc(jsonObject, 0);
    }
    for (int i = 0; i < 10; i++) {
        Peer sender;
        json rpc;
        bool ret = net_handler.recieve_rpc(sender, rpc);
        printf("Success: %d\n", ret);
        printf("got %s from %s\n", rpc.dump().c_str(), static_cast<std::string>(sender).c_str());
    }
    
    return 0;
}