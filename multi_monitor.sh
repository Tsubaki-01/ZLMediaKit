#!/bin/bash

# 定义总运行时间（24小时，单位为秒）
TOTAL_RUNTIME=$((24*60*60))

# 记录开始时间
START_TIME=$(date +%s)

# 定义循环次数计数器
COUNT=0

# 进入循环
while true; do
    # 当前时间
    CURRENT_TIME=$(date +%s)
    
    # 计算已运行时间
    ELAPSED_TIME=$((CURRENT_TIME - START_TIME))
    
    # 检查是否超过24小时
    if [ $ELAPSED_TIME -ge $TOTAL_RUNTIME ]; then
        echo "已经运行24小时，停止测试..."
        break
    fi

    # 运行测试
    COUNT=$((COUNT + 1))
    echo "开始第 $COUNT 次测试..."
    ./test/multi_player_connection/multi_connection rtsp://192.168.13.155/live/ch00_0

    echo "第 $COUNT 次测试结束。"
    
    sleep(1)
done

echo "所有测试完成。"
