#ifndef CLIENT_H
#define CLIENT_H
#include <vector>
#include "public_function.h"

class client {
public:
    int client_id;  // 客户端ID
    std::string state;  // 客户端状态
    std::vector<std::vector<int>> client_send_to_database;  // 向数据库发送的随机数

    // 构造函数，初始化客户端状态和ID
    client() = default;
    client(std::string state, int client_id, int N);

    // 创建并发送本地随机数
    // void create_and_send_local_randomness(int L, int b, int N);
};

#endif // CLIENT_H
