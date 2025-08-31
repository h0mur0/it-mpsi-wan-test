#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include <unordered_set>
#include "leader.h"

class database {
public:
	database() = default;
    std::vector<int> P_db;  // 数据库中的元素集合
    std::vector<std::vector<int>> incidence_vectors;  // 关联向量
    std::vector<std::vector<int>> database_recv_from_leader;  // 从leader接收的数据
    std::vector<int> database_send_to_leader;  // 发送给leader的数据
    std::vector<int> location_randomness;  // 本地随机数
    std::vector<int> relatived_randomness;  // 相关随机数
    int global_randomness;  // 全局随机数
    int client_id;
    int database_id;
    std::string role;  // "base" 或 "not base"

    database(std::string filename, int client_id, int database_id, std::string role);
    void preprocessing(int L, int b, int eta);
    void create_and_send_reply(int L, int b, int N);
    
    void send_reply_to_leader();
    int connect_to_port(const char* ip, int port);
};

#endif
