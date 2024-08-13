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
#include "MediaPlayer.h"

using namespace std;
using namespace toolkit;

namespace mediakit {

    // MediaPlayer类的构造函数，接受一个EventPoller的指针作为参数
    MediaPlayer::MediaPlayer(const EventPoller::Ptr& poller) {
        // 如果参数不为空，则使用参数，否则使用EventPollerPool的单例获取一个EventPoller
        _poller = poller ? poller : EventPollerPool::Instance().getPoller();
    }

    // 设置Socket的创建回调函数
    static void setOnCreateSocket_l(const std::shared_ptr<PlayerBase>& delegate, const Socket::onCreateSocket& cb) {
        // 将delegate转换为SocketHelper类型
        auto helper = dynamic_pointer_cast<SocketHelper>(delegate);
        if (helper) {
            // 如果回调函数不为空，则设置回调函数
            if (cb) {
                helper->setOnCreateSocket(cb);
            }
            else {
                // 客户端，确保开启互斥锁
                helper->setOnCreateSocket([](const EventPoller::Ptr& poller) {
                    return Socket::createSocket(poller, true);
                    });
            }
        }
    }

    // 播放函数，接受一个url作为参数
    void MediaPlayer::play(const string& url) {
        // 根据url创建一个PlayerImp对象，使用_poller和url作为参数
        _delegate = PlayerBase::createPlayer(_poller, url);
        // 断言_player不为空
        assert(_delegate);
        // 设置Socket的创建回调函数
        setOnCreateSocket_l(_delegate, _on_create_socket);
        // 设置_delegate的关闭回调函数
        _delegate->setOnShutdown(_on_shutdown);
        // 设置_delegate的播放结果回调函数
        _delegate->setOnPlayResult(_on_play_result);
        // 设置_delegate的恢复播放回调函数
        _delegate->setOnResume(_on_resume);
        // 设置_delegate的媒体源
        _delegate->setMediaSource(_media_src);
        // 遍历MediaPlayer的属性，将属性设置到_delegate中（PlayerBase有继承了一个自实现的map的子类）
        for (auto& pr : *this) {
            (*_delegate)[pr.first] = pr.second;
        }
        // 播放url
        _delegate->play(url);
    }

    // 获取EventPoller的指针
    EventPoller::Ptr MediaPlayer::getPoller() {
        return _poller;
    }

    // 设置Socket的创建回调函数
    void MediaPlayer::setOnCreateSocket(Socket::onCreateSocket cb) {
        // 设置回调函数
        setOnCreateSocket_l(_delegate, cb);
        // 保存回调函数
        _on_create_socket = std::move(cb);
    }

} /* namespace mediakit */
