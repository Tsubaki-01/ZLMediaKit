/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include "mk_common.h"
#include <stdarg.h>
#include <unordered_map>
#include "Util/logger.h"
#include "Util/SSLBox.h"
#include "Util/File.h"
 // #include "Network/TcpServer.h"
 // #include "Network/UdpServer.h"
#include "Thread/WorkThreadPool.h"

#include "Rtsp/RtspSession.h"
#include "Shell/ShellSession.h"
using namespace std;
using namespace toolkit;
using namespace mediakit;

// static TcpServer::Ptr rtsp_server[2];
// static TcpServer::Ptr rtmp_server[2];
// static TcpServer::Ptr http_server[2];
// static TcpServer::Ptr shell_server;


//////////////////////////environment init///////////////////////////

API_EXPORT void API_CALL mk_env_init(const mk_config* cfg)
{
    assert(cfg);
    mk_env_init1(cfg->thread_num,
        cfg->log_level,
        cfg->log_mask,
        cfg->log_file_path,
        cfg->log_file_days,
        cfg->ini_is_path,
        cfg->ini,
        cfg->ssl_is_path,
        cfg->ssl,
        cfg->ssl_pwd);
}




API_EXPORT void API_CALL mk_env_init2(int thread_num,
    int log_level,
    int log_mask,
    const char* log_file_path,
    int log_file_days,
    int ini_is_path,
    const char* ini,
    int ssl_is_path,
    const char* ssl,
    const char* ssl_pwd)
{
    //确保只初始化一次
    static onceToken token([&] ()
        {
            if (log_mask & LOG_CONSOLE)
            {
                //控制台日志
                Logger::Instance().add(std::make_shared<ConsoleChannel>("ConsoleChannel", (LogLevel) log_level));
            }

            if (log_mask & LOG_CALLBACK)
            {
                //广播日志
                Logger::Instance().add(std::make_shared<EventChannel>("EventChannel", (LogLevel) log_level));
            }

            if (log_mask & LOG_FILE)
            {
                //日志文件
                auto channel = std::make_shared<FileChannel>("FileChannel",
                    log_file_path ? File::absolutePath("", log_file_path) :
                    exeDir() + "log/", (LogLevel) log_level);
                channel->setMaxDay(log_file_days ? log_file_days : 1);
                Logger::Instance().add(channel);
            }

            //异步日志线程
            Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

            //设置线程数
            EventPollerPool::setPoolSize(thread_num);
            WorkThreadPool::setPoolSize(thread_num);

            if (ini && ini[0])
            {
                //设置配置文件
                if (ini_is_path)
                {
                    try
                    {
                        mINI::Instance().parseFile(ini);
                    }
                    catch (std::exception&)
                    {
                        InfoL << "dump ini file to:" << ini;
                        mINI::Instance().dumpFile(ini);
                    }
                }
                else
                {
                    mINI::Instance().parse(ini);
                }
            }

            if (ssl && ssl[0])
            {
                //设置ssl证书
                SSL_Initor::Instance().loadCertificate(ssl, true, ssl_pwd ? ssl_pwd : "", ssl_is_path);
            }
        });
}

API_EXPORT void API_CALL mk_set_log(int file_max_size, int file_max_count)
{
    auto channel = dynamic_pointer_cast<FileChannel>(Logger::Instance().get("FileChannel"));
    if (channel)
    {
        channel->setFileMaxSize(file_max_size);
        channel->setFileMaxCount(file_max_count);
    }
}

API_EXPORT void API_CALL mk_set_option(const char* key, const char* val)
{
    assert(key && val);
    if (mINI::Instance().find(key) == mINI::Instance().end())
    {
        WarnL << "key:" << key << " not existed!";
        return;
    }
    mINI::Instance()[key] = val;
    //广播配置文件热加载
    NOTICE_EMIT(BroadcastReloadConfigArgs, Broadcast::kBroadcastReloadConfig);
}

API_EXPORT const char* API_CALL mk_get_option(const char* key)
{
    assert(key);
    if (mINI::Instance().find(key) == mINI::Instance().end())
    {
        WarnL << "key:" << key << " not existed!";
        return nullptr;
    }
    return mINI::Instance()[key].data();
}


