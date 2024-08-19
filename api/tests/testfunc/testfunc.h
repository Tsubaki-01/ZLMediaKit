#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <dirent.h>
#include <chrono>
#include <thread>
#include <sys/times.h>
#include <sys/wait.h>
#include <signal.h>

int getThreadCount(pid_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/task";
    int threadCount = 0;

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr)
    {
        std::cerr << "Failed to open directory: " << path << std::endl;
        return -1;
    }

    while (readdir(dir) != nullptr)
    {
        ++threadCount;
    }

    closedir(dir);
    return threadCount - 2; // "." and ".." entries are counted
}


size_t getMemoryUsage(pid_t pid)
{
    std::ifstream file("/proc/" + std::to_string(pid) + "/status");
    std::string line;
    size_t memoryUsage = 0;

    while (std::getline(file, line))
    {
        if (line.find("VmRSS:") == 0)
        {
            std::istringstream iss(line);
            std::string key;
            size_t value;
            iss >> key >> value;
            memoryUsage = value; // KB
        }
    }

    return memoryUsage;
}

void monitorPidstat(pid_t pid, const std::string output_file, bool& running)
{
    std::ofstream file(output_file.data(), std::ios::out);
    if (!file.is_open())
    {
        std::cerr << "Failed to open output file: " << output_file << std::endl;
        return;
    }

    // Run pidstat command and capture the output
    std::string command = "pidstat -p " + std::to_string(pid) + " -u 1 >> " + output_file;
    pid_t ppid = fork();
    int status;

    if (ppid == 0)
    {
        execl("/bin/sh", "sh", "-c", command.c_str(), (char*) NULL);
    }
    else
    {
        while (running)
        {
        }

        kill(ppid, SIGINT);
        waitpid(ppid, &status, 0);
    }



    file.close();
}