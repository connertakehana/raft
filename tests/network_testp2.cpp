#include "../src/network_handler.h"
#include "../src/server.h"
#include "../src/message.h"
#include <nlohmann/json.hpp>
#include <zmq.hpp>
#include <chrono>
#include <thread>
#include <iostream>

using json = nlohmann::json;
//network_testp1 and network_testp2 communicate
// std::string MY_IP = "10.31.41.49";
// std::string MY_PORT = "4205";

// std::string PEER_IP = "10.31.41.49";
// std::string PEER_PORT = "4204";


// std::string MY_IP = "*";
// std::string MY_PORT = "4205";

// std::string PEER_IP = "localhost";
// std::string PEER_PORT = "4204";

std::string MY_IP = "10.34.148.91";
std::string MY_PORT = "4207";

std::string PEER_IP = "10.34.148.91";
std::string PEER_PORT = "4206";


// int main() {
//     std::vector<Peer> peers;
//     Peer p2 = {PEER_IP, PEER_PORT};
//     peers.push_back(p2);
//     NetworkHandler net_handler(peers, MY_IP, MY_PORT);
//     json rpc;
//     while (true) {
//         int32_t ret = net_handler.recieve_rpc(rpc, 0);
//         printf("Success: %d\n", ret);
//         for (int i = 0; i < 1000000000;i++);
//     }
//     return 0;
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PULL);
//     socket.connect("tcp://localhost:5555");  // Connect to the sender's port

//     zmq::message_t message;
//     socket.recv(message, zmq::recv_flags::none);
//     std::string received_msg(message.data<char>(), message.size());
//     std::cout << "Received message: " << received_msg << std::endl;
    
//     zmq::message_t message2;
//     socket.recv(message2, zmq::recv_flags::none);
//     std::string received_msg2(message.data<char>(), message.size());
//     std::cout << "Received message: " << received_msg2 << std::endl;
    
//     zmq::message_t message3;
//     socket.recv(message3, zmq::recv_flags::none);
//     std::string received_msg3(message.data<char>(), message.size());
//     std::cout << "Received message: " << received_msg3 << std::endl;
//     return 0;
// }


// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PULL);
//     socket.connect("tcp://localhost:5555");  // Connect to the sender's port

//     zmq::message_t message;
//     socket.recv(message, zmq::recv_flags::none);
//     std::string received_msg(message.data<char>(), message.size());
//     std::cout << "Received message: " << received_msg << std::endl;
    
//     // zmq::message_t message2;
//     // socket.recv(message2, zmq::recv_flags::none);
//     // std::string received_msg2(message.data<char>(), message.size());
//     // std::cout << "Received message: " << received_msg2 << std::endl;
    
//     // zmq::message_t message3;
//     // socket.recv(message3, zmq::recv_flags::none);
//     // std::string received_msg3(message.data<char>(), message.size());
//     // std::cout << "Received message: " << received_msg3 << std::endl;
//     return 0;
// }

// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_ROUTER);

//     // Use a unique name for the socket identity configuration
//     std::string socketIdentity = "Router2";
//     socket.set(zmq::sockopt::routing_id, socketIdentity);

//     // Connect to Router 1 using its IP address
//     socket.connect("tcp://0.0.0.0:5555");  // Replace 192.168.1.100 with the actual IP address of Router 1

//     for (int i = 0; i < 10; ++i) {
//         std::string msg = "Hello " + std::to_string(i);
//         zmq::message_t message(msg.data(), msg.size());

//         // Send a message to Router 1
//         socket.send(zmq::message_t("Router1", 6), zmq::send_flags::sndmore);
//         socket.send(message, zmq::send_flags::none);
//     }

//     return 0;
// }

// int main() {
//     // Context
//     zmq::context_t context(1);

//     // Socket to receive messages
//     zmq::socket_t receiver(context, ZMQ_ROUTER);
//     receiver.bind("tcp://*:5555");

//     // Socket to send messages
//     zmq::socket_t sender(context, ZMQ_DEALER);
//     sender.connect("tcp://localhost:5555"); // Assuming the peer it communicates with is on this port

//     zmq::message_t request;
//     // zmq::message_t identity;
//     // receiver.recv(identity, zmq::recv_flags::none);  // Receive identity frame first
//     receiver.recv(request, zmq::recv_flags::none);   // Now receive the actual message

//     std::string received_message = request.to_string();
//     std::cout << "Received: " << received_message << std::endl;

//     // Process the message (simple echo in this example)
//     std::string response_str = "Echo: " + received_message;

//     // Send a reply
//     zmq::message_t reply(response_str.size());
//     memcpy(reply.data(), response_str.data(), response_str.size());
//     // receiver.send(identity, zmq::send_flags::sndmore); // Send identity back first
//     receiver.send(reply, zmq::send_flags::none);       // Then send the reply


//     return 0;
// }


// int main() {
//     zmq::context_t context(1);
//     zmq::socket_t socket(context, ZMQ_PULL);
//     socket.connect("tcp://localhost:5555");  // Connect to the sender's port

//     zmq::message_t message;
//     socket.recv(message, zmq::recv_flags::none);
//     std::string received_msg(message.data<char>(), message.size());
//     std::cout << "Received message: " << received_msg << std::endl;
//     return 0;
// }

// int main() {
//     zmq::context_t context(1);

//     // Socket to communicate with Peer A
//     zmq::socket_t socket(context, ZMQ_PAIR);
//     socket.connect("tcp://localhost:5555");

//     // Receive a message from Peer A
//     zmq::message_t received_message;
//     socket.recv(received_message, zmq::recv_flags::none);
//     std::string msg_received(received_message.data<char>(), received_message.size());
//     std::cout << "Peer B received: " << msg_received << std::endl;

//     for (int i = 0; i < 10; i++) {
//         std::string msg_to_send = "Hiiiii";
//         zmq::message_t message(msg_to_send.begin(), msg_to_send.end());
//         socket.send(message, zmq::send_flags::dontwait);
//     }
//     return 0;
// }

// void sendMessage(zmq::socket_t& socket, const std::string& address, const std::string& message) {
//     socket.connect(address);
//     zmq::message_t msg(message.begin(), message.end());
//     socket.send(msg, zmq::send_flags::none);
// }

// int main() { //PEER B
//     zmq::context_t context(1);
//     zmq::socket_t reciever(context, ZMQ_ROUTER);
//     reciever.bind("tcp://*:5556"); // PPER A's INFO

//     zmq::socket_t b_sender(context, ZMQ_DEALER);
//     // sendMessage(b_sender, "tcp://localhost:5556", "Hi from A");
//     zmq::message_t identity;
//     zmq::message_t message;
//     for (int i = 0; i < 10; i++) {
//         reciever.recv(identity, zmq::recv_flags::none);
//         reciever.recv(message, zmq::recv_flags::none);
//         std::string msg_str(static_cast<char*>(message.data()), message.size());
//         std::string identity_str(static_cast<char*>(identity.data()), identity.size());
//         std::cout << "Received message: " << identity_str << std::endl;
//         std::cout << "Received message: " << msg_str << std::endl;
//     }
// }

// int main() { //PEER B
//     zmq::context_t context(1);
//     zmq::socket_t reciever(context, ZMQ_ROUTER);
//     reciever.bind("tcp://10.34.104.44:5556"); // PPER B's INFO

//     zmq::message_t identity;
//     zmq::message_t message;
//     for (int i = 0; i < 10; i++) {
//         reciever.recv(identity, zmq::recv_flags::none);
//         reciever.recv(message, zmq::recv_flags::none);
//         std::string msg_str(static_cast<char*>(message.data()), message.size());
//         std::string identity_str(static_cast<char*>(identity.data()), identity.size());
//         std::cout << "Received message: " << identity_str << std::endl;
//         std::cout << "Received message: " << msg_str << std::endl;
//     }
// }

int main() {

    json jsonObject;
    jsonObject["message"] = "hello";
    std::vector<Peer> peers;
    Peer p2 = {PEER_IP, PEER_PORT};
    peers.push_back(p2);

    NetworkHandler net_handler(peers, MY_IP, MY_PORT);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    for (int i = 0; i < 10; i++) {
        Peer sender;
        json rpc;
        printf("recieving\n");
        bool ret = net_handler.recieve_rpc(sender, rpc);
        printf("Success: %d\n", ret);
        printf("got %s from %s\n", rpc.dump().c_str(), static_cast<std::string>(sender).c_str());
    }
    printf("sending\n");
    for (int i = 0; i < 10; i++) {
        net_handler.send_rpc(jsonObject, 0);
    }
    printf("sent\n");
    return 0;
}