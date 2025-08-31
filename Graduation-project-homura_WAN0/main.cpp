#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <cstdlib>
#include <map>
#include <memory>
#include <chrono>
#include <algorithm>
#include <unordered_set>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "client.h"
#include "leader.h"
#include "database.h"
#include "channel.h"
#include "public_function.h"

using namespace std;

// 全局
int M;                                // 参与方数量
int leader_id, sp_id;                
vector<string> fileNames;            
vector<int> N;                       
int K, L, b, eta;                    
vector<client> clients;              
vector<vector<database>> control_databases;
vector<vector<database>> targeted_databases;
leader ld;
shared_ptr<channel> chan;            
long long com_bit = 0;
bool running = true;
ofstream outFile("output_PBC.txt", ios::app);
mutex file_mutex;                     // 同步写文件
string net;

// 单个线程的计时结果
struct ThreadTiming {
    string name;
    long long microseconds;
};
vector<ThreadTiming> timings;
mutex timing_mutex;

// 记录某段代码的执行时间并保存
template<typename F>
void time_and_run(const string& name, F func) {
    auto t0 = chrono::high_resolution_clock::now();
    func();
    auto t1 = chrono::high_resolution_clock::now();
    long long us = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
    {
        lock_guard<mutex> lk(timing_mutex);
        timings.push_back({name, us});
    }
}

// 多线程预处理（leader单独，数据库实例并行）
void threaded_preprocessing() {
    // leader 预处理
    time_and_run("leader_preproc", [&](){
        ld.preprocessing(N[0], M, b, eta, L);
    });

    // control + targeted databases 并行
    vector<thread> ths;
    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++) {
            ths.emplace_back([i,j](){
                string nm1 = "ctrl_DB_" + to_string(i) + "_" + to_string(j) + "_preproc";
                time_and_run(nm1, [&](){
                    control_databases[i][j].preprocessing(L, b, eta);
                });

                string nm2 = "tgt_DB_" + to_string(i) + "_" + to_string(j) + "_preproc";
                time_and_run(nm2, [&](){
                    targeted_databases[i][j].preprocessing(L, b, eta);
                });
            });
        }
    }
    for (auto &t : ths) t.join();
}

// 随机数生成阶段
void create_randomness() {
    // 本地随机数
    for (int i = 0; i < M - 1; i++) {
        for(int j = 0; j < N[0]; j++){
            auto location_randomness_tmp = generate_random_vector(L, b);  // 生成本地随机数
            control_databases[i][j].location_randomness = location_randomness_tmp;  // 设置本地随机数
            targeted_databases[i][j].location_randomness = location_randomness_tmp;  // 设置目标数据库的本地随机数
        }
    }
    // 相关随机数
    //普通数据库
    for (int j = 0; j < N[0]; j++){        
        vector<vector<int>> relatived_randomness;  // 初始化相关随机数列表
        for (int i = 0; i < M-2; i++) {
            auto relatived_randomness1 = generate_random_vector(L, b);  // 生成相关随机数
            targeted_databases[i][j].relatived_randomness = relatived_randomness1;  // 设置相关随机数
            relatived_randomness.push_back(relatived_randomness1);  // 将相关随机数添加到列表中
        }
        vector<int> related_randomness2;
        for (int ell = 0; ell < b; ell++) {
            int sum = L - (M - 1);
            for (int i = 0; i < M - 2; i++) {
                sum -= relatived_randomness[i][ell];  // 累加相关随机数
            }
            related_randomness2.push_back(sum);  // 将累加结果添加到相关随机数列表中
        }
        targeted_databases[M-2][j].relatived_randomness = related_randomness2;  // 设置特殊数据库的相关随机数
    }

    // 全局随机数
    int c = rand() % (L - 1) + 1;  // 随机生成系数 c
    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++){        
            control_databases[i][j].global_randomness = c;
            targeted_databases[i][j].global_randomness = c;  // 设置全局随机数
        }
    }
}

// 多线程回复阶段
void threaded_reply() {
    vector<thread> ths;
    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++) {
            ths.emplace_back([i,j](){
                // control db reply
                string nm1 = "ctrl_DB_" + to_string(i) + "_" + to_string(j) + "_reply";
                time_and_run(nm1, [&](){
                    control_databases[i][j].create_and_send_reply(L, b, N[0]);
                });
                // targeted db reply
                string nm2 = "tgt_DB_" + to_string(i) + "_" + to_string(j) + "_reply";
                time_and_run(nm2, [&](){
                    targeted_databases[i][j].create_and_send_reply(L, b, N[0]);
                });
            });
        }
    }
    for (auto &t : ths) t.join();
}

// 创建并绑定socket
int create_and_bind_socket(int port) {
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

// Database 服务器线程函数
void database_server_thread(int client_id, int db_id, bool is_control, vector<vector<database>>& dbs) {
    int port = 8000 + client_id * 100 + db_id * 2 + (is_control ? 0 : 1);
    int sockfd = create_and_bind_socket(port);
    
    cout << "Database " << (is_control ? "Control" : "Targeted") 
         << " [" << client_id << "][" << db_id << "] listening on port " << port << endl;
    
    while (true) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);

        if (!running) {
            cout << "Database server at port " << port << " stopped" << endl;
            break;
        }

        if (newsockfd < 0) {
            perror("accept failed");
            continue;
        }
        
        // 接收数据大小
        size_t data_size;
        if (recv(newsockfd, &data_size, sizeof(data_size), 0) != sizeof(data_size)) {
            perror("recv data size failed");
            close(newsockfd);
            continue;
        }
        
        // 接收实际数据
        vector<char> buffer(data_size);
        size_t total_received = 0;
        while (total_received < data_size) {
            ssize_t received = recv(newsockfd, buffer.data() + total_received, data_size - total_received, 0);
            if (received <= 0) {
                perror("recv data failed");
                break;
            }
            total_received += received;
        }
        
        if (total_received == data_size) {
            // 反序列化数据
            auto data = deserialize_data_2d(buffer.data(), buffer.size());
            
            // 更新数据库
            if (is_control) {
                dbs[client_id][db_id].database_recv_from_leader = data;
            } else {
                dbs[client_id][db_id].database_recv_from_leader = data;
            }
        }
        
        close(newsockfd);
    }
    
    close(sockfd);
}

int main(int argc, char* argv[]) {
    // 时间戳
    time_t now = time(nullptr);
    tm* lt = localtime(&now);
    outFile << "-------------------------\n";
    outFile << "运行时间：" << put_time(lt, "%Y-%m-%d %H:%M:%S") << "\n";

    // 设置网络环境

    // 参数解析
    parse_args(argc, argv, M, fileNames, N, K, net);
    L         = select_L(M);
    leader_id = M-1;
    sp_id     = M-2;
    eta       = (K + N[0] - 1) / N[0];
    b         = (3*eta + 1) / 2;

    if (net == "LAN"){
        system("sudo ./setup_network_LAN.sh");
        outFile << "网络环境：LAN" << endl;
    }
    else if (net == "WAN"){
        system("sudo ./setup_network_WAN.sh");
        outFile << "网络环境：WAN" << endl;
    }

    clients = vector<client>(M-1);
    control_databases  = vector<vector<database>>(M-1, vector<database>(N[0]));
    targeted_databases = vector<vector<database>>(M-1, vector<database>(N[0]));
    ld = leader(fileNames[leader_id], leader_id, M, N[0], b);
    chan = make_shared<channel>();

    // 启动leader回复服务器
    ld.start_reply_server();

    // 启动数据库服务器线程
    vector<thread> server_threads;
    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++) {
            server_threads.emplace_back(database_server_thread, i, j, true, ref(control_databases));
            server_threads.emplace_back(database_server_thread, i, j, false, ref(targeted_databases));
        }
    }
    
    // 等待服务器启动
    this_thread::sleep_for(chrono::seconds(2));

    outFile << "参与方数量：" << M << "\n";
    outFile << "全集大小：" << K << "\n";
    cout << "number of participants: " << M << endl;
    cout << "universe size: " << K << endl;

    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++) {
            control_databases[i][j]  = database(fileNames[i], i, j, "base");
            targeted_databases[i][j] = database(fileNames[i], i, j, "not base");
        }
    }

    auto t_start = chrono::high_resolution_clock::now();

    // 预处理阶段
    time_and_run("setup_phase", [](){threaded_preprocessing(); });
    cout << "setup finished" << endl;

    // 随机数生成（单线程）
    time_and_run("randomness_gen", [&](){
        create_randomness();
    });
    cout << "randomness_gen finished" << endl;

    // 查询阶段（仅leader）
    time_and_run("query_phase", [&](){
        ld.send_query();
    });

    cout << "query finished" << endl;
   
    // 传查询
    time_and_run("dispatch_query", [&](){
        for (int i = 0; i < M-1; i++)
            for (int j = 0; j < N[0]; j++) {
                chan->leader_to_database(ld, control_databases[i][j]);
                chan->leader_to_database(ld, targeted_databases[i][j]);
            }
    });

    cout << "dispatch query finished" << endl;
    

    // 回复阶段（并行数据库）
    threaded_reply();

    cout << "reply finished" << endl; 

    cout << "Waiting for all replies..." << endl;
    this_thread::sleep_for(chrono::seconds(1)); 

    // 计算阶段（leader）
    vector<int> intersection;
    time_and_run("compute_intersection", [&](){
        intersection = ld.calculate_intersection(M, N[0], b, L);
    });

    cout << "compute_intersection finished" << endl;

    // 输出各阶段耗时
    {
        lock_guard<mutex> lk(file_mutex);
        for (auto &tg : timings) {
            outFile << tg.name << " 用时: " << tg.microseconds << " μs\n";
        }
        outFile << "交集大小: " << intersection.size() << "\n";
    }

    cout << "intersection is:";
    for (auto &e : intersection) {
        cout << " " << e;
    }
    cout << endl;
    auto t_end = chrono::high_resolution_clock::now();
    long long total_us = chrono::duration_cast<chrono::microseconds>(t_end - t_start).count();
    cout << "total time: " << total_us << " us" << endl;
    outFile << "总耗时: " << total_us << " 微秒\n" << endl;
    
    // 停止leader服务器
    ld.stop_reply_server();
    running = false;

    // 唤醒阻塞的accept
    for (int i = 0; i < M-1; i++) {
        for (int j = 0; j < N[0]; j++) {
            for (int k = 0; k < 2; k++) {
                int port = 8000 + i * 100 + j * 2 + k;
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in serv_addr;
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port);
                inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
                connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
                close(sock);
            }
        }
    }
    
    // 等待所有服务器线程结束
    for (auto& t : server_threads) {
        if (t.joinable()) t.join();
    }
    
    // 恢复网络设置
    system("sudo tc qdisc del dev lo root");
    
    return 0;
}
