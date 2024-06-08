#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct Log {
    uint64_t term;
    uint64_t idx;
    uint64_t key;
    uint64_t value;

    Log(uint64_t term,
        uint64_t idx,
        uint64_t key,
        uint64_t value);
    Log(json recieved);
    json serialize();

    bool operator==(const Log& other) const {
      return idx == other.idx && term == other.term;
    }

    bool operator<(const Log& other) const {
      return idx < other.idx;
    }
    bool operator<(const uint64_t& other) const {
      return idx < other;
    }
};

