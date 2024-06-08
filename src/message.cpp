#include "message.h"
#include <iostream>
#include <sstream>
#include "server.h"

// START
//---------------------Message-----------------------//
Message::Message(
    uint64_t term, std::string sender_id, uint64_t message_id) 
  : term(term), sender_id(sender_id), message_id(message_id) {}

json Message::serialize_logs(std::vector<Log>& logs) {
    json log_arr;
    for (Log& log: logs) {
        log_arr.push_back(log.serialize());
    }
    return log_arr;
}

std::vector<Log> Message::deserialize_logs(json json_logs) {
    std::vector<Log> logs;
    for (auto& json_log: json_logs) {
        Log log(json_log);
        logs.push_back(json_log);
    }
    return logs;
}

// END
//---------------------Message-----------------------//


// START
//---------------------AppendEntriesMessage-----------------------//
AppendEntriesMessage::AppendEntriesMessage(json recieved) 
  : Message(recieved["params"]["term"], 
            recieved["params"]["sender_id"],
            recieved["id"]),  
  prev_log_index(recieved["params"]["prev_log_index"]),
  prev_log_term(recieved["params"]["prev_log_term"]),
  leader_commit(recieved["params"]["leader_commit"]),
  logs(Message::deserialize_logs(recieved["params"]["logs"])) 
  {}

AppendEntriesMessage::AppendEntriesMessage(
    Server& server, uint64_t prev_log_index, 
    uint64_t prev_log_term, std::vector<Log>& logs, 
    uint64_t message_id) 
  : Message(server.get_cur_term(), 
            server.get_server_id(),
            message_id), 
  prev_log_index(prev_log_index),
  prev_log_term(prev_log_term),
  leader_commit(server.get_last_commit()),
  logs(logs)
  {}

json AppendEntriesMessage::serialize(){
    json rpc_send = {
        {"jsonrpc", "2.0"},
        {"method", "AppendEntriesMessage"}, // Specify your method name here
        {"params", {
            {"term", term},
            {"sender_id", sender_id},
            {"prev_log_index", prev_log_index},
            {"prev_log_term", prev_log_term},
            {"leader_commit", leader_commit},
            {"logs", Message::serialize_logs(logs)}
        }},
        {"id", message_id}
    };
    return rpc_send;
}
// END
//---------------------AppendEntriesMessage-----------------------//

// START
//---------------------AppendEntriesReply-----------------------//
AppendEntriesReply::AppendEntriesReply(json recieved) 
  : Message(recieved["params"]["term"], 
            recieved["params"]["sender_id"],
            recieved["id"]), 
  success(recieved["params"]["success"])
  {}

AppendEntriesReply::AppendEntriesReply(
    Server& server, bool success, uint64_t message_id) 
  : Message(server.get_cur_term(), 
            server.get_server_id(),
            message_id), 
    success(success)
  {}

json AppendEntriesReply::serialize(){
    json rpc_send = {
        {"jsonrpc", "2.0"},
        {"method", "AppendEntriesReply"}, // Specify your method name here
        {"params", {
            {"term", term},
            {"sender_id", sender_id},
            {"success", success}
        }},
        {"id", message_id}
    };
    return rpc_send;
}

// END
//---------------------AppendEntriesReply-----------------------//

// START
//---------------------RequestVoteMessage-----------------------//
RequestVoteMessage::RequestVoteMessage(Server& server, 
                                       uint64_t message_id) 
  : Message(server.get_cur_term(), 
            server.get_server_id(),
            message_id), 
  last_log_index(server.get_logs().empty() ? 0 : server.get_logs().back().idx),
  last_log_term(server.get_logs().empty() ? 0 : server.get_logs().back().term)
  {}

RequestVoteMessage::RequestVoteMessage(json recieved) 
  : Message(recieved["params"]["term"], 
            recieved["params"]["sender_id"],
            recieved["id"]), 
  last_log_index(recieved["params"]["last_log_index"]),
  last_log_term(recieved["params"]["last_log_term"])
  {}


json RequestVoteMessage::serialize(){
    json rpc_send = {
        {"jsonrpc", "2.0"},
        {"method", "RequestVoteMessage"}, 
        {"params", {
            {"term", term},
            {"sender_id", sender_id},
            {"last_log_index", last_log_index},
            {"last_log_term", last_log_term}
        }},
        {"id", message_id}
    };
    return rpc_send;
}
// END
//---------------------RequestVoteMessage-----------------------//

// START
//---------------------RequestVoteReply-----------------------//

RequestVoteReply::RequestVoteReply(
    Server& server, bool vote_granted, uint64_t message_id) 
  : Message(server.get_cur_term(), 
            server.get_server_id(),
            message_id), 
    vote_granted(vote_granted)
  {}

RequestVoteReply::RequestVoteReply(json recieved) 
  : Message(recieved["params"]["term"], 
            recieved["params"]["sender_id"],
            recieved["id"]), 
  vote_granted(recieved["params"]["vote_granted"])
  {}


json RequestVoteReply::serialize(){
    json rpc_send = {
        {"jsonrpc", "2.0"},
        {"method", "RequestVoteReply"}, // Specify your method name here
        {"params", {
            {"term", term},
            {"sender_id", sender_id},
            {"vote_granted",vote_granted}
        }},
        {"id", message_id}
    };
    return rpc_send;
}

// END
//---------------------RequestVoteReply-----------------------//

// START
//---------------------ClientRequest-----------------------//

ClientRequest::ClientRequest(uint64_t key, uint64_t value, uint64_t id)
  : key(key), value(value), id(id) {}

ClientRequest::ClientRequest(json recieved)
  : key(recieved["params"]["key"]),
  value(recieved["params"]["value"]),
  id(recieved["id"])
  {}

json ClientRequest::serialize(){
    json rpc_send = {
        {"jsonrpc", "2.0"},
        {"method", "ClientRequest"},
        {"params", {
            {"key", key},
            {"value", value}
        }},
        {"id", id}
    };
    return rpc_send;
}

// END
//---------------------ClientRequest-----------------------//