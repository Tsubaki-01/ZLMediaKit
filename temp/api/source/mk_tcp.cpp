/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include "string.h"
#include "mk_tcp.h"
#include "Network/Buffer.h"

using namespace toolkit;

class BufferForC : public Buffer
{
public:
    BufferForC(const char* data, size_t len, on_mk_buffer_free cb, std::shared_ptr<void> user_data)
    {
        if (len <= 0)
        {
            len = strlen(data);
        }
        if (!cb)
        {
            auto ptr = malloc(len);
            memcpy(ptr, data, len);
            data = (char*) ptr;

            cb = [] (void* user_data, void* data)
                {
                    free(data);
                };
        }
        _data = (char*) data;
        _size = len;
        _cb = cb;
        _user_data = std::move(user_data);
    }

    ~BufferForC() override
    {
        _cb(_user_data.get(), _data);
    }

    char* data() const override
    {
        return _data;
    }

    size_t size() const override
    {
        return _size;
    }

private:
    char* _data;
    size_t _size;
    on_mk_buffer_free _cb;
    std::shared_ptr<void> _user_data;
};

API_EXPORT mk_buffer API_CALL mk_buffer_from_char(const char* data, size_t len, on_mk_buffer_free cb, void* user_data)
{
    return mk_buffer_from_char2(data, len, cb, user_data, nullptr);
}

API_EXPORT mk_buffer API_CALL mk_buffer_from_char2(const char* data, size_t len, on_mk_buffer_free cb, void* user_data, on_user_data_free user_data_free)
{
    assert(data);
    std::shared_ptr<void> ptr(user_data, user_data_free ? user_data_free : [] (void*) { });
    return (mk_buffer)new Buffer::Ptr(std::make_shared<BufferForC>(data, len, cb, std::move(ptr)));
}

API_EXPORT mk_buffer API_CALL mk_buffer_ref(mk_buffer buffer)
{
    assert(buffer);
    return (mk_buffer)new Buffer::Ptr(*((Buffer::Ptr*) buffer));
}

API_EXPORT void API_CALL mk_buffer_unref(mk_buffer buffer)
{
    assert(buffer);
    delete (Buffer::Ptr*) buffer;
}

API_EXPORT const char* API_CALL mk_buffer_get_data(mk_buffer buffer)
{
    assert(buffer);
    return (*((Buffer::Ptr*) buffer))->data();
}

API_EXPORT size_t API_CALL mk_buffer_get_size(mk_buffer buffer)
{
    assert(buffer);
    return (*((Buffer::Ptr*) buffer))->size();
}