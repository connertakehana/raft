#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "log.h"

using json = nlohmann::json;

class Server;

#define GETTER(type, name) \
    type get_##name() { return name; }

struct Message {
    uint64_t term;
    std::string sender_id;
    uint64_t message_id;   
    Message(uint64_t term, std::string sender_id, uint64_t message_id);

    static std::vector<Log> deserialize_logs(json json_logs);
    static json serialize_logs(std::vector<Log>& logs);

    GETTER(uint64_t, term);
    GETTER(std::string, sender_id);
    GETTER(uint64_t, message_id);
};


struct AppendEntriesMessage : public Message {
    // previous log index + term for followers to agree w/, checking for consistency
    uint64_t prev_log_index, prev_log_term; 
    uint64_t leader_commit; // commit index for leader
    std::vector<Log> logs;

    AppendEntriesMessage(
        Server& server, uint64_t prev_log_index, 
        uint64_t prev_log_term, std::vector<Log>& logs, 
        uint64_t message_id);
    AppendEntriesMessage(json recieved);
    json serialize();
};


struct AppendEntriesReply : public Message {
    // previous log index + term for followers to agree w/, checking for cosnistency
    bool success;

    AppendEntriesReply(Server& server, bool success, uint64_t message_id);
    AppendEntriesReply(json recieved);
    json serialize();
};


struct RequestVoteMessage : public Message {
    uint64_t last_log_index; // index of candidates last log entry
    uint64_t last_log_term; // latest term in log

    RequestVoteMessage(Server& server, uint64_t message_id);
    RequestVoteMessage(json recieved);
    json serialize();
};


struct RequestVoteReply : public Message {
    bool vote_granted;

    RequestVoteReply(Server& server, bool vote_granted, uint64_t message_id);
    RequestVoteReply(json recieved);
    json serialize();
};


struct ClientRequest {
    uint64_t key;
    uint64_t value;
    uint64_t id;

    ClientRequest(uint64_t key, uint64_t value, uint64_t id);
    ClientRequest(json recieved);
    json serialize();
};
