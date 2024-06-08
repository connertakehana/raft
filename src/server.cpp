#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <thread>

#include "server.h"

ServerState::ServerState(std::string server_id, NetworkHandler* network_handler)
  : server_id(server_id), network_handler(network_handler), 
  cur_term(1),
  voted_for(std::nullopt), cur_leader_id(std::nullopt), 
  last_commit(0), last_applied(0) {}

Server::Server(std::string server_id, NetworkHandler* network_handler)
  : state(std::make_shared<ServerState>(server_id, network_handler)) {}

Server::Server(std::shared_ptr<ServerState> state)
  : state(state) {}

void Server::apply_logs() {
    assert(!state->logs.empty());
    printf("server %d APPYING LOGS\n", std::this_thread::get_id());
    // the logs we've applied should always be less equal than the log to commit until
    assert(state->last_applied <= state->last_commit);
    auto itt = std::lower_bound(state->logs.begin(), state->logs.end(), state->last_applied);
    // logs not empty, so log w/ index last_applied MUST exist, even if nothing commited to machine yet
    // AND either nothing commit to machine yet, or the lgo w/ index last_applied EXISTS
    assert(itt != state->logs.end() && (state->machine_state.empty() ||itt->idx == state->last_applied));
    if (!state->machine_state.empty())
        itt++;
    for (; itt != state->logs.end() && itt->idx <= state->last_commit; itt++) {
      state->machine_state[itt->key] = itt->value;
      state->last_applied = itt->idx;
    }
}

//—————————————————————————————————————————————————————————————————————————————---—-//
//—————————————————————————————— FOLLOWER LOGIC START ——————————————————————————————//
//—————————————————————————————————————————————————————————————————————————————---—-//
Follower::Follower(std::string server_id, NetworkHandler* network_handler)
  : Server(server_id, network_handler), shutdown(false) {}
 
Follower::Follower(std::shared_ptr<ServerState> state)
  : Server(state), shutdown(false) {}

// EVERYTHING STARTS FROM MAIN
SERVER_TYPE Follower::main() {
    // launch 2 threads: 
    // one that waits for a heartbeat timeout to occur, allowing follower to become candidate
    // one that processes all follower logic
    std::thread thread_timeout(std::bind(&Follower::timeout_timer, this));
    std::thread thread_process(std::bind(&Follower::process_logic, this));
    // if the threads exits, it means that thread_timeout caused the threads
    // to close, meaning we should become a candidate
    thread_timeout.join();
    thread_process.join();
    return SERVER_TYPE::CANDIDATE;
}

void Follower::process_logic() {
    // keep looping until the timout_timer tells us to shutdown
    while (!shutdown) {
        // printf("follower looping\n");
        Peer sender; json rpc;
        // if nothing to recieve, conitnue
        if (!state->network_handler->recieve_rpc(sender, rpc))
            continue;
        // "method" describes what type of rpc we're recieving
        assert(rpc.contains("method") && rpc["method"].is_string());
        std::string message_type = rpc["method"];
        printf("follower %d got rpc %s\n", std::this_thread::get_id(), 
              rpc.dump().c_str());
        if (message_type == "AppendEntriesMessage") {
            // if we get smth from leader, we should reset heartbeat_recieved for timeout_timer
            heartbeat_recieved.store(true);
            process_append_entries_msg(sender, rpc);
        } else if (message_type == "AppendEntriesReply") {
            process_append_entries_reply(sender, rpc);
        } else if (message_type == "RequestVoteMessage") {
            process_request_vote_msg(sender, rpc);
        } else if (message_type == "RequestVoteReply") {
            process_request_vote_reply(sender, rpc);
        } else {
            std::cerr << "message type: " << message_type << " unrecognized" << std::endl;
        }
        printf("thread %d new logs: ", std::this_thread::get_id()); 
        for (auto& log : state->logs)
            printf("%d, %d |", log.key, log.value);
        printf("\n");

        printf("thread %d new state: ", std::this_thread::get_id()); 
        for (std::pair<uint64_t, uint64_t> log : state->machine_state)
            printf("%d, %d | ", log.first, log.second);
        printf("\n");

    }
}

void Follower::process_append_entries_msg(Peer sender, AppendEntriesMessage msg) {
    bool append_entries_success = true;
    // if the leader's term is smaller, tell them
    if (msg.term < state->cur_term) {
        printf("follower %d got a smaller term, telling leader\n", std::this_thread::get_id());
        append_entries_success = false;
    }
    // if the leader's term is greater, reset who we voted for
    if (msg.term > state->cur_term)
        state->voted_for = std::nullopt;
    // if term is valid, update term and leader id
    if (msg.term >= state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = msg.sender_id;
    }
    // if we can find a log w/ the same term and indx in our logs, we can safely add the new logs (provability guraentee from raft)
    auto itt = std::lower_bound(state->logs.begin(), state->logs.end(), msg.prev_log_index);
    // if log can't be found, can't add new logs so reply false
    if (msg.prev_log_index != 0 && 
        (itt == state->logs.end() || itt->term != msg.prev_log_term || itt->idx != msg.prev_log_index))
        append_entries_success = false;
    // if everything valid, replace our old logs w/ the new ones & update "last_commit"
    if (append_entries_success) {
        // erase everything after prev_log_index
        if (msg.prev_log_index != 0)
            state->logs.erase(++itt, state->logs.end());
        // insert the new logs
        state->logs.insert(state->logs.end(), msg.logs.begin(), msg.logs.end()); 
        // update the last_commit value
        state->last_commit = std::max(state->last_commit, msg.leader_commit);
        // apply newly commited logs
        if (state->last_commit > state->last_applied) 
            apply_logs();
        
    }
    // send reply w/ if success or not
    AppendEntriesReply resp(*this, append_entries_success, msg.message_id);
    json resp_json = resp.serialize();
    state->network_handler->send_rpc(resp_json, sender);
}

void Follower::process_append_entries_reply(Peer sender, AppendEntriesReply msg) {
    // follower shouldn't get an append entries reply (since its not a leader)
    std::cerr << "follower got AppendEntriesReply" << std::endl;
    if (msg.term > state->cur_term) { // if higher term, update
        state->cur_term = msg.term;
        state->voted_for = std::nullopt; // no votes for this term yet
        state->cur_leader_id = std::nullopt; // new term, new leader unknown
    }
}
void Follower::process_request_vote_msg(Peer sender, RequestVoteMessage msg) {
  // if they have a higher term, update who we voted for to noone
  if (msg.last_log_term > state->cur_term)
    state->voted_for = std::nullopt;
  bool vote_granted = false;
  // if they have a higher term AND their log is as up to date as ours, give them a vote
  if(!state->voted_for.has_value() && // we haven't voted for anyone yet
     ((msg.last_log_term == 0 && msg.last_log_index == 0 && state->logs.empty()) || // both have no logs
        (msg.last_log_term >= state->cur_term) ||  // OR they have a higher term
        (msg.last_log_term == state->cur_term && msg.last_log_index >= state->logs.back().idx))) { // OR same term and higher index
    state->cur_term = msg.term;
    state->voted_for = sender;
    vote_granted = true;
  }
  RequestVoteReply resp(*this, vote_granted, 1);
  json resp_json = resp.serialize();
  state->network_handler->send_rpc(resp_json, sender);
  printf("follower %d granting vote %d\n", std::this_thread::get_id(), vote_granted);
}

void Follower::process_request_vote_reply(Peer sender, RequestVoteReply msg) {
    // follower shouldn't get a request vote reply (since its not a candidate)
    std::cerr << "follower got RequestVoteReply" << std::endl;
    if (msg.term > state->cur_term) { //if higher term, update
        state->cur_term = msg.term;
        state->voted_for = std::nullopt; // no votes for this term yet
        state->cur_leader_id = std::nullopt; // new term, new leader unknown
    }
}

void Follower::timeout_timer() {
  // random number generator between LEADER_TIMEOUT_LB_MS and LEADER_TIMEOUT_UB_MS
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(LEADER_TIMEOUT_LB_MS, 
                                       LEADER_TIMEOUT_UB_MS);
  while (true) {
    // reset if heartbeat was recieved
    heartbeat_recieved.store(false);  // .store for atomic clarity
    // sleep for random time
    int random_duration = dist(gen); 
    std::this_thread::sleep_for(std::chrono::milliseconds(random_duration));
    // if heartbeat not recieved during sleep, quit follower, become candidate
    if (!heartbeat_recieved) { 
      shutdown.store(true);
      return;
    }
  }
}

//——————————————————————————————————————————————————————————————————————————--———---—//
//—————————————————————————————— FOLLOWER LOGIC END ————————————————————————---——————//
//—————————————————————————————————————————————————————————————————————————————-----—//
//—————————————————————————————————————————————————————————————————————————————---—--//
//—————————————————————————————— CANDIDATE LOGIC START ——————————————————————————————//
//————————————————————————————————————————————————————————————————--—————————————---—//
 Candidate::Candidate(std::shared_ptr<ServerState> state)
  : Server(state),
  msg_recieved(false), reelection_timeout(false), change_state(false) {}

// ALL CANDIDATE LOG START AT MAIN
SERVER_TYPE Candidate::main()  {
    // no leader exists, aka why we are a candidate
    state->cur_leader_id = std::nullopt;
    std::thread thread_timeout(std::bind(&Candidate::reelection_timer, this));
    std::thread thread_process(std::bind(&Candidate::process_logic, this));

    thread_timeout.join();
    thread_process.join();
    return next_state;
}

void Candidate::process_logic() {
    while (true) { // keep retrying candidacy
        change_state.store(false); 
        state->cur_term++; // at new election, we increment our term
        state->voted_for = state->server_id; // voted for self!
        votes_obtained = 1; // we have one vote (ourself)
        send_vote_request(); // send vote request to all peers
        ELECTION_RESULT result = recieve_replies(); // we recieve replies untill win, lose, or timeout
        switch (result) { 
            case ELECTION_RESULT::SUCCESS: // got majority of votes
                next_state = SERVER_TYPE::LEADER;
                change_state.store(true); // tell reelection_timer to stop and return back
                return;
            case ELECTION_RESULT::FAILURE: // someone else becomes leader/won
                next_state = SERVER_TYPE::FOLLOWER;
                return;
            case ELECTION_RESULT::TIMEOUT: // noone won, retrying the election
                reelection_timeout.store(false); // reset reelection timeout and start again!
                break;
        }
    }
}

void Candidate::send_vote_request() {
    RequestVoteMessage msg(*this, 0);
    for (uint32_t i = 0; i < state->network_handler->get_num_peers(); i++) {
        json msg_json = msg.serialize();
        printf("candidate %d sending to %d\n", std::this_thread::get_id(), 
              i);
        state->network_handler->send_rpc(msg_json, i);
    }
}

Candidate::ELECTION_RESULT Candidate::recieve_replies() {
    // while the election hasn't timed out or we don't have a majority of votes
    while (!reelection_timeout || 
           votes_obtained < (state->network_handler->get_num_peers() + 1) / 2) {
        Peer sender; json rpc;
        // if nothing to recieve, conitnue
        if (!state->network_handler->recieve_rpc(sender, rpc))
            continue;
        printf("candidate %d got rpc %s\n", std::this_thread::get_id(), 
              rpc.dump().c_str());
        assert(rpc.contains("method") && rpc["method"].is_string());
        std::string message_type = rpc["method"];
        if (message_type == "AppendEntriesMessage") {
            std::optional<Candidate::ELECTION_RESULT> res = process_append_entries_msg(sender, rpc);
            if (res.has_value()) return res.value();
        } else if (message_type == "AppendEntriesReply") {
            std::optional<Candidate::ELECTION_RESULT> res = process_append_entries_reply(sender, rpc);
            if (res.has_value()) return res.value();;
        } else if (message_type == "RequestVoteMessage") {
            std::optional<Candidate::ELECTION_RESULT> res = process_request_vote_msg(sender, rpc);
            if (res.has_value()) return res.value();
        } else if (message_type == "RequestVoteReply") {
            std::optional<Candidate::ELECTION_RESULT> res = process_request_vote_reply(sender, rpc);
            printf("candidate got a response!!!\n");
            if (res.has_value()) printf("candidate %d is shutting down\n", std::this_thread::get_id());
            if (res.has_value()) return res.value(); 
        } else {
            std::cerr << "message type: " << message_type << " unrecognized" << std::endl;
        }
    }
    // if we have majority votes, election is success
    if (votes_obtained >= (state->network_handler->get_num_peers() + 1) / 2) {
        return Candidate::ELECTION_RESULT::SUCCESS;
    }
    // if for loop exited and we didn't get majorioty votes, was beacause of timeout
    return Candidate::ELECTION_RESULT::TIMEOUT;
}

std::optional<Candidate::ELECTION_RESULT> Candidate::process_append_entries_msg(Peer sender, AppendEntriesMessage msg) {
    // if greater term, update who we voted for
    if (msg.term > state->cur_term) 
        state->voted_for = std::nullopt;
    // set new term and leader, election has failed
    if (msg.term >= state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = msg.sender_id;
        // *NOTE* we don't reply to leader, can just let them resend it when we get back to follower status
        return Candidate::ELECTION_RESULT::FAILURE;
    }
    // msg term smaller, tell leader of our higher term
    AppendEntriesReply resp(*this, false, msg.message_id);
    json resp_json = resp.serialize();
    state->network_handler->send_rpc(resp_json, sender);
    return std::nullopt;
}

std::optional<Candidate::ELECTION_RESULT> Candidate::process_append_entries_reply(Peer sender, AppendEntriesReply msg) {
    // not supposed to get a append entries reply if we're not a leader
    std::cerr << "candidate got AppendEntriesReply" << std::endl;
    if (msg.term > state->cur_term) 
        return Candidate::ELECTION_RESULT::FAILURE;
    return std::nullopt;
}

std::optional<Candidate::ELECTION_RESULT> Candidate::process_request_vote_msg(Peer sender, RequestVoteMessage msg) {
    // if another candidate exists w/ same term, don't do anything
    if (msg.term == state->cur_term)
        return std::nullopt;
    bool vote_granted = true;
    // if their term is smaller, tell them
    if (msg.term < state->cur_term) 
        vote_granted = false;
    // if they have a higher term, stop candidacy, grant them vote
    if (msg.term > state->cur_term) {
        state->cur_term = msg.term; // update term
        state->cur_leader_id = std::nullopt; // new term, new leader unknown
        state->voted_for = msg.sender_id; // vote for them
        vote_granted = true;
    }
    RequestVoteReply resp(*this, vote_granted, 1);
    json resp_json = resp.serialize();
    state->network_handler->send_rpc(resp_json, sender);
    if (vote_granted)
        return Candidate::ELECTION_RESULT::FAILURE;
    return std::nullopt;
}

std::optional<Candidate::ELECTION_RESULT> Candidate::process_request_vote_reply(Peer sender, RequestVoteReply msg) {
    msg_recieved.store(true); // recieved a reply, update for reelection_timer
    if (msg.vote_granted == true) {
        votes_obtained++;
        if (votes_obtained >= (state->network_handler->get_num_peers() + 1) / 2) {
            return Candidate::ELECTION_RESULT::SUCCESS;
        }
    }
    // they voted no :(
    // if they have higher term, stop candidacy
    if (msg.term > state->cur_term) { 
        state->cur_term = msg.term;
        state->cur_leader_id = std::nullopt;
        state->voted_for = std::nullopt;
        return Candidate::ELECTION_RESULT::FAILURE;
    }
    // otherwise, voted no cuz outdated log or alrdy voted for someone else, can just ignore
    return std::nullopt;
}

void Candidate::reelection_timer() {
    // random number generator between LEADER_TIMEOUT_LB_MS and LEADER_TIMEOUT_UB_MS
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(CANDIDATE_TIMEOUT_LB_MS, 
                                         CANDIDATE_TIMEOUT_UB_MS);
    while (!change_state) {
        // reset if heartbeat was recieved
        msg_recieved.store(false);  // .store for atomic clarity
        // sleep for random time
        int random_duration = dist(gen); 
        // split sleeping into CANDIDATE_WIN_CHECK_FREQ chunks to ensure that we promote to leader ASAP after winning
        for (int i = 0; i < CANDIDATE_WIN_CHECK_FREQ; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(random_duration / CANDIDATE_WIN_CHECK_FREQ));
            printf("CANDIDATE AWAKE on TERM %d\n", state->cur_term);
            if (change_state) 
                return;
        }
        // if heartbeat not recieved during sleep, tell process_logic to start new election
        if (!msg_recieved) { 
            reelection_timeout.store(true);
        }
    }
}

//——————————————————————————————————————————————————————————————————————————————---//
//—————————————————————————————— CANDIDATE LOGIC END ——————————————————————————————//
//—————————————————————————————————————————————————————————————————————————————---—//
//——————————————————————————————————————————————————————————————————————————————---//
//—————————————————————————————— LEADER LOGIC START —————————————————————————-—————//
//————————————————————————————————————————————————————————————————————————————-——--//
Leader::Leader(std::shared_ptr<ServerState> state)
  : Server(state),
  next_index(state->network_handler->get_num_peers(), 
             state->logs.empty() ? 1 : state->logs.back().idx + 1),
  match_index(state->network_handler->get_num_peers(), 0),
  shutdown(false) {

    state->cur_leader_id = state->server_id;
  }

SERVER_TYPE Leader::main() {
    state->network_handler->connect_client(); // enable connection to client
    std::thread thread_nodes(std::bind(&Leader::recieve_nodes, this)); // thread that recieves msgs from peers
    std::thread thread_client(std::bind(&Leader::recieve_client, this)); // thread that recieves msgs from client
    std::thread thread_heartbeat(std::bind(&Leader::send_heartbeats, this)); // thread that sends AppenedEntries
    
    // if the threads ever exit, it means that we must revert to follower state
    thread_nodes.join();
    thread_client.join();
    thread_heartbeat.join();
    state->network_handler->disconnect_client(); // disable connection to client
    return SERVER_TYPE::FOLLOWER;
}

void Leader::recieve_nodes() {
    bool continue_leader = true;
    while (continue_leader) {
        Peer sender; json rpc;
        // async recieve rpc
        if (!state->network_handler->recieve_rpc(sender, rpc))
            continue;
        assert(rpc.contains("method") && rpc["method"].is_string());
        std::string message_type = rpc["method"];
        printf("leader %d got rpc %s\n", std::this_thread::get_id(), 
              rpc.dump().c_str());
        if (message_type == "AppendEntriesMessage") {
            continue_leader = process_append_entries_msg(sender, rpc);
        } else if (message_type == "AppendEntriesReply") {
            continue_leader = process_append_entries_reply(sender, rpc);
        } else if (message_type == "RequestVoteMessage") {
            continue_leader = process_request_vote_msg(sender, rpc);
        } else if (message_type == "RequestVoteReply") {
            continue_leader = process_request_vote_reply(sender, rpc);
        } else {
            std::cerr << "message type: " << message_type << " unrecognized" << std::endl;
        }
    }
    shutdown.store(true);
}

bool Leader::process_append_entries_msg(Peer sender, AppendEntriesMessage msg) {
    assert(msg.term != state->cur_term); // shouldn't have 2 leaders at once
    // if leader has higher term, become follower
    if (msg.term > state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = msg.sender_id;
        state->voted_for = std::nullopt;
        // *NOTE* we don't reply to leader, can just let leader resend when back to follower status
        return false; // should become follower
    }
    // msg.term < state->cur_term
    AppendEntriesReply resp(*this, false, 1); // let old leader know of newer term
    json resp_json = resp.serialize();
    state->network_handler->send_rpc(resp_json, sender);
    return true;
}

bool Leader::process_append_entries_reply(Peer sender, AppendEntriesReply msg) {
    assert(msgid_to_logidx.contains(msg.message_id));
    // lowerbound & upperbound of logs we sent them
    uint64_t prev_log_idx_sent = msgid_to_logidx[msg.message_id].first;
    uint64_t highest_index_sent = msgid_to_logidx[msg.message_id].second;  // == 0 if no logs sent
    msgid_to_logidx.erase(msg.message_id);
    uint64_t node_idx = state->network_handler->get_peer_idx(sender);
    // if they approved and atleast 1 log was sent
    if (msg.success == true && highest_index_sent != 0) {
        assert(match_index[node_idx] <= highest_index_sent);
        // set match index to the highest index sent
        match_index[node_idx] = highest_index_sent;

        // set the next_index to log after the upper bound we sent them
        // bs for highest_index_sent
        auto itt = std::lower_bound(state->logs.begin(), state->logs.end(), highest_index_sent);
        // log for log_idx_sent_up must exist
        assert(itt != state->logs.end() && itt->idx == highest_index_sent);
        // if the log is last element, next index is + 1
        if (itt == (state->logs.end() - 1)) 
            next_index[node_idx] = highest_index_sent + 1;
        else //otherwise, move itt forward and get index
            next_index[node_idx] = (itt + 1)->idx;
        // get the value replicated on majority servers
        std::vector<uint64_t> sorted_nums = match_index;
        std::sort(sorted_nums.begin(), sorted_nums.end());
        state->last_commit = sorted_nums[(sorted_nums.size() - 1) / 2];
        apply_logs();
    } 
    if (msg.success == true)
        return true; // continue leadership
    // else
    // if they have higher term, stop leadership
    if (msg.term > state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = std::nullopt;
        state->voted_for = std::nullopt;
        return false; // should become follower
    }
    // if logs are empty/prev_log_idx_sent we sent was 0, they should've accepted
    assert(!state->logs.empty() && prev_log_idx_sent != 0); 
    // the prev log term and index didn't match, need to sned them smaller next_index
    auto itt = std::lower_bound(state->logs.begin(), state->logs.end(), prev_log_idx_sent);  
    // the log index we sent them must exist (they shouldn't say "no" to 0)
    assert(itt != state->logs.end() && itt->idx == prev_log_idx_sent); 
    // just set next_idx to the prev_log_idx they rejected
    next_index[node_idx] = itt->idx;
    return true; // continue leadership

}

bool Leader::process_request_vote_msg(Peer sender, RequestVoteMessage msg) {
    bool vote_granted = false;
    // if they have higher term, grant vote
    if (msg.term > state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = std::nullopt;
        state->voted_for = msg.sender_id;
        vote_granted = true;
    } else
        vote_granted = false;
    RequestVoteReply resp(*this, vote_granted, 1);
    json resp_json = resp.serialize();
    state->network_handler->send_rpc(resp_json, sender);
    // if we granted vote, stop leadership
    return !vote_granted;
}

bool Leader::process_request_vote_reply(Peer sender, RequestVoteReply msg) {
    std::cerr << "leader got RequestVoteReply" << std::endl;
    if (msg.term > state->cur_term) {
        state->cur_term = msg.term;
        state->cur_leader_id = std::nullopt;
        state->voted_for = msg.sender_id;
        return false; // stop leadership
    }
    return true;
}

void Leader::recieve_client() {
    while (!shutdown) {
        json rpc;
        if (!state->network_handler->recieve_client_rpc(rpc))
            continue;
        printf("LEADER %d got rpc %s\n", std::this_thread::get_id(), rpc.dump().c_str());
        ClientRequest request(rpc);
        uint64_t next_index = (state->logs.empty() ? 1 : state->logs.back().idx + 1);
        state->logs.emplace_back(state->cur_term, next_index, request.key, request.value);
    }
}


void Leader::send_heartbeats() {
    uint64_t msg_id = 0;
    while (!shutdown) {
        for (int i = 0; i < state->network_handler->get_num_peers(); i++) {
            assert(next_index[i] != 0);
            assert((state->logs.empty() && next_index[i] == 1) || 
                   (next_index[i] <= (state->logs.back().idx + 1))); 
            uint64_t highest_idx; // get highest index we are sending
            // if logs are empty or the highest log entry is less than the next index to send, 
            // not sending any logs
            if (state->logs.empty() || next_index[i] > state->logs.back().idx) 
                highest_idx = 0;
            else
                highest_idx = state->logs.back().idx;
            
            // find index/term before next_index[i]
            uint64_t prev_log_idx;
            uint64_t prev_log_term;
            auto next_idx_itt = std::lower_bound(state->logs.begin(), state->logs.end(), next_index[i]);
            // if no logs, or next index is first log, prev log is 0,0
            if (state->logs.empty() || next_idx_itt == state->logs.begin()) // logs empty or first log
                prev_log_idx = 0, prev_log_term = 0;
            else
                prev_log_idx = (next_idx_itt - 1)->idx, prev_log_term = (next_idx_itt - 1)->term;
            
            msgid_to_logidx[msg_id] = {prev_log_idx, highest_idx};
            // get the vector of logs we need to send
            std::vector<Log> new_logs = highest_idx == 0 ? 
                std::vector<Log>() : std::vector<Log>(next_idx_itt, state->logs.end());
            AppendEntriesMessage msg(*this, prev_log_idx, prev_log_term, new_logs, msg_id++);
            json msg_json = msg.serialize();
            if (!msg.logs.empty())
                printf("Leader %d sending to %d, sending rpc %s\n", std::this_thread::get_id(), i,
                   msg_json.dump().c_str());
            state->network_handler->send_rpc(msg_json, i);
            
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(LEADER_HEARTBEAT_FREQ_MS));
    }
}

//——————————————————————————————————————————————————————————————————————————————//
//—————————————————————————————— LEADER LOGIC END ——————————————————————————————//
//——————————————————————————————————————————————————————————————————————————————//