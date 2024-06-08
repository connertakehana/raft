#include "log.h"

Log::Log(uint64_t term,
        uint64_t idx,
        uint64_t key,
        uint64_t value)
  : term(term), idx(idx), key(key), value(value) {}

Log::Log(json log_recieved) 
  : term(log_recieved["term"]),
  idx(log_recieved["idx"]),
  key(log_recieved["key"]),
  value(log_recieved["value"])
  {}

json Log::serialize(){
    json rpc_log = {
        {"term", term},
        {"idx", idx},
        {"key", key},
        {"value", value}
    };
    return rpc_log;
}