#include <vector>
#include <cmath>
#include <string>

#include "client.h"

using namespace std;

// 构造函数实现
client::client(string state, int client_id, int N) : state(state), client_id(client_id) {
    client_send_to_database = vector<vector<int>>(N);
}

// // 创建并发送本地随机数
// void client::create_and_send_local_randomness(int L, int b, int N) {
//     auto rv = generate_random_vector(L, b);
//     for (int i = 0; i < N; i++) {
//         client_send_to_database[i] = rv;     
//     }
// }
