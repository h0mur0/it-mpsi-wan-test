#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <unordered_set>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "database.h"
#include <arpa/inet.h>  // for inet_pton
#include "public_function.h" // for dot_product

using namespace std;

// 构造函数实现
database::database(string filename, int client_id, int database_id, string role)
    :client_id(client_id), database_id(database_id), role(role) {
        ifstream file(filename);
        string line;
        while (getline(file, line)) {
            // 处理行中的空格，可能需要去掉前后空格
            string data = line;
            data.erase(0, data.find_first_not_of(" \t"));
            data.erase(data.find_last_not_of(" \t") + 1);
            P_db.push_back(stoi(data));
        }
    }

void database::preprocessing(int L, int b, int eta) {
    // 预处理步骤
    CuckooHashTableProducer producer_table(b);
    int j = database_id;
    int start = j * eta;
    int end = (j + 1)  * eta;
    for (int k = start; k < end; k++) {
        producer_table.insert(k);
    }
    auto index_hash_table = producer_table.table;
    incidence_vectors = vector<vector<int>>(b, vector<int>(3, 0));
    std::unordered_set<int> pset(P_db.begin(), P_db.end());
    for (int ell = 0; ell < b; ell++) {
        for (int k = 0; k < 3; k++) {
            int tmp = index_hash_table[ell][k];
            if (pset.count(tmp)) {
                incidence_vectors[ell][k] = 1;
            }
        }
    }
}

// 序列化数据
std::vector<char> serialize_data(const std::vector<int>& data) {
    std::vector<char> buffer;
    // 写入vector大小
    size_t size = data.size();
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&size), 
                 reinterpret_cast<const char*>(&size) + sizeof(size));
    
    // 写入实际数据
    if (size > 0) {
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(data.data()), 
                     reinterpret_cast<const char*>(data.data()) + size * sizeof(int));
    }
    return buffer;
}

// 发送回复给leader
void database::send_reply_to_leader() {
    // 连接到leader
    int sock = connect_to_port("127.0.0.1", 9000);
    if (sock < 0) {
        perror("connect to leader failed");
        return;
    }
    
    // 创建消息头
    ReplyHeader header;
    header.client_id = client_id;
    header.database_id = database_id;
    header.role = (role == "base") ? 0 : 1;
    header.data_size = database_send_to_leader.size();
    
    // 序列化数据
    auto data_buffer = serialize_data(database_send_to_leader);
    size_t data_size = data_buffer.size();
    
    // 发送消息头
    size_t total_sent = 0;
    while (total_sent < sizeof(ReplyHeader)) {
        ssize_t sent = send(sock, 
                           reinterpret_cast<const char*>(&header) + total_sent, 
                           sizeof(ReplyHeader) - total_sent, 0);
        if (sent < 0) {
            perror("send header failed");
            close(sock);
            return;
        }
        total_sent += sent;
    }
    
    // 发送数据大小
    if (send(sock, &data_size, sizeof(data_size), 0) != sizeof(data_size)) {
        perror("send data size failed");
        close(sock);
        return;
    }
    
    // 发送实际数据
    total_sent = 0;
    while (total_sent < data_size) {
        ssize_t sent = send(sock, data_buffer.data() + total_sent, 
                           data_size - total_sent, 0);
        if (sent < 0) {
            perror("send data failed");
            break;
        }
        total_sent += sent;
    }
    
    cout << "Sent reply from client " << client_id 
         << ", database " << database_id 
         << ", role " << role 
         << ", data size: " << data_size << endl;
    
    close(sock);
}


// 创建并发送回复
void database::create_and_send_reply(int L, int b, int N) {
    if (role == "not base") {
        for (int ell = 0; ell < b; ell++) {
            int reply = global_randomness * (dot_product(incidence_vectors[ell], database_recv_from_leader[ell]) + location_randomness[ell] + relatived_randomness[ell]) % L;
            database_send_to_leader.push_back(reply);
        }
    }
    else {
        for (int ell = 0; ell < b; ell++) {
            int reply = global_randomness * (dot_product(incidence_vectors[ell], database_recv_from_leader[ell]) + location_randomness[ell]) % L;
            database_send_to_leader.push_back(reply);
        }
    }
    
    // 发送回复给leader
    send_reply_to_leader();
}

// 连接到指定端口
int database::connect_to_port(const char* ip, int port) {
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
