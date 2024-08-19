#!/bin/bash

# 定义运行时间（24小时，单位为秒）
RUNTIME=$((24*60*60 ))

# 记录开始时间
START_TIME=$(date +%s)

# 启动程序并使用valgrind监控
valgrind --leak-check=full --log-file=test/long_connection/valgrind_log.txt ./test/long_connection/long_connection rtsp://192.168.13.155/live/ch00_0

# 获取程序的PID
PID=$!

# 监控程序运行时间
while true; do
    # 当前时间
    CURRENT_TIME=$(date +%s)
    
    # 计算已运行时间
    ELAPSED_TIME=$((CURRENT_TIME - START_TIME))
    
    # 检查是否超过24小时
    if [ $ELAPSED_TIME -ge $RUNTIME ]; then
        echo "已经运行24小时，关闭程序..."
        kill $PID
        break
    fi
    
    # 每10分钟检查一次
    sleep 600
done

echo "程序已停止。"
