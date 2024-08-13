/*
 * 版权所有 (c) 2016-present The ZLMediaKit项目作者。保留所有权利。
 *
 * 此文件是ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit)的一部分。
 *
 * 使用此源代码受MIT-like许可证的约束，该许可证可在源代码根目录的LICENSE文件中找到。
 * 源代码树中的所有贡献项目作者都可以在根目录的AUTHORS文件中找到。
 */

#include "HttpRequestSplitter.h"
#include "Util/logger.h"
#include "Util/util.h"
using namespace toolkit;
using namespace std;

//协议解析最大缓存4兆数据
static constexpr size_t kMaxCacheSize = 4 * 1024 * 1024;

namespace mediakit {

    //输入数据
    void HttpRequestSplitter::input(const char* data, size_t len) {
        {
            auto size = remainDataSize();
            if (size > _max_cache_size) {
                //缓存太多数据无法处理则上抛异常
                reset();
                throw std::out_of_range("remain data size is too huge, now cleared:" + to_string(size));
            }
        }
        const char* ptr = data;
        if (!_remain_data.empty()) {
            _remain_data.append(data, len);
            data = ptr = _remain_data.data();
            len = _remain_data.size();
        }

    splitPacket:

        /*确保ptr最后一个字节是0，防止strstr越界
         *由于ZLToolKit确保内存最后一个字节是保留未使用字节并置0，
         *所以此处可以不用再次置0
         *但是上层数据可能来自其他渠道，保险起见还是置0
         */

        char& tail_ref = ((char*)ptr)[len];
        char tail_tmp = tail_ref;
        tail_ref = 0;

        //数据按照请求头处理
        const char* index = nullptr;
        _remain_data_size = len;
        while (_content_len == 0 && _remain_data_size > 0 && (index = onSearchPacketTail(ptr, _remain_data_size)) != nullptr) {
            if (index == ptr) {
                break;
            }
            if (index < ptr || index > ptr + _remain_data_size) {
                throw std::out_of_range("上层分包逻辑异常");
            }
            //_content_len == 0，这是请求头
            const char* header_ptr = ptr;
            ssize_t header_size = index - ptr;
            ptr = index;
            _remain_data_size = len - (ptr - data);
            _content_len = onRecvHeader(header_ptr, header_size);
        }

        /*
         * 恢复末尾字节
         * 移动到这来，目的是防止HttpRequestSplitter::reset()导致内存失效
         */
        tail_ref = tail_tmp;

        if (_remain_data_size <= 0) {
            //没有剩余数据，清空缓存
            _remain_data.clear();
            return;
        }

        if (_content_len == 0) {
            //尚未找到http头，缓存定位到剩余数据部分
            _remain_data.assign(ptr, _remain_data_size);
            return;
        }

        //已经找到http头了
        if (_content_len > 0) {
            //数据按照固定长度content处理
            if (_remain_data_size < (size_t)_content_len) {
                //数据不够，缓存定位到剩余数据部分
                _remain_data.assign(ptr, _remain_data_size);
                return;
            }
            //收到content数据，并且接收content完毕
            onRecvContent(ptr, _content_len);

            _remain_data_size -= _content_len;
            ptr += _content_len;
            //content处理完毕,后面数据当做请求头处理
            _content_len = 0;

            if (_remain_data_size > 0) {
                //还有数据没有处理完毕
                _remain_data.assign(ptr, _remain_data_size);
                data = ptr = (char*)_remain_data.data();
                len = _remain_data.size();
                goto splitPacket;
            }
            _remain_data.clear();
            return;
        }


        //_content_len < 0;数据按照不固定长度content处理
        onRecvContent(ptr, _remain_data_size);//消费掉所有剩余数据
        _remain_data.clear();
    }

    //设置content长度
    void HttpRequestSplitter::setContentLen(ssize_t content_len) {
        _content_len = content_len;
    }

    //重置
    void HttpRequestSplitter::reset() {
        _content_len = 0;
        _remain_data_size = 0;
        _remain_data.clear();
    }

    //搜索数据包尾部
    const char* HttpRequestSplitter::onSearchPacketTail(const char* data, size_t len) {
        auto pos = strstr(data, "\r\n\r\n");
        if (pos == nullptr) {
            return nullptr;
        }
        return  pos + 4;
    }

    //获取剩余数据大小
    size_t HttpRequestSplitter::remainDataSize() {
        return _remain_data_size;
    }

    //获取剩余数据
    const char* HttpRequestSplitter::remainData() const {
        return _remain_data.data();
    }

    //设置最大缓存大小
    void HttpRequestSplitter::setMaxCacheSize(size_t max_cache_size) {
        if (!max_cache_size) {
            max_cache_size = kMaxCacheSize;
        }
        _max_cache_size = max_cache_size;
    }

    //构造函数
    HttpRequestSplitter::HttpRequestSplitter() {
        setMaxCacheSize(0);
    }

} /* namespace mediakit */