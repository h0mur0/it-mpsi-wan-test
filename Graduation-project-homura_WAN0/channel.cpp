#include <vector>
#include <iostream>
#include <type_traits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "channel.h"

using namespace std;
extern long long com_bit;


// 构造函数实现
channel::channel() {}

// 序列化数据
std::vector<char> serialize_data(const std::vector<std::vector<int>>& data) {
    std::vector<char> buffer;
    size_t outer_size = data.size();
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&outer_size), 
                 reinterpret_cast<const char*>(&outer_size) + sizeof(outer_size));
    
    for (const auto& inner : data) {
        size_t inner_size = inner.size();
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&inner_size), 
                     reinterpret_cast<const char*>(&inner_size) + sizeof(inner_size));
        
        if (inner_size > 0) {
            buffer.insert(buffer.end(), reinterpret_cast<const char*>(inner.data()), 
                         reinterpret_cast<const char*>(inner.data()) + inner_size * sizeof(int));
        }
    }
    return buffer;
}

// 二维向量反序列化实现
std::vector<std::vector<int>> deserialize_data_2d(const char* buffer, size_t length) {
    const char* ptr = buffer;
    std::vector<std::vector<int>> result;
    
    // 读取外层vector大小
    size_t outer_size = *reinterpret_cast<const size_t*>(ptr);
    ptr += sizeof(size_t);
    result.resize(outer_size);
    
    for (size_t i = 0; i < outer_size; i++) {
        // 读取内层vector大小
        size_t inner_size = *reinterpret_cast<const size_t*>(ptr);
        ptr += sizeof(size_t);
        result[i].resize(inner_size);
        
        // 读取实际数据
        if (inner_size > 0) {
            memcpy(result[i].data(), ptr, inner_size * sizeof(int));
            ptr += inner_size * sizeof(int);
        }
    }
    return result;
}

// 反序列化数据
std::vector<std::vector<std::vector<int>>> deserialize_data(const char* buffer, size_t length) {
    const char* ptr = buffer;
    std::vector<std::vector<std::vector<int>>> result;
    
    // 读取外层vector大小
    size_t outer_size = *reinterpret_cast<const size_t*>(ptr);
    ptr += sizeof(size_t);
    result.resize(outer_size);
    
    for (size_t i = 0; i < outer_size; i++) {
        // 读取中层vector大小
        size_t middle_size = *reinterpret_cast<const size_t*>(ptr);
        ptr += sizeof(size_t);
        result[i].resize(middle_size);
        
        for (size_t j = 0; j < middle_size; j++) {
            // 读取内层vector大小
            size_t inner_size = *reinterpret_cast<const size_t*>(ptr);
            ptr += sizeof(size_t);
            result[i][j].resize(inner_size);
            
            // 读取实际数据
            if (inner_size > 0) {
                memcpy(result[i][j].data(), ptr, inner_size * sizeof(int));
                ptr += inner_size * sizeof(int);
            }
        }
    }
    return result;
}

// 连接到指定端口
int connect_to_port(const char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    return sockfd;
}

// leader 到 database 的数据传输
void channel::leader_to_database(leader& ld, database& db) {
    // 序列化数据
    std::vector<char> send_buffer;
    if (db.role == "base") {
        send_buffer = serialize_data(ld.leader_send_to_cb[db.client_id][db.database_id]);
    } else {
        send_buffer = serialize_data(ld.leader_send_to_tb[db.client_id][db.database_id]);
    }
    
    // 连接到database
    int port = 8000 + db.client_id * 100 + db.database_id * 2 + (db.role == "base" ? 0 : 1);
    int sock = connect_to_port("127.0.0.1", port);
    
    // 发送数据大小
    size_t data_size = send_buffer.size();
    if (send(sock, &data_size, sizeof(data_size), 0) != sizeof(data_size)) {
        perror("send data size failed");
        close(sock);
        return;
    }
    
    // 发送实际数据
    size_t total_sent = 0;
    while (total_sent < data_size) {
        ssize_t sent = send(sock, send_buffer.data() + total_sent, data_size - total_sent, 0);
        if (sent < 0) {
            perror("send failed");
            break;
        }
        total_sent += sent;
    }
    
    close(sock);
    com_bit += data_size;
}

// database 到 leader 的数据传输
void channel::database_to_leader(database& db, leader& ld) {
    // 此函数在database端实现
}
