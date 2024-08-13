/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SRC_MEDIAFILE_RECORDER_H_
#define SRC_MEDIAFILE_RECORDER_H_

#include <memory>
#include <string>

namespace mediakit
{

    struct MediaTuple
    {
        std::string vhost;
        std::string app;
        std::string stream;
        std::string params;
        std::string shortUrl() const
        {
            return vhost + '/' + app + '/' + stream;
        }
    };

    class RecordInfo : public MediaTuple
    {
    public:
        time_t start_time;  // GMT 标准时间，单位秒
        float time_len;     // 录像长度，单位秒
        uint64_t file_size;    // 文件大小，单位 BYTE
        std::string file_path;   // 文件路径
        std::string file_name;   // 文件名称
        std::string folder;      // 文件夹路径
        std::string url;         // 播放路径
    };


} /* namespace mediakit */
#endif /* SRC_MEDIAFILE_RECORDER_H_ */
