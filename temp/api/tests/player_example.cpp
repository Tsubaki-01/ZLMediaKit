/*
 * Copyright (c) 2016-present The ZLMediaKit project authors. All Rights Reserved.
 *
 * This file is part of ZLMediaKit(https://github.com/ZLMediaKit/ZLMediaKit).
 *
 * Use of this source code is governed by MIT-like license that can be found in the
 * LICENSE file in the root of the source tree. All contributing project authors
 * may be found in the AUTHORS file in the root of the source tree.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "mk_transcode.h"
#include "mk_track.h" 
#include "mk_player.h"
#include "mk_frame.h" 
#include "mk_common.h"
#include "mk_util.h"
#include "mk_tcp.h" 

 //
int flag = 1;

//
typedef struct
{
    mk_player player;
    mk_decoder video_decoder;
    mk_swscale swscale;
} Context;

// 当有新的track数据输出时的回调函数
void API_CALL on_track_frame_out(void* user_data, mk_frame frame)
{
    Context* ctx = (Context*) user_data;
    mk_decoder_decode(ctx->video_decoder, frame, 1, 1);
    {
        std::ofstream outFile;
        // 打开一个文件以进行写入
        if (flag)
        {
            outFile.open("output.h264", std::ios::out | std::ios::binary);
            flag = !flag;
        } // 以二进制模式打开文件
        else outFile.open("output.h264", std::ios::app | std::ios::binary);

        // 将缓冲区的内容写入到文件中
        outFile.write(mk_frame_get_data(frame), mk_frame_get_data_size(frame));
        log_debug("flag:%d", flag);
        // 关闭文件
        outFile.close();
    }
}

// 当有新的解码帧输出时的回调函数
void API_CALL on_frame_decode(void* user_data, mk_frame_pix frame)
{
    //
    Context* ctx = (Context*) user_data;
    int w = mk_get_av_frame_width(mk_frame_pix_get_av_frame(frame));
    int h = mk_get_av_frame_height(mk_frame_pix_get_av_frame(frame));

    //
    int align = 32;
    size_t pixel_size = 3;
    size_t raw_linesize = w * pixel_size;
    // 对齐后的宽度
    size_t aligned_linesize = (raw_linesize + align - 1) & ~(align - 1);
    size_t total_size = aligned_linesize * h;
    uint8_t* brg24 = (uint8_t*) malloc(total_size);
    //
    mk_swscale_input_frame(ctx->swscale, frame, brg24);

    free(brg24);

}

// 当播放事件发生时的回调函数
void API_CALL on_mk_play_event_func(void* user_data, int err_code, const char* err_msg, mk_track tracks [],
    int track_count)
{
    Context* ctx = (Context*) user_data;
    if (err_code == 0)
    {
        //success
        log_debug("play success!");
        int i;
        for (i = 0; i < track_count; ++i)
        {
            //
            if (mk_track_is_video(tracks[i]))
            {
                log_info("got video track: %s", mk_track_codec_name(tracks[i]));
                ctx->video_decoder = mk_decoder_create(tracks[i], 0);
                // 监听track数据回调
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
                //
                mk_decoder_set_cb(ctx->video_decoder, on_frame_decode, user_data);
            }
        }
    }
    else
    {
        log_warn("play failed: %d %s", err_code, err_msg);
    }
}

// 当播放中断时的回调函数
void API_CALL on_mk_shutdown_func(void* user_data, int err_code, const char* err_msg, mk_track tracks [], int track_count)
{
    log_warn("play interrupted: %d %s", err_code, err_msg);
}

int main(int argc, char* argv [])
{
    mk_config config = {
            .thread_num = 0,
            .log_level = 0,
            .log_mask = LOG_CONSOLE,
            .ini_is_path = 0,
            .ini = NULL,
            .ssl_is_path = 1,
            .ssl = NULL,
            .ssl_pwd = NULL,
    };
    mk_env_init(&config);
    if (argc != 2)
    {
        printf("Usage: ./player url\n");
        return -1;
    }

    Context ctx;
    memset(&ctx, 0, sizeof(Context));

    //
    ctx.player = mk_player_create();
    ctx.swscale = mk_swscale_create(3, 0, 0);
    mk_player_set_on_result(ctx.player, on_mk_play_event_func, &ctx);
    mk_player_set_on_shutdown(ctx.player, on_mk_shutdown_func, &ctx);
    mk_player_play(ctx.player, argv[1]);

    log_info("enter any key to exit");
    getchar();

    if (ctx.player)
    {
        mk_player_release(ctx.player);
    }
    if (ctx.video_decoder)
    {
        mk_decoder_release(ctx.video_decoder, 1);
    }
    if (ctx.swscale)
    {
        mk_swscale_release(ctx.swscale);
    }
    return 0;
}