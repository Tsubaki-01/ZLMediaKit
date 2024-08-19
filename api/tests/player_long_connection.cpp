#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "mk_playerkit.h"
#include "testfunc/testfunc.h"
#include <iomanip>

void getAverageMemoryUsage(pid_t pid, std::string outputPath, bool& running)
{
    unsigned long sum = 0;
    size_t cnt = 0;
    std::ofstream outFile;
    outFile.open((outputPath + "memoryMonitor").data(), std::ios::out);
    while (running)
    {
        size_t memoryUsage = getMemoryUsage(pid);
        sum += memoryUsage;
        cnt++;
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // 将时间转换为本地时间
        std::tm* now_tm = std::localtime(&now_c);

        // 将时间戳格式化为字符串
        outFile << "Timestamp: " << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
        outFile << "   -------   MemoryUsage : " << memoryUsage << " KB" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    outFile.close();
    outFile.open((outputPath + "output").data(), std::ios::app);
    outFile << "AverageMemoryUsage : " << (double) (sum / cnt);

    outFile.close();
}


int main(int argc, char* argv [])
{
    // 检查命令行参数个数
    if (argc != 2)
    {
        printf("Usage: ./player url\n");
        return -1;
    }

    pid_t pid = getpid();
    bool running = true;

    std::string outputPath("");
    std::ofstream outFile((outputPath + "output").data(), std::ios::out);
    if (!outFile.is_open())
    {
        std::cerr << "Failed to open output file " << std::endl;
        return -2;
    }





    playerkit::Player player;
    player.init();
    player.play(argv[1]);

    outFile << "pid : " << (pid) << std::endl;
    outFile << "ThreadCount : " << getThreadCount(pid) << std::endl;
    std::thread memoryMonitor(getAverageMemoryUsage, pid, (outputPath), std::ref(running));
    // std::thread CPUMonitor(monitorPidstat, pid, (outputPath + "CPUMonitor"), std::ref(running));


    // 输出提示信息
    log_info("enter any key to exit");
    // 等待用户输入
    getchar();

    // CPUMonitor.join();
    running = false;

    memoryMonitor.join();
    outFile.close();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}