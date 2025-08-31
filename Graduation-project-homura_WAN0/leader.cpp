#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "leader.h"
#include <numeric>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <thread>
#include <fstream>
#include <random>
#include <string>
#include <unordered_set>
#include <omp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <atomic>
#include <arpa/inet.h>  // for inet_pton
#include "public_function.h" // for dot_product

using namespace std;

// 在 leader.cpp 中添加
std::vector<int> deserialize_data_1d(const char* buffer, size_t length) {
    const char* ptr = buffer;
    std::vector<int> result;
    
    // 读取vector大小
    size_t size = *reinterpret_cast<const size_t*>(ptr);
    ptr += sizeof(size_t);
    result.resize(size);
    
    // 读取实际数据
    if (size > 0) {
        memcpy(result.data(), ptr, size * sizeof(int));
    }
    return result;
}

// 构造函数实现
leader::leader(string filename, int leader_id, int M, int N, int b){
    leader_id = leader_id;
        ifstream file(filename);
        string line;
        while (getline(file, line)) {
            // 处理行中的空格，可能需要去掉前后空格
            string data = line;
            data.erase(0, data.find_first_not_of(" \t"));
            data.erase(data.find_last_not_of(" \t") + 1);
            P_leader.push_back(stoi(data));
        }
        leader_recv_from_cb = vector<vector<vector<int>>>(M, vector<vector<int>>(N));
        leader_recv_from_tb = vector<vector<vector<int>>>(M, vector<vector<int>>(N));
        control_queries = vector<vector<vector<vector<int>>>>(M, vector<vector<vector<int>>>(N, vector<vector<int>>(b, vector<int>(3))));
        targeted_queries = vector<vector<vector<vector<int>>>>(M, vector<vector<vector<int>>>(N, vector<vector<int>>(b, vector<int>(3))));
        schedule_hash_tables = vector<vector<int>>(N);
    }

void leader::preprocessing(int N, int M, int b, int eta, int L) {
    vector<CuckooHashTableProducer> producer_tables(N, CuckooHashTableProducer(b));
    vector<CuckooHashTableConsumer> consumer_tables(N, CuckooHashTableConsumer(b));
    vector<vector<vector<int>>> index_hash_tables(N);
    // 计时
    auto t_start = chrono::high_resolution_clock::now();
    for (int j = 0; j < N; j++){
        int start = j * eta;
        int end = (j + 1) * eta;
        for (int k = start; k < end; k++) {
            producer_tables[j].insert(k);
            consumer_tables[j].insert(k);
        }
        schedule_hash_tables[j] = consumer_tables[j].table;
        index_hash_tables[j] = producer_tables[j].table;
    }
    auto t_end = chrono::high_resolution_clock::now();
    long long total_us = chrono::duration_cast<chrono::microseconds>(t_end - t_start).count();
    cout << "预处理1: " << total_us << " 微秒" << endl;
    unordered_set<int> pset(P_leader.begin(), P_leader.end());
    vector<vector<vector<int>>> pre_query_vectors(N, vector<vector<int>>(b, vector<int>(3, 0)));
    for (int j = 0; j < N; j++) {
        for (int ell = 0; ell < b; ell++) {
                int tmp = schedule_hash_tables[j][ell];
                if (tmp == -1) continue; // 跳过未使用的槽位             
                if (pset.count(tmp)) {
                    int k = find(index_hash_tables[j][ell].begin(),index_hash_tables[j][ell].end(),tmp) - index_hash_tables[j][ell].begin();
                    pre_query_vectors[j][ell][k] = 1;             
            }
        }
    } 
    //计时
    auto t_end1 = chrono::high_resolution_clock::now();
    long long total_us1 = chrono::duration_cast<chrono::microseconds>(t_end1 - t_end).count();
    cout << "预处理2: " << total_us1 << " 微秒" << endl;
    vector<int> control_query;
    vector<int> targeted_query;
     thread_local static std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, L - 1);

    // 2) 并行化三重循环（需编译时加 -fopenmp）
    #pragma omp parallel for collapse(3) schedule(static)
    for (int i = 0; i < M - 1; i++) {
        for (int j = 0; j < N; j++) {
            for (int ell = 0; ell < b; ell++) {
                // 3) 直接生成并写入三元数组，避免 vector 分配/复制
                auto &slot = control_queries[i][j][ell];
                slot[0] = dist(rng);
                slot[1] = dist(rng);
                slot[2] = dist(rng);
            }
        }
    }
    auto t_end2 = chrono::high_resolution_clock::now();
    long long total_us2 = chrono::duration_cast<chrono::microseconds>(t_end2 - t_end1).count();
    cout << "预处理3: " << total_us2 << " 微秒" << endl;
    targeted_queries = control_queries;           
    for (int i = 0; i < M - 1; i++) {
        for (int j = 0; j < N; j++) {
            for (int ell = 0; ell < b; ell++) {
                // 生成目标查询
                for (int k = 0; k < 3; k++) {
                    targeted_queries[i][j][ell][k] += pre_query_vectors[j][ell][k];  // 将查询与预处理向量结合
                }
            }}}
                
    auto t_end3 = chrono::high_resolution_clock::now();
    long long total_us3 = chrono::duration_cast<chrono::microseconds>(t_end3 - t_end2).count();
    cout << "预处理4: " << total_us3 << " 微秒" << endl;
}

// 创建并发送查询
void leader::send_query() {
    leader_send_to_cb = control_queries;  // 控制查询发送给控制数据库
    leader_send_to_tb = targeted_queries;  // 目标查询发送给目标数据库
}

// 计算交集
vector<int> leader::calculate_intersection(int M, int N, int b, int L) {
    vector<int> intersection;
    unordered_set<int> pset(P_leader.begin(), P_leader.end());
    for (int j = 0; j < N; j++) {
        for (int ell = 0; ell < b; ell++) {
            int element = schedule_hash_tables[j][ell];
            if (element == -1) continue; // 跳过未使用的槽位
            if (pset.count(element) == 0) continue; // 如果元素不在 P_leader 中，跳过
            vector<int> z;
            for (int i = 0; i < M - 1; i++) {
                z.push_back(leader_recv_from_tb[i][j][ell] - leader_recv_from_cb[i][j][ell]);
            }
            int e = accumulate(z.begin(), z.end(), 0) % L;
            if (e == 0) {
                intersection.push_back(element);
            }
        }
    }
    return intersection;
}

// 创建并绑定socket
int leader::create_and_bind_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    
    // 设置地址重用
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(sockfd, 10) < 0) {
        perror("listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    return sockfd;
}

// 启动回复服务器
void leader::start_reply_server() {
    server_running = true;
    server_socket = create_and_bind_socket(9000);
    reply_thread = thread(&leader::reply_server_loop, this);
    cout << "Leader reply server started on port 9000" << endl;
}

// 停止回复服务器
void leader::stop_reply_server() {
    if (!server_running) {
        return; // 已经停止
    }
    
    server_running = false;
    cout << "Stopping reply server..." << endl;
    
    // 先关闭服务器 socket，这会使得 accept 调用失败并退出循环
    if (server_socket >= 0) {
        shutdown(server_socket, SHUT_RDWR); // 先关闭读写
        close(server_socket);
        server_socket = -1; // 标记为无效
    }
    
    // 创建虚拟连接以唤醒可能阻塞的 accept
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(9000);
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
        
        // 设置连接超时
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        
        connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        close(sock);
    }
    
    // 等待回复线程结束
    if (reply_thread.joinable()) {
        reply_thread.join();
    }
    
    cout << "Leader reply server stopped" << endl;
}

// 反序列化数据
std::vector<int> deserialize_reply(const char* buffer, size_t length) {
    const char* ptr = buffer;
    std::vector<int> result;
    
    // 读取vector大小
    size_t size = *reinterpret_cast<const size_t*>(ptr);
    ptr += sizeof(size_t);
    result.resize(size);
    
    // 读取实际数据
    if (size > 0) {
        memcpy(result.data(), ptr, size * sizeof(int));
    }
    return result;
}

// 回复服务器循环

void leader::reply_server_loop() {
    while (server_running) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        // 检查服务器 socket 是否有效
        if (server_socket < 0) {
            cerr << "Server socket is invalid, stopping loop" << endl;
            break;
        }
        int newsockfd = accept(server_socket, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            if (server_running) perror("accept failed");
            continue;
        }
        
        // 1. 接收消息头
        ReplyHeader header;
        size_t total_received = 0;
        while (total_received < sizeof(ReplyHeader)) {
            ssize_t received = recv(newsockfd, 
                                   reinterpret_cast<char*>(&header) + total_received, 
                                   sizeof(ReplyHeader) - total_received, 0);
            if (received <= 0) {
                perror("recv header failed");
                close(newsockfd);
                continue;
            }
            total_received += received;
        }
        
        // 2. 接收数据大小
        size_t data_size;
        if (recv(newsockfd, &data_size, sizeof(data_size), 0) != sizeof(data_size)) {
            perror("recv data size failed");
            close(newsockfd);
            continue;
        }
        
        // 3. 接收实际数据
        vector<char> buffer(data_size);
        total_received = 0;
        while (total_received < data_size && server_running) {
            ssize_t received = recv(newsockfd, buffer.data() + total_received, 
                                   data_size - total_received, 0);
            if (received <= 0) {
                perror("recv data failed");
                break;
            }
            total_received += received;
        }
        
        if (total_received == data_size) {
            // 4. 反序列化数据
            auto reply_data = deserialize_data_1d(buffer.data(), buffer.size());
            
            // 5. 根据消息头信息存储到正确位置
            if (header.client_id < leader_recv_from_cb.size() && 
                header.database_id < leader_recv_from_cb[header.client_id].size()) {
                
                if (header.role == 0) { // Control Database
                    leader_recv_from_cb[header.client_id][header.database_id] = reply_data;
                    cout << "Received control reply from client " << header.client_id 
                         << ", database " << header.database_id 
                         << ", size: " << reply_data.size() << endl;
                } else { // Targeted Database
                    leader_recv_from_tb[header.client_id][header.database_id] = reply_data;
                    cout << "Received targeted reply from client " << header.client_id 
                         << ", database " << header.database_id 
                         << ", size: " << reply_data.size() << endl;
                }
            } else {
                cerr << "Invalid client_id or database_id in reply header: " 
                     << header.client_id << ", " << header.database_id << endl;
            }
        } else {
            cerr << "Failed to receive complete data. Expected: " 
                 << data_size << ", Received: " << total_received << endl;
        }
        
        close(newsockfd);
    }
}
