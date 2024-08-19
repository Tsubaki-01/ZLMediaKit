﻿/* #include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "mk_playerkit.h"

typedef struct
{
    mk_player player;
    mk_decoder video_decoder;
    mk_swscale swscale;
} Context;

// 当播放事件发生时的回调函数
void API_CALL on_mk_play_event_func(void* user_data, int err_code, const char* err_msg,
    mk_track tracks [], int track_count);

// 当播放中断时的回调函数
void API_CALL on_mk_shutdown_func(void* user_data, int err_code, const char* err_msg,
    mk_track tracks [], int track_count);

// 当有新的track数据输出时的回调函数
void API_CALL on_track_frame_out(void* user_data, mk_frame frame);

// 当有新的解码帧输出时的回调函数
void API_CALL on_frame_decode(void* user_data, mk_frame_pix frame);


int main(int argc, char* argv [])
{
    // 初始化配置
    mk_config config = {
            .thread_num = 0,
            .log_level = 0,
            .log_mask = LOG_CONSOLE | LOG_FILE ,
            .log_file_path = "./log/player.log",
            .ini_is_path = 0,
            .ini = NULL,
            .ssl_is_path = 1,
            .ssl = NULL,
            .ssl_pwd = NULL,
    };
    mk_env_init(&config);

    // 检查命令行参数个数
    if (argc != 2)
    {
        printf("Usage: ./player url\n");
        return -1;
    }

    // 创建上下文
    Context ctx;

    onceToken token(
        [&ctx] ()
        {
            memset(&ctx, 0, sizeof(Context));
            ctx.player = mk_player_create();
            ctx.swscale = mk_swscale_create(3, 0, 0);
        },
        [&ctx] ()
        {
            // 释放播放器
            if (ctx.player)
            {
                mk_player_release(ctx.player);
            }
            // 释放视频解码器
            if (ctx.video_decoder)
            {
                mk_decoder_release(ctx.video_decoder, 1);
            }
            // 释放图像缩放器
            if (ctx.swscale)
            {
                mk_swscale_release(ctx.swscale);
            }
        }
    );

    // 设置播放器回调函数
    mk_player_set_on_result(ctx.player, on_mk_play_event_func, &ctx);
    // 设置播放器关闭回调函数
    mk_player_set_on_shutdown(ctx.player, on_mk_shutdown_func, &ctx);
    // 播放
    mk_player_play(ctx.player, argv[1]);

    // 输出提示信息
    log_info("enter any key to exit");
    // 等待用户输入
    getchar();

    return 0;
}

void API_CALL on_mk_play_event_func(void* user_data, int err_code, const char* err_msg, mk_track tracks [],
    int track_count)
{
    Context* ctx = (Context*) user_data;

    if (err_code == 0)
    {
        log_debug("play success!");
        for (int i = 0; i < track_count; ++i)
        {
            if (mk_track_is_video(tracks[i]))
            {
                log_info("got video track: %s", mk_track_codec_name(tracks[i]));
                // 创建视频解码器
                ctx->video_decoder = mk_decoder_create(tracks[i], 0);
                // 监听track数据回调
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
                // 设置解码回调
                mk_decoder_set_cb(ctx->video_decoder, on_frame_decode, user_data);
            }
        }
    }
    else
    {
        log_warn("play failed: %d %s", err_code, err_msg);
    }
}


void API_CALL on_mk_shutdown_func(void* user_data, int err_code, const char* err_msg, mk_track tracks [], int track_count)
{
    log_warn("play interrupted: %d %s", err_code, err_msg);
}


void API_CALL on_track_frame_out(void* user_data, mk_frame frame)
{
    Context* ctx = (Context*) user_data;
    mk_decoder_decode(ctx->video_decoder, frame, 1, 1);

    std::ofstream outFile;

    static onceToken token(
        [&outFile] ()
        {
            outFile.open("output.h264", std::ios::out | std::ios::binary);
            outFile.close();
        }, nullptr
    );

    outFile.open("output.h264", std::ios::app | std::ios::binary);
    outFile.write(mk_frame_get_data(frame), mk_frame_get_data_size(frame));
    outFile.close();

    log_debug("tiktok");
}


void API_CALL on_frame_decode(void* user_data, mk_frame_pix frame)
{

    Context* ctx = (Context*) user_data;
    int w = mk_get_av_frame_width(mk_frame_pix_get_av_frame(frame));
    int h = mk_get_av_frame_height(mk_frame_pix_get_av_frame(frame));


    int align = 32;
    size_t pixel_size = 3;
    size_t raw_linesize = w * pixel_size;
    // 对齐后的宽度
    size_t aligned_linesize = (raw_linesize + align - 1) & ~(align - 1);
    size_t total_size = aligned_linesize * h;
    uint8_t* brg24 = (uint8_t*) malloc(total_size);

    mk_swscale_input_frame(ctx->swscale, frame, brg24);

    free(brg24);
}


 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "mk_playerkit.h"

int main(int argc, char* argv [])
{
    // 检查命令行参数个数
    if (argc != 2)
    {
        printf("Usage: ./player url\n");
        return -1;
    }

    playerkit::Player player;

    player.init();
    player.play(argv[1]);

    // 输出提示信息
    // log_info("enter any key to exit");
    // 等待用户输入
    // getchar();

    std::this_thread::sleep_for(std::chrono::minutes(1));

    return 0;
}