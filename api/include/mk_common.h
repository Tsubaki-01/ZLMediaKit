/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MK_COMMON_H
#define MK_COMMON_H

#include <stdint.h>
#include <stddef.h>

#if defined(GENERATE_EXPORT)
#include "mk_export.h"
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#    define API_CALL __cdecl
#else
#    define API_CALL
#endif

#ifndef _WIN32
#define _strdup strdup
#endif

#if defined(_WIN32) && defined(_MSC_VER)
#    if !defined(GENERATE_EXPORT)
#        if defined(MediaKitApi_EXPORTS)
#            define API_EXPORT __declspec(dllexport)
#        else
#            define API_EXPORT __declspec(dllimport)
#        endif
#    endif
#elif !defined(GENERATE_EXPORT)
#   define API_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

    //输出日志到shell
#define LOG_CONSOLE     (1 << 0)
//输出日志到文件
#define LOG_FILE        (1 << 1)
//输出日志到回调函数(mk_events::on_mk_log)
#define LOG_CALLBACK    (1 << 2)

//向下兼容
#define mk_env_init1 mk_env_init2

//回调user_data回调函数
    typedef void(API_CALL* on_user_data_free)(void* user_data);

    typedef struct
    {
        // 线程数
        int thread_num;

        // 日志级别,支持0~4
        int log_level;
        //控制日志输出的掩模，请查看LOG_CONSOLE、LOG_FILE、LOG_CALLBACK等宏
        int log_mask;
        //文件日志保存路径,路径可以不存在(内部可以创建文件夹)，设置为NULL关闭日志输出至文件
        const char* log_file_path;
        //文件日志保存天数,设置为0关闭日志文件
        int log_file_days;

        // 配置文件是内容还是路径
        int ini_is_path;
        // 配置文件内容或路径，可以为NULL,如果该文件不存在，那么将导出默认配置至该文件
        const char* ini;

        // ssl证书是内容还是路径
        int ssl_is_path;
        // ssl证书内容或路径，可以为NULL
        const char* ssl;
        // 证书密码，可以为NULL
        const char* ssl_pwd;
    } mk_config;

    /**
     * 初始化环境，调用该库前需要先调用此函数
     * @param cfg 库运行相关参数
     */
    API_EXPORT void API_CALL mk_env_init(const mk_config* cfg);

    /**
     * 关闭所有服务器，请在main函数退出时调用
     */
    API_EXPORT void API_CALL mk_stop_all_server();

    /**
     * 基础类型参数版本的mk_env_init，为了方便其他语言调用
     * @param thread_num 线程数
     * @param log_level 日志级别,支持0~4
     * @param log_mask 日志输出方式掩模，请查看LOG_CONSOLE、LOG_FILE、LOG_CALLBACK等宏
     * @param log_file_path 文件日志保存路径,路径可以不存在(内部可以创建文件夹)，设置为NULL关闭日志输出至文件
     * @param log_file_days 文件日志保存天数,设置为0关闭日志文件
     * @param ini_is_path 配置文件是内容还是路径
     * @param ini 配置文件内容或路径，可以为NULL,如果该文件不存在，那么将导出默认配置至该文件
     * @param ssl_is_path ssl证书是内容还是路径
     * @param ssl ssl证书内容或路径，可以为NULL
     * @param ssl_pwd 证书密码，可以为NULL
     */
    API_EXPORT void API_CALL mk_env_init2(int thread_num,
        int log_level,
        int log_mask,
        const char* log_file_path,
        int log_file_days,
        int ini_is_path,
        const char* ini,
        int ssl_is_path,
        const char* ssl,
        const char* ssl_pwd);

    /**
    * 设置日志文件
    * @param file_max_size 单个切片文件大小(MB)
    * @param file_max_count 切片文件个数
    */
    API_EXPORT void API_CALL mk_set_log(int file_max_size, int file_max_count);

    /**
     * 设置配置项
     * @deprecated 请使用mk_ini_set_option替代
     * @param key 配置项名
     * @param val 配置项值
     */
    API_EXPORT void API_CALL mk_set_option(const char* key, const char* val);

    /**
     * 获取配置项的值
     * @deprecated 请使用mk_ini_get_option替代
     * @param key 配置项名
     */
    API_EXPORT const char* API_CALL mk_get_option(const char* key);


#ifdef __cplusplus
}
#endif


#endif /* MK_COMMON_H */
