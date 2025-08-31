#include <vector>
#include <random>
#include <cmath>
#include <stdexcept>
#include <string>
#include <iostream>
#include <cstdlib>
#include <map>
#include <fstream>
#include <sstream>
#include "public_function.h"


using namespace std;

int mod_exp(int a, int b, int m) {
    int result = 1;
    a = a % m;  // 防止a大于m

    while (b > 0) {
        if (b % 2 == 1) {  // 如果b是奇数
            result = (result * a) % m;
        }
        a = (a * a) % m;  // 将a平方
        b /= 2;  // b右移一位，相当于除以2
    }

    return result;
}

// 生成一个长度为 K 的随机向量，每个元素在 [0, L-1] 范围内
vector<int> generate_random_vector(int L, int K) {
    vector<int> random_vector(K);
    for (int i = 0; i < K; i++) {
        random_vector[i] = rand() % L;  // 生成 [0, L-1] 范围内的随机数
    }
    return random_vector;
}

// 计算两个向量的点积
int dot_product(const vector<int>& vec1, const vector<int>& vec2) {
    if (vec1.size() != vec2.size()) {
        throw invalid_argument("两个向量的长度必须相同");
    }
    int result = 0;
    for (size_t i = 0; i < vec1.size(); i++) {
        result += vec1[i] * vec2[i];
    }
    return result;
}

// 判断一个数是否为素数，使用 Miller-Rabin 算法
bool is_prime(int n, int k) {
    if (n <= 1) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    int d = n - 1;
    int s = 0;
    while (d % 2 == 0) {
        d /= 2;
        s++;
    }

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(2, n - 2);

    for (int i = 0; i < k; i++) {
        int a = dis(gen);
        int x = mod_exp(a, d, n);
        if (x == 1 || x == n - 1) continue;
        for (int j = 0; j < s - 1; j++) {
            x = (x * x) % n;
            if (x == n - 1) break;
        }
        if (x != n - 1) return false;
    }
    return true;
}

// 查找大于 M 的下一个素数
int select_L(int M) {
    while (!is_prime(M)) {
        M++;
    }
    return M;
}

// 选择最优的领导者
int select_leader(const vector<vector<int>>& P, const vector<int>& N, const int& M) {
    int t = 0;
    int min_cost = 0;

    // 计算选择每个客户端作为领导者时的代价
    auto get_cost = [&](int i) {
        int sum = 0;
        for (int j = 0; j < M; j++) {
            if (j != i) {
                sum += ceil((double)P[i].size() * N[j] / (N[j] - 1));
            }
        }
        return sum;
    };

    // 计算每个可能的领导者的代价，选择代价最小的领导者
    for (int i = 0; i < M; i++) {
        int cost = get_cost(i);
        if (cost < min_cost || min_cost == 0) {
            min_cost = cost;
            t = i;
        }
    }
    return t;
}


// 打印帮助信息
void print_help() {
    cout << "命令行参数说明:" << endl;
    cout << "-m [正整数a]    : 将a赋值给变量M" << endl;
    cout << "-t [a个文件名]  : 将a个文件名赋值给字符串向量" << endl;
    cout << "-n [a个整数]    : 将a个整数赋值给整型向量" << endl;
    cout << "-h              : 输出帮助信息" << endl;
}

// 解析命令行参数的函数
void parse_args(int argc, char* argv[], int& M, vector<string>& fileNames, vector<int>& N, int & K, string net) {
    // 默认值
    M = -1;
    fileNames.clear();
    N.clear();

    // 命令行参数解析
    int i = 1;
    while (i < argc) {
        string arg = argv[i];

        if (arg == "-m") {
            if (i + 1 < argc) {
                M = stoi(argv[i + 1]);
                if (M <= 0) {
                    cerr << "错误: -m 参数后的值必须是正整数。" << endl;
                    exit(1);
                }
                i += 2;
            } else {
                cerr << "错误: -m 后缺少参数。" << endl;
                exit(1);
            }
        }
        else if (arg == "-t") {
            if (i + 1 < argc) {
                //int count = stoi(argv[i + 1]);
                fileNames.clear();
                i += 1;

                for (int j = 0; j < M; j++) {
                    if (i < argc) {
                        fileNames.push_back(argv[i]);
                        i++;
                    } else {
                        cerr << "错误: -t 后缺少足够的文件名。" << endl;
                        exit(1);
                    }
                }
            } else {
                cerr << "错误: -t 后缺少参数。" << endl;
                exit(1);
            }
        }
        else if (arg == "-n") {
            if (i + 1 < argc) {
                //int count = stoi(argv[i + 1]);
                N.clear();
                i += 1;

                for (int j = 0; j < M; j++) {
                    if (i < argc) {
                        N.push_back(stoi(argv[i]));
                        i++;
                    } else {
                        cerr << "错误: -n 后缺少足够的整数。" << endl;
                        exit(1);
                    }
                }
            } else {
                cerr << "错误: -n 后缺少参数。" << endl;
                exit(1);
            }
        }
        else if (arg == "-K") {
            if (i + 1 < argc) {
                K = stoi(argv[i + 1]);
                i += 2;
            } else {
                cerr << "错误: -K 后缺少参数。" << endl;
                exit(1);
            }
        }
        else if (arg == "-net") {
            if (i + 1 < argc) {
                string value = argv[i + 1];
                if (value == "WAN") net = "WAN";
                else if (value == "LAN") net = "LAN";
                else{
                    cerr << "错误：无效的网络模式。" << endl;
                    exit(1);
                }
            }
            else {
                cerr << "错误：-net 后缺少参数。" << endl;
                exit(1);
            }
        }
        else if (arg == "-h") {
            cout << "帮助文档:" << endl;
            cout << "-m [正整数a]    设置M的值为正整数a。" << endl;
            cout << "-t [a个文件名]  设置文件名列表，后面必须跟a个文件名。" << endl;
            cout << "-n [a个整数]    设置整数列表，后面必须跟a个整数。" << endl;
            cout << "-h              输出此帮助信息。" << endl;
            exit(0);
        }
        else {
            cerr << "错误: 未知的命令行参数 " << arg << endl;
            exit(1);
        }
    }

    // 检查参数是否完整
    if (M == -1) {
        cerr << "错误: -m 参数必须指定一个正整数。" << endl;
        exit(1);
    }
    if (fileNames.empty()) {
        cerr << "错误: -t 参数必须指定至少一个文件名。" << endl;
        exit(1);
    }
    if (N.empty()) {
        cerr << "错误: -n 参数必须指定至少一个整数。" << endl;
        exit(1);
    }
}


// encode函数
void encode(const vector<string>& fileNames, vector<int>& Sk, map<string, int>& data2Sk, vector<vector<int>>& P, int& K) {
    // 遍历每个文件名
    for (int i = 0; i < fileNames.size(); ++i) {
        ifstream file(fileNames[i]);
        string line;
        vector<int> currentFileData;

        // 逐行读取文件中的数据
        while (getline(file, line)) {
            // 处理行中的空格，可能需要去掉前后空格
            string data = line;
            data.erase(0, data.find_first_not_of(" \t"));
            data.erase(data.find_last_not_of(" \t") + 1);

            // // 判断该数据是否已经存在于data2Sk中
            // if (data2Sk.find(data) != data2Sk.end()) {
            //     // 如果存在，将当前数据的Sk值加入P[i]中
            //     int existingSk = data2Sk[data];
            //     currentFileData.push_back(existingSk);
            // } else {
            //     // 如果不存在，新增一个Sk值
            //     int newSk = Sk.size();
            //     Sk.push_back(newSk);
            //     data2Sk[data] = newSk;
            //     currentFileData.push_back(newSk);
            // }
        }
        P.push_back(currentFileData);
    }

    // 设置K为Sk的长度
    // K = Sk.size();
}

// decode函数
void decode(const vector<int>& intersection, const map<string, int>& data2Sk, vector<string>& intersection_string) {
    // 遍历每个整数，将对应的字符串加入intersection_string中
    for (int a : intersection) {
        for (const auto& pair : data2Sk) {
            if (pair.second == a) {
                intersection_string.push_back(pair.first);
                break;
            }
        }
    }
}


uint64_t fnv1a_64(const char* data, size_t len) {
    const uint64_t prime = 0x00000100000001B3; // 1099511628211
    uint64_t hash = 0xCBF29CE484222325;         // 14695981039346656037

    for (size_t i = 0; i < len; ++i) {
        hash ^= static_cast<uint64_t>(data[i]);
        hash *= prime;
    }
    return hash;
}

uint64_t murmur3_64(const char* key, uint64_t len, uint64_t seed = 0) {
    const uint64_t c1 = 0xcc9e2d51;
    const uint64_t c2 = 0x1b873593;
    const uint64_t r1 = 15;
    const uint64_t r2 = 13;
    const uint64_t m = 5;
    const uint64_t n = 0xe6546b64;

    uint64_t hash = seed;
    const uint64_t* blocks = (const uint64_t*)key;
    for (uint64_t i = 0; i < len / 4; i++) {
        uint64_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = (hash << r2) | (hash >> (32 - r2));
        hash = hash * m + n;
    }

    // 处理剩余字节
    const uint8_t* tail = (const uint8_t*)(key + (len / 4) * 4);
    uint64_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << r1) | (k1 >> (32 - r1));
                k1 *= c2;
                hash ^= k1;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}


CuckooHashTableConsumer::CuckooHashTableConsumer(int initSize, int maxSteps)
    : size(initSize), maxSteps(maxSteps) {
    table.assign(size, -1);
}

int CuckooHashTableConsumer::hash1(int key) {
    string h = to_string(key);
    uint64_t hash = fnv1a_64(h.data(),h.size());
    return hash % size;
}

int CuckooHashTableConsumer::hash2(int key) {
    string h = to_string(key);
    uint64_t hash = murmur3_64(h.data(),h.size(), 0x1234);
    return hash % size;
}

int CuckooHashTableConsumer::hash3(int key) {
    string h = to_string(key);
    uint64_t hash = murmur3_64(h.data(),h.size(), 0x5678);
    return hash % size;
}

std::vector<int> CuckooHashTableConsumer::hashFunctions(int key) {
    // auto h1 = hash1(key);
    // auto h2 = hash2(key);
    // auto h3 = hash3(key);
    // if (h2 == h1){
    //     h2 =  (h2 + 1) % size;
    // }
    // while (h3 == h1 or h3 == h2){
    //     h3 = (h3 + 1) % size;
    // }
    // return {h1, h2, h3};
    size_t h1 = std::hash<int>{}(key);
    size_t h2 = h1 ^ 0x9e3779b97f4a7c15ULL;
    size_t h3 = h2 ^ 0xc6a4a7935bd1e995ULL;
    return {int(h1 % size), int(h2 % size), int(h3 % size)};
}

void CuckooHashTableConsumer::rehash() {
    std::vector<int> oldData;
    for (int key : table) {
        if (key != -1) oldData.push_back(key);
    }
    size *= 2;
    table.assign(size, -1);
    for (int key : oldData) {
        insert(key);
    }
}

bool CuckooHashTableConsumer::insert(int key) {
    for (int i = 0; i < maxSteps; ++i) {
        auto hs = hashFunctions(key);
        for (int idx : hs) {
            if (table[idx] == -1) {
                table[idx] = key;
                return true;
            }
        }

        int index = hs[rand() % 3];
        std::swap(table[index], key);
    }
    rehash();
    return insert(key);
}

bool CuckooHashTableConsumer::search(int key) {
    for (int idx : hashFunctions(key)) {
        if (table[idx] == key) return true;
    }
    return false;
}

bool CuckooHashTableConsumer::remove(int key) {
    for (int idx : hashFunctions(key)) {
        if (table[idx] == key) {
            table[idx] = -1;
            return true;
        }
    }
    return false;
}

void CuckooHashTableConsumer::display() {
    std::cout << "Table: ";
    for (int val : table) {
        if (val == -1)
            std::cout << " - ";
        else
            std::cout << " " << val << " ";
    }
    std::cout << std::endl;
}



// CuckooHashTableProducer Implementation

CuckooHashTableProducer::CuckooHashTableProducer(int size)
    : size(size){
        table.assign(size,vector<int>(3, -1));
    }

int CuckooHashTableProducer::hash1(int key) {
    string h = to_string(key);
    uint64_t hash = fnv1a_64(h.data(),h.size());
    return hash % size;
}

int CuckooHashTableProducer::hash2(int key) {
    string h = to_string(key);
    uint64_t hash = murmur3_64(h.data(),h.size(), 0x1234);
    return hash % size;
}

int CuckooHashTableProducer::hash3(int key) {
    string h = to_string(key);
    uint64_t hash = murmur3_64(h.data(),h.size(), 0x5678);
    return hash % size;
}

std::vector<int> CuckooHashTableProducer::hashFunctions(int key) {
    // auto h1 = hash1(key);
    // auto h2 = hash2(key);
    // auto h3 = hash3(key);
    // if (h2 == h1){
    //     h2 =  (h2 + 1) % size;
    // }
    // while (h3 == h1 or h3 == h2){
    //     h3 = (h3 + 1) % size;
    // }
    // return {h1, h2, h3};
    size_t h1 = std::hash<int>{}(key);
    size_t h2 = h1 ^ 0x9e3779b97f4a7c15ULL;
    size_t h3 = h2 ^ 0xc6a4a7935bd1e995ULL;
    return {int(h1 % size), int(h2 % size), int(h3 % size)};
}

void CuckooHashTableProducer::insert(int key) {
    int cnt = 0;
    for (int idx : hashFunctions(key)) {
        table[idx][cnt] = key;
        cnt++;
    }
}

void CuckooHashTableProducer::display() {
    for (int i = 0; i < size; i++) {
        std::cout << "Index " << i << ": " << table[i].size() << std::endl;
    }
}

void add_plain(const int* A, const int* B, int* C,
    size_t N, size_t b) {
    size_t total = N * b * 3;
    const int* pa = A;
    const int* pb = B;
    int* pc = C;
    for (size_t i = 0; i < total; ++i) {
        pc[i] = pa[i] + pb[i];
    }
}