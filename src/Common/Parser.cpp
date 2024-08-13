/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <cinttypes>
#include "Parser.h"
#include "strCoding.h"
#include "Util/base64.h"
#include "Network/sockutil.h"
#include "Common/macros.h"

using namespace std;
using namespace toolkit;

namespace mediakit {

    // 函数用于在buf中查找子字符串，返回从start到end之间的子字符串
    string findSubString(const char* buf, const char* start, const char* end, size_t buf_size) {
        // 如果buf_size小于等于0，则将其设置为buf的长度
        if (buf_size <= 0) {
            buf_size = strlen(buf);
        }
        // 定义msg_start和msg_end，分别指向buf的开头和结尾
        auto msg_start = buf;
        auto msg_end = buf + buf_size;
        size_t len = 0;
        // 如果start不为空，则计算start的长度，并将msg_start指向buf中第一个start的位置
        if (start != NULL) {
            len = strlen(start);
            msg_start = strstr(buf, start);
        }
        // 如果msg_start为空，则返回空字符串
        if (msg_start == NULL) {
            return "";
        }
        // 将msg_start指向start之后的位置
        msg_start += len;
        // 如果end不为空，则将msg_end指向msg_start中第一个end的位置
        if (end != NULL) {
            msg_end = strstr(msg_start, end);
            // 如果msg_end为空，则返回空字符串
            if (msg_end == NULL) {
                return "";
            }
        }
        // 返回从msg_start到msg_end之间的子字符串
        return string(msg_start, msg_end);
    }
    // 文本协议解析器
    void Parser::parse(const char* buf, size_t size) {
        // 清空解析器
        clear();
        // 指向缓冲区的指针
        auto ptr = buf;
        // 无限循环，直到解析完毕
        while (true) {
            // 查找下一个换行符
            auto next_line = strchr(ptr, '\n');
            // 默认偏移量为1
            auto offset = 1;
            // 检查是否找到换行符，并且换行符在当前指针之后
            CHECK(next_line && next_line > ptr);
            // 如果上一个字符是回车符，则将指针向前移动一位，偏移量为2
            if (*(next_line - 1) == '\r') {
                next_line -= 1;
                offset = 2;
            }
            // 如果当前指针是缓冲区的起始位置
            if (ptr == buf) {
                // 查找空格
                auto blank = strchr(ptr, ' ');
                // 检查空格是否在当前指针之后，并且空格在换行符之前
                CHECK(blank > ptr && blank < next_line);
                // 将方法名赋值给_method
                _method = std::string(ptr, blank - ptr);
                // 查找下一个空格
                auto next_blank = strchr(blank + 1, ' ');
                // 检查下一个空格是否在换行符之前
                CHECK(next_blank && next_blank < next_line);
                // 将URL赋值给_url
                _url.assign(blank + 1, next_blank);
                // 查找URL中的参数
                auto pos = _url.find('?');
                // 如果找到参数
                if (pos != string::npos) {
                    // 将参数赋值给_params
                    _params = _url.substr(pos + 1);
                    // 解析参数
                    _url_args = parseArgs(_params);
                    // 将URL中的参数部分去掉
                    _url = _url.substr(0, pos);
                }
                // 将协议赋值给_protocol
                _protocol = std::string(next_blank + 1, next_line);
            }
            // 如果当前指针不是缓冲区的起始位置
            else {
                // 查找冒号
                auto pos = strchr(ptr, ':');
                // 检查冒号是否在当前指针之后，并且冒号在换行符之前
                CHECK(pos > ptr && pos < next_line);
                // 将键名赋值给key
                std::string key{ ptr, static_cast<std::size_t>(pos - ptr) };
                // 将值赋值给value
                std::string value;
                // 如果冒号后面有一个空格
                if (pos[1] == ' ') {
                    // 将值赋值给value
                    value.assign(pos + 2, next_line);
                }
                // 如果冒号后面没有空格
                else {
                    // 将值赋值给value
                    value.assign(pos + 1, next_line);
                }
                // 将键值对插入到_headers中
                _headers.emplace_force(trim(std::move(key)), trim(std::move(value)));
            }
            // 将指针移动到下一个换行符之后
            ptr = next_line + offset;
            // 如果当前指针是"\r\n"，则协议解析完毕
            if (strncmp(ptr, "\r\n", 2) == 0) {
                // 将内容赋值给_content
                _content.assign(ptr + 2, buf + size);
                // 跳出循环
                break;
            }
        }
    }
    const string& Parser::method() const {
        return _method;
    }

    const string& Parser::url() const {
        return _url;
    }

    const std::string& Parser::status() const {
        return url();
    }

    string Parser::fullUrl() const {
        if (_params.empty()) {
            return _url;
        }
        return _url + "?" + _params;
    }

    const string& Parser::protocol() const {
        return _protocol;
    }

    const std::string& Parser::statusStr() const {
        return protocol();
    }

    static std::string kNull;

    const string& Parser::operator[](const char* name) const {
        auto it = _headers.find(name);
        if (it == _headers.end()) {
            return kNull;
        }
        return it->second;
    }

    const string& Parser::content() const {
        return _content;
    }

    void Parser::clear() {
        _method.clear();
        _url.clear();
        _params.clear();
        _protocol.clear();
        _content.clear();
        _headers.clear();
        _url_args.clear();
    }

    const string& Parser::params() const {
        return _params;
    }

    void Parser::setUrl(string url) {
        _url = std::move(url);
    }

    void Parser::setContent(string content) {
        _content = std::move(content);
    }

    StrCaseMap& Parser::getHeader() const {
        return _headers;
    }

    StrCaseMap& Parser::getUrlArgs() const {
        return _url_args;
    }

    // 解析参数
    StrCaseMap Parser::parseArgs(const string& str, const char* pair_delim, const char* key_delim) {
        // 定义返回值
        StrCaseMap ret;
        // 将字符串按照pair_delim分割成键值对
        auto arg_vec = split(str, pair_delim);
        // 遍历键值对
        for (auto& key_val : arg_vec) {
            // 如果键值对为空，则忽略
            if (key_val.empty()) {
                // 忽略
                continue;
            }
            // 在键值对中查找key_delim的位置
            auto pos = key_val.find(key_delim);
            if (pos != string::npos) {
                // 如果找到了key_delim，则将键值对分割成键和值
                auto key = trim(std::string(key_val, 0, pos));
                auto val = trim(key_val.substr(pos + strlen(key_delim)));
                // 将键和值放入返回值中
                ret.emplace_force(std::move(key), std::move(val));
            }
            else {
                // 如果没有找到key_delim，则将键值对作为键，值为空
                trim(key_val);
                if (!key_val.empty()) {
                    ret.emplace_force(std::move(key_val), "");
                }
            }
        }
        // 返回返回值
        return ret;
    }

    std::string Parser::mergeUrl(const string& base_url, const string& path) {
        // 以base_url为基础, 合并path路径生成新的url, path支持相对路径和绝对路径
        if (base_url.empty()) {
            return path;
        }
        if (path.empty()) {
            return base_url;
        }
        // 如果包含协议，则直接返回
        if (path.find("://") != string::npos) {
            return path;
        }

        string protocol = "http://";
        size_t protocol_end = base_url.find("://");
        if (protocol_end != string::npos) {
            protocol = base_url.substr(0, protocol_end + 3);
        }
        // 如果path以"//"开头，则直接拼接协议
        if (path.find("//") == 0) {
            return protocol + path.substr(2);
        }
        string host;
        size_t pos = 0;
        if (protocol_end != string::npos) {
            pos = base_url.find('/', protocol_end + 3);
            host = base_url.substr(0, pos);
            if (pos == string::npos) {
                pos = base_url.size();
            }
            else {
                pos++;
            }
        }
        // 如果path以"/"开头，则直接拼接协议和主机
        if (path[0] == '/') {
            return host + path;
        }
        vector<string> path_parts;
        size_t next_pos = 0;
        if (!host.empty()) {
            path_parts.emplace_back(host);
        }
        while ((next_pos = base_url.find('/', pos)) != string::npos) {
            path_parts.emplace_back(base_url.substr(pos, next_pos - pos));
            pos = next_pos + 1;
        }
        pos = 0;
        while ((next_pos = path.find('/', pos)) != string::npos) {
            string part = path.substr(pos, next_pos - pos);
            if (part == "..") {
                if (!path_parts.empty() && !path_parts.back().empty()) {
                    if (path_parts.size() > 1 || protocol_end == string::npos) {
                        path_parts.pop_back();
                    }
                }
            }
            else if (part != "." && !part.empty()) {
                path_parts.emplace_back(part);
            }
            pos = next_pos + 1;
        }

        string part = path.substr(pos);
        if (part != ".." && part != "." && !part.empty()) {
            path_parts.emplace_back(part);
        }
        stringstream final_url;
        for (size_t i = 0; i < path_parts.size(); ++i) {
            if (i == 0) {
                final_url << path_parts[i];
            }
            else {
                final_url << '/' << path_parts[i];
            }
        }
        return final_url.str();
    }

    void RtspUrl::parse(const string& strUrl) {
        // 查找字符串中第一个出现的"//"，用于提取协议
        auto schema = findSubString(strUrl.data(), nullptr, "://");
        // 判断协议是否为rtsps，如果是，则is_ssl为true
        bool is_ssl = strcasecmp(schema.data(), "rtsps") == 0;
        // 查找"://"与"/"之间的字符串，用于提取用户名密码
        auto middle_url = findSubString(strUrl.data(), "://", "/");
        if (middle_url.empty()) {
            // 如果没有找到"/"，则查找"://"与字符串末尾之间的字符串
            middle_url = findSubString(strUrl.data(), "://", nullptr);
        }
        // 查找字符串中最后一个出现的"@"，用于提取用户名密码
        auto pos = middle_url.rfind('@');
        if (pos == string::npos) {
            // 如果没有找到"@"，则没有用户名密码
            return setup(is_ssl, strUrl, "", "");
        }

        // 如果有用户名密码
        // 提取用户名密码
        auto user_pwd = middle_url.substr(0, pos);
        // 提取url的后缀
        auto suffix = strUrl.substr(schema.size() + 3 + pos + 1);
        // 拼接url
        auto url = StrPrinter << "rtsp://" << suffix << endl;
        // 如果用户名密码中没有":"，则没有密码
        if (user_pwd.find(":") == string::npos) {
            return setup(is_ssl, url, user_pwd, "");
        }
        // 提取用户名
        auto user = findSubString(user_pwd.data(), nullptr, ":");
        // 提取密码
        auto pwd = findSubString(user_pwd.data(), ":", nullptr);
        // 设置url
        return setup(is_ssl, url, user, pwd);
    }

    void RtspUrl::setup(bool is_ssl, const string& url, const string& user, const string& passwd) {
        auto ip = findSubString(url.data(), "://", "/");
        if (ip.empty()) {
            ip = split(findSubString(url.data(), "://", NULL), "?")[0];
        }
        // 根据是否使用ssl，设置端口号
        uint16_t port = is_ssl ? 322 : 554;
        // 将ip地址和端口号分离
        splitUrl(ip, ip, port);

        // 将url、用户名、密码、ip地址、端口号和是否使用ssl保存到成员变量中
        _url = std::move(url);
        _user = strCoding::UrlDecodeUserOrPass(user);
        _passwd = strCoding::UrlDecodeUserOrPass(passwd);
        _host = std::move(ip);
        _port = port;
        _is_ssl = is_ssl;
    }

    static void inline checkHost(std::string& host) {
        if (host.back() == ']' && host.front() == '[') {
            // ipv6去除方括号
            host.pop_back();
            host.erase(0, 1);
            CHECK(SockUtil::is_ipv6(host.data()), "not a ipv6 address:", host);
        }
    }

    void splitUrl(const std::string& url, std::string& host, uint16_t& port) {
        // 检查url是否为空
        CHECK(!url.empty(), "empty url");
        // 在url中查找冒号的位置
        auto pos = url.rfind(':');
        // 如果没有冒号，或者最后一个字符是']'，则说明没有指定端口，或者是纯粹的ipv6地址
        if (pos == string::npos || url.back() == ']') {
            // 将url赋值给host
            host = url;
            // 检查host是否合法
            checkHost(host);
            return;
        }
        // 检查冒号的位置是否大于0
        CHECK(pos > 0, "invalid url:", url);
        // 从url中解析出端口号
        CHECK(sscanf(url.data() + pos + 1, "%" SCNu16, &port) == 1, "parse port from url failed:", url);
        // 将url中冒号之前的部分赋值给host
        host = url.substr(0, pos);
        // 检查host是否合法
        checkHost(host);
    }

    void parseProxyUrl(const std::string& proxy_url, std::string& proxy_host, uint16_t& proxy_port, std::string& proxy_auth) {
        // 判断是否包含http://, 如果是则去掉
        std::string host;
        auto pos = proxy_url.find("://");
        if (pos != string::npos) {
            host = proxy_url.substr(pos + 3);
        }
        else {
            host = proxy_url;
        }
        // 判断是否包含用户名和密码
        pos = host.rfind('@');
        if (pos != string::npos) {
            proxy_auth = encodeBase64(host.substr(0, pos));
            host = host.substr(pos + 1, host.size());
        }
        splitUrl(host, proxy_host, proxy_port);
    }

#if 0
    //测试代码
    static onceToken token([]() {
        string host;
        uint16_t port;
        splitUrl("www.baidu.com:8880", host, port);
        splitUrl("192.168.1.1:8880", host, port);
        splitUrl("[::]:8880", host, port);
        splitUrl("[fe80::604d:4173:76e9:1009]:8880", host, port);
        });
#endif

} // namespace mediakit
