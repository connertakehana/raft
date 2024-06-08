#pragma once
#include <atomic>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "message.h"
#include "network_handler.h"

const int LEADER_TIMEOUT_LB_MS = 300;
const int LEADER_TIMEOUT_UB_MS = 500;

const int CANDIDATE_WIN_CHECK_FREQ = 50;
const int CANDIDATE_TIMEOUT_LB_MS = 300;
const int CANDIDATE_TIMEOUT_UB_MS = 500;

const int LEADER_HEARTBEAT_FREQ_MS = 50;

class Follower;
class Candidate;
class Leader;

#define STATE_GETTER(type, name) \
    type get_##name() { return state->name; }

#define GETTER(type, name) \
    type get_##name() { return name; }

const uint32_t ELECTION_TIMEOUT = 100;

enum class SERVER_TYPE {
    FOLLOWER,
    CANDIDATE,
    LEADER,
};

struct ServerState {
    std::string server_id; // your server id
    NetworkHandler* network_handler; // the network handler

    uint64_t cur_term; // the current term 

    std::optional<std::string> voted_for; // for term "cur_term", who you voted for
    std::optional<std::string> cur_leader_id; // for term "cur_term", who is the leader

    uint64_t last_commit; // highest log idx known commited by leader
    uint64_t last_applied; // highest log entry applied to state machine

    std::vector<Log> logs; // all logs
    std::map<uint64_t,uint64_t> machine_state; // the state (commited only)

    ServerState(std::string server_id, NetworkHandler* network_handler);
};


class Server {
protected:
    std::shared_ptr<ServerState> state;
    
public:
    Server(std::string server_id, NetworkHandler* network_handler);
    Server(std::shared_ptr<ServerState> state);
    virtual ~Server() = default;
    
    // im lazy
    STATE_GETTER(std::string, server_id); 
    STATE_GETTER(uint64_t, cur_term);
    STATE_GETTER(std::optional<std::string>, cur_leader_id);
    STATE_GETTER(uint64_t, last_commit);
    STATE_GETTER(uint64_t, last_applied);
    STATE_GETTER(std::vector<Log>&, logs);
    GETTER(std::shared_ptr<ServerState>, state);

    // all server types start here
    virtual SERVER_TYPE main() = 0;

    // applies logs from state->last_applied to state->last_commit
    void apply_logs();
};

class Follower : public Server {
private:
    std::atomic<bool> heartbeat_recieved; // if we recieved a heartbeat
    std::atomic<bool> shutdown; // shut down false == server online

    void process_logic(); // all follower logic, called from main()
    void timeout_timer(); // checks for heartbeats and timeouts, called from main()

    // called from process_logic()
    void process_append_entries_msg(Peer sender, AppendEntriesMessage msg);
    void process_append_entries_reply(Peer sender, AppendEntriesReply msg);
    void process_request_vote_msg(Peer sender, RequestVoteMessage msg);
    void process_request_vote_reply(Peer sender, RequestVoteReply msg);

public:
    Follower(std::string server_id, NetworkHandler* network_handler);
    Follower(std::shared_ptr<ServerState> state);
    Follower(Server& server);

    // runs all follower logic. if it returns, wants to switch to different SERVER_TYPE.
    SERVER_TYPE main() override;
};


class Candidate : public Server {
public:
    // all ways eleciton results can go
    enum class ELECTION_RESULT {
        SUCCESS,
        FAILURE,
        TIMEOUT,
    };
private:
    SERVER_TYPE next_state;  // if we should change to follower or leader
    uint64_t votes_obtained; // # of votes we have received
    std::atomic<bool> msg_recieved; // if we recieved a request vote
    std::atomic<bool> reelection_timeout; // if we've timed out
    std::atomic<bool> change_state; // if the candidate should stop being a candidate

    void process_logic(); // all candidate logic, called from main()
    void reelection_timer(); // checks for election timeout, called from main()

    void send_vote_request(); // requests votes from peers, called from process_logic()
    ELECTION_RESULT recieve_replies(); // recievees votes from peers, called from process_logic()

    // called from recieve_rplies()
    std::optional<ELECTION_RESULT> process_append_entries_msg(Peer sender, AppendEntriesMessage msg);
    std::optional<ELECTION_RESULT> process_append_entries_reply(Peer sender, AppendEntriesReply msg);
    std::optional<ELECTION_RESULT> process_request_vote_msg(Peer sender, RequestVoteMessage msg);
    std::optional<ELECTION_RESULT> process_request_vote_reply(Peer sender, RequestVoteReply msg);
public:     
    Candidate(std::shared_ptr<ServerState> state);

    // runs all Candidate logic. if it returns, wants to switch to different SERVER_TYPE.
    SERVER_TYPE main() override;
     
};

class Leader : public Server {
private:
    std::vector<uint64_t> next_index; // for each peer, the next_index to send
    std::vector<uint64_t> match_index; // for each peer, the highest index replciated in their log

    // msgid -> (prev_log_idx we sent, highest_idx in msg [0 if no logs])
    std::map<uint64_t, std::pair<uint64_t, uint64_t>> msgid_to_logidx; 

    std::atomic<bool> shutdown; // if we should shut down (and demote to follower)

    void recieve_nodes(); // recieves and processes msgs from peers, called from main()
    void recieve_client(); // recieves and processes msgs from client, called from main()
    void send_heartbeats(); // sends heartbeats to peers, called from main()
    
    
    // called from recieve_nodes()
    bool process_append_entries_msg(Peer sender, AppendEntriesMessage msg);
    bool process_append_entries_reply(Peer sender, AppendEntriesReply msg);
    bool process_request_vote_msg(Peer sender, RequestVoteMessage msg);
    bool process_request_vote_reply(Peer sender, RequestVoteReply msg);
public:
    Leader(std::shared_ptr<ServerState> state);
    // runs all Candidate logic. if it returns, wants to switch to different SERVER_TYPE.
    SERVER_TYPE main() override;
};
