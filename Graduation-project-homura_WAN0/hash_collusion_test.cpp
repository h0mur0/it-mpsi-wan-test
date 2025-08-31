#include <iostream>
#include <unordered_set>
#include <random>
#include <vector>
#include <algorithm>

#include "public_function.h"

int k = 10000;
int b = (3*k + 1) / 2;
int test_time = 500;
int rehash_times = 0;
int main(){
    
    vector<CuckooHashTableConsumer> producer_tables(test_time,CuckooHashTableConsumer(b));
    for (int i =0;i<test_time;i++) {
        const int count = k;
        const int max_val = 1000000; // 可根据需要调整最大值

        std::unordered_set<int> unique_numbers;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, max_val);

        // 插入直到收集到100个不同的值
        while (unique_numbers.size() < count) {
            unique_numbers.insert(dis(gen));
        }
        for (auto element : unique_numbers) {
            producer_tables[i].insert(element);
        }
    }  
    cout << rehash_times << endl;  
}