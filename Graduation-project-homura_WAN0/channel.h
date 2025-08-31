#ifndef CHANNEL_H
#define CHANNEL_H

#include <vector>
#include <string>
#include "leader.h"
#include "database.h"

class channel {
public:
    channel();
    void leader_to_database(leader& ld, database& db);
    void database_to_leader(database& db, leader& ld);
};

// 序列化和反序列化函数的声明
std::vector<char> serialize_data(const std::vector<std::vector<int>>& data);
std::vector<std::vector<std::vector<int>>> deserialize_data(const char* buffer, size_t length);
std::vector<char> serialize_data(const std::vector<int>& data);
std::vector<std::vector<int>> deserialize_data_2d(const char* buffer, size_t length); // 新增二维版本
std::vector<int> deserialize_reply(const char* buffer, size_t length);

// 网络连接辅助函数
int create_and_bind_socket(int port);
int connect_to_port(const char* ip, int port);

#endif
