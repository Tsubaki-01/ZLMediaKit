/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <algorithm>
#include "PlayerBase.h"
#include "Rtsp/RtspPlayerImp.h"
#include "Rtmp/RtmpPlayerImp.h"
#include "Rtmp/FlvPlayer.h"
#include "Http/HlsPlayer.h"
#include "Http/TsPlayerImp.h"

using namespace std;
using namespace toolkit;

namespace mediakit {

    // 创建一个PlayerBase对象
    PlayerBase::Ptr PlayerBase::createPlayer(const EventPoller::Ptr& in_poller, const string& url_in) {
        // 如果传入的EventPoller对象不为空，则使用传入的EventPoller对象，否则使用EventPollerPool中的EventPoller对象
        auto poller = in_poller ? in_poller : EventPollerPool::Instance().getPoller();
        // 创建一个弱指针，指向EventPoller对象
        std::weak_ptr<EventPoller> weak_poller = poller;
        // 创建一个释放函数，用于释放PlayerBase对象
        auto release_func = [weak_poller](PlayerBase* ptr) {
            // 如果弱指针指向的EventPoller对象不为空，则使用EventPoller对象的异步函数释放PlayerBase对象
            if (auto poller = weak_poller.lock()) {
                poller->async([ptr]() {
                    onceToken token(nullptr, [&]() { delete ptr; });
                    ptr->teardown();
                    });
            }
            else {
                // 否则直接释放PlayerBase对象
                delete ptr;
            }
            };
        // 将传入的url赋值给url变量
        string url = url_in;
        // 获取url中的协议部分
        string prefix = findSubString(url.data(), NULL, "://");
        // 获取url中的参数部分
        auto pos = url.find('?');
        if (pos != string::npos) {
            //去除？后面的字符串
            url = url.substr(0, pos);
        }

        // 如果协议部分为rtsps，则创建一个TcpClientWithSSL<RtspPlayerImp>对象
        if (strcasecmp("rtsps", prefix.data()) == 0) {
            return PlayerBase::Ptr(new TcpClientWithSSL<RtspPlayerImp>(poller), release_func);
        }

        // 如果协议部分为rtsp，则创建一个RtspPlayerImp对象
        if (strcasecmp("rtsp", prefix.data()) == 0) {
            // 复制初始化
            return PlayerBase::Ptr(new RtspPlayerImp(poller), release_func);
        }

        // 如果协议部分为rtmps，则创建一个TcpClientWithSSL<RtmpPlayerImp>对象
        if (strcasecmp("rtmps", prefix.data()) == 0) {
            return PlayerBase::Ptr(new TcpClientWithSSL<RtmpPlayerImp>(poller), release_func);
        }

        // 如果协议部分为rtmp，则创建一个RtmpPlayerImp对象
        if (strcasecmp("rtmp", prefix.data()) == 0) {
            return PlayerBase::Ptr(new RtmpPlayerImp(poller), release_func);
        }
        // 如果协议部分为http或https，则根据url的后缀创建相应的PlayerBase对象
        if ((strcasecmp("http", prefix.data()) == 0 || strcasecmp("https", prefix.data()) == 0)) {
            // 如果url后缀为.m3u8，则创建一个HlsPlayerImp对象
            if (end_with(url, ".m3u8") || end_with(url_in, ".m3u8")) {
                return PlayerBase::Ptr(new HlsPlayerImp(poller), release_func);
            }
            // 如果url后缀为.ts，则创建一个TsPlayerImp对象
            if (end_with(url, ".ts") || end_with(url_in, ".ts")) {
                return PlayerBase::Ptr(new TsPlayerImp(poller), release_func);
            }
            // 如果url后缀为.flv，则创建一个FlvPlayerImp对象
            if (end_with(url, ".flv") || end_with(url_in, ".flv")) {
                return PlayerBase::Ptr(new FlvPlayerImp(poller), release_func);
            }
        }

        // 如果协议部分不支持，则抛出异常
        throw std::invalid_argument("not supported play schema:" + url_in);
    }

    // PlayerBase的构造函数
    PlayerBase::PlayerBase() {
        // 设置Client的参数——mINI继承的一个map，这边相当于是访问这个哈希表
        this->mINI::operator[](Client::kTimeoutMS) = 10000;
        this->mINI::operator[](Client::kMediaTimeoutMS) = 5000;
        this->mINI::operator[](Client::kBeatIntervalMS) = 5000;
        this->mINI::operator[](Client::kWaitTrackReady) = true;
    }
} /* namespace mediakit */
