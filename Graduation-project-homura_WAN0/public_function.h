#ifndef PUBLIC_FUNCTION_H
#define PUBLIC_FUNCTION_H

#include <vector>
#include <tuple>
#include <map>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <list>

using namespace std;

// 计算 a^b mod m
int mod_exp(int a, int b, int m);

// 生成一个长度为 K 的随机向量，每个元素在 [0, L-1] 范围内
vector<int> generate_random_vector(int L, int K);

// 计算两个向量的点积
int dot_product(const vector<int>& vec1, const vector<int>& vec2);

// 判断一个数是否为素数，使用 Miller-Rabin 算法
bool is_prime(int n, int k = 20);

// 查找大于 M 的下一个素数
int select_L(int M);

// 选择最优的领导者
int select_leader(const vector<vector<int>>& P, const vector<int>& N, const int& M);
void print_help();
void parse_args(int argc, char* argv[], int& M, vector<string>& fileNames, vector<int>& N, int& K, string net);
void encode(const vector<string>& fileNames, vector<int>& Sk, map<string, int>& data2Sk, vector<vector<int>>& P, int& K);
void decode(const vector<int>& intersection, const map<string, int>& data2Sk, vector<string>& intersection_string);
void add_plain(const int* A, const int* B, int* C, size_t N, size_t b);
uint64_t fnv1a_64(const char* data, size_t len);
uint64_t murmur3_64(const char* key, uint64_t len, uint64_t seed);
class CuckooHashTableConsumer {
    private:
        int size;
        int maxSteps;
    
        int hash1(int key);
        int hash2(int key);
        int hash3(int key);
        std::vector<int> hashFunctions(int key);
        void rehash();
    
    public:
        CuckooHashTableConsumer(int initSize, int maxSteps = 1000);
        bool insert(int key);
        std::vector<int> table;
        bool search(int key);
        bool remove(int key);
        void display();
    };
    
class CuckooHashTableProducer {
private:
    int size;

    int hash1(int key);
    int hash2(int key);
    int hash3(int key);
    std::vector<int> hashFunctions(int key);

public:
    CuckooHashTableProducer(int size);
    std::vector<std::vector<int>> table;
    void insert(int key);
    void display();
};

#endif // PUBLIC_FUNCTION_H
