#ifndef LEADER_H
#define LEADER_H

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <unordered_set>

// 在 leader.h 中添加
struct ReplyHeader {
    int client_id;
    int database_id;
    int role; // 0=control, 1=targeted
    size_t data_size;
};

// 在 leader.h 中添加
std::vector<int> deserialize_data_1d(const char* buffer, size_t length);

class leader {
public:
	leader() = default;
    std::vector<int> P_leader;  // leader的元素集合
    std::vector<std::vector<std::vector<int>>> leader_recv_from_cb;  // 从控制数据库接收的数据
    std::vector<std::vector<std::vector<int>>> leader_recv_from_tb;  // 从目标数据库接收的数据
    std::vector<std::vector<std::vector<std::vector<int>>>> control_queries;  // 控制查询
    std::vector<std::vector<std::vector<std::vector<int>>>> targeted_queries;  // 目标查询
    std::vector<std::vector<std::vector<std::vector<int>>>> leader_send_to_cb;  // 发送给控制数据库的数据
    std::vector<std::vector<std::vector<std::vector<int>>>> leader_send_to_tb;  // 发送给目标数据库的数据
    std::vector<std::vector<int>> schedule_hash_tables;  // 调度哈希表
    int leader_id;
    
    leader(std::string filename, int leader_id, int M, int N, int b);
    void preprocessing(int N, int M, int b, int eta, int L);
    void send_query();
    std::vector<int> calculate_intersection(int M, int N, int b, int L);
    
    // 网络服务器功能
    void start_reply_server();
    void stop_reply_server();
    
    bool server_running;
    std::thread reply_thread;
    int server_socket;
    
    int create_and_bind_socket(int port);
    void reply_server_loop();
    
    
};

#endif
