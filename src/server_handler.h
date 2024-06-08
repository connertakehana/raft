#pragma once
#include "server.h"


class ServerHandler {
private:
    std::shared_ptr<Server> server;
public:
    ServerHandler(std::string server_info_path);
    void main();
};  