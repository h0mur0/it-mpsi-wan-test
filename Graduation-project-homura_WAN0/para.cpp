#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <map>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "para.h"

using namespace std;

int M = 0;  // 参与方数量
vector<string> fileNames;  // 存数据的文件列表
vector<int> N;  // 数据库数量
vector<int> Sk; // 整型列表Sk
map<string, int> data2Sk; // 数据到Sk的映射
vector<vector<int>> P(fileNames.size()); // 编码后的数据
int K = 0; // 最终的Sk列表长度

// 打印帮助信息
void print_help() {
    cout << "命令行参数说明:" << endl;
    cout << "-m [正整数a]    : 将a赋值给变量M" << endl;
    cout << "-t [a个文件名]  : 将a个文件名赋值给字符串向量" << endl;
    cout << "-n [a个整数]    : 将a个整数赋值给整型向量" << endl;
    cout << "-h              : 输出帮助信息" << endl;
}

// 解析命令行参数
bool parse_args(int argc, char* argv[]) {
    if (argc == 1) {
        cout << "请提供命令行参数，使用 -h 查看帮助。" << endl;
        return false;
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        // 解析 -m 参数
        if (arg == "-m") {
            if (i + 1 < argc) {
                M = atoi(argv[i + 1]);
                if (M <= 0) {
                    cout << "错误：-m 后应为正整数。" << endl;
                    return false;
                }
                i++;  // 跳过下一个参数
            } else {
                cout << "错误：-m 后缺少正整数。" << endl;
                return false;
            }
        }
        // 解析 -t 参数
        else if (arg == "-t") {
            if (i + 1 < argc) {
                int count = atoi(argv[i + 1]);
                fileNames.clear();
                for (int j = 0; j < count; j++) {
                    if (i + 2 + j < argc) {
                        fileNames.push_back(argv[i + 2 + j]);
                    } else {
                        cout << "错误：-t 后缺少文件名。" << endl;
                        return false;
                    }
                }
                i += count + 1;  // 跳过文件名
            } else {
                cout << "错误：-t 后缺少文件个数。" << endl;
                return false;
            }
        }
        // 解析 -n 参数
        else if (arg == "-n") {
            if (i + 1 < argc) {
                int count = atoi(argv[i + 1]);
                N.clear();
                for (int j = 0; j < count; j++) {
                    if (i + 2 + j < argc) {
                        N.push_back(atoi(argv[i + 2 + j]));
                    } else {
                        cout << "错误：-n 后缺少整数。" << endl;
                        return false;
                    }
                }
                i += count + 1;  // 跳过整数列表
            } else {
                cout << "错误：-n 后缺少整数个数。" << endl;
                return false;
            }
        }
        // 解析 -h 参数
        else if (arg == "-h") {
            print_help();
            return false;
        } else {
            cout << "未知参数: " << arg << endl;
            return false;
        }
    }

    return true;
}

// encode函数
void encode(const vector<string>& fileNames, vector<int>& Sk, unordered_map<string, int>& data2Sk, vector<vector<int>>& P, int& K) {
    // 遍历每个文件名
    for (int i = 0; i < fileNames.size(); ++i) {
        ifstream file(fileNames[i]);
        string line;
        vector<int> currentFileData;

        // 逐行读取文件中的数据
        while (getline(file, line)) {
            // 使用stringstream去掉前后空格
            stringstream ss(line);
            string data;
            ss >> data; // 提取第一个单词并自动去除前后空格

            // 判断该数据是否已经存在于data2Sk中
            auto it = data2Sk.find(data);
            if (it != data2Sk.end()) {
                // 如果存在，将当前数据的Sk值加入P[i]中
                currentFileData.push_back(it->second);
            } else {
                // 如果不存在，新增一个Sk值
                int newSk = Sk.size();
                Sk.push_back(newSk);
                data2Sk[data] = newSk;
                currentFileData.push_back(newSk);
            }
        }

        P.push_back(currentFileData);
    }

    // 设置K为Sk的长度
    K = Sk.size();
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

