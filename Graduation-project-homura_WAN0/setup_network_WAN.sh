#!/bin/bash

# 清除现有规则
sudo tc qdisc del dev lo root 2>/dev/null

# 设置广域网模拟参数
LATENCY="48ms"     # 基础延迟
VARIANCE="50ms"     # 延迟变化范围
LOSS="1.5%"         # 丢包率
CORRUPT="0.2%"      # 数据包损坏率
DUPLICATE="0.5%"    # 数据包重复率
BANDWIDTH="10mbit"  # 带宽限制
RATE="200Mbit"

# 应用网络规则到回环接口
sudo tc qdisc add dev lo root handle 1: netem \
    delay $LATENCY \
    rate $RATE
    #loss $LOSS \
    #corrupt $CORRUPT \
    #duplicate $DUPLICATE

echo "Network rules applied:"
echo "  Latency: $LATENCY"
echo "  Rate: $RATE"
