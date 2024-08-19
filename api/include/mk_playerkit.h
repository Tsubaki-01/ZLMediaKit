#ifndef MK_API_H_
#define MK_API_H_

#include "mk_common.h"
#include "mk_player.h"
#include "mk_tcp.h"
#include "mk_util.h"
#include "mk_track.h"
#include "mk_transcode.h"
#include "onceToken.h"

namespace playerkit
{
    typedef void (API_CALL* playerEvent)(void* user_data, int err_code, const char* err_msg,
        mk_track tracks [], int track_count);
    typedef void (API_CALL* frameOutEvent)(void* user_data, mk_frame frame);
    typedef void (API_CALL* frameEvent)(void* user_data, mk_frame_pix frame);

    typedef struct
    {
        mk_player player;
        mk_decoder video_decoder;
    } Context;

    void (API_CALL default_onPlayEvent)(void* user_data, int err_code, const char* err_msg,
        mk_track tracks [], int track_count);
    void API_CALL default_onShutDownEvent(void* user_data, int err_code, const char* err_msg, mk_track tracks [], int track_count);
    void API_CALL default_onTrackFrameOut(void* user_data, mk_frame frame);
    void API_CALL default_onDecodeFrameOut(void* user_data, mk_frame_pix frame);


    class Player
    {
    public:
        Player()
        {
            _onTrackFrameOut = default_onTrackFrameOut;
            _onDecodeFrameOut = default_onDecodeFrameOut;
            _onShutDownEvent = default_onShutDownEvent;
            _onPlayEvent = default_onPlayEvent;
        };
        ~Player()
        {
            if (_ctx.player)
            {
                mk_player_release(_ctx.player);
            }
            if (_ctx.video_decoder)
            {
                mk_decoder_release(_ctx.video_decoder, 1);
            }
        };

        void init(int thread_num = 0,
            int log_level = 0,
            int log_mask = LOG_CONSOLE | LOG_FILE,
            const char* log_file_path = "./log/player.log",
            int log_file_days = 30,
            int ini_is_path = 0,
            const char* ini = NULL,
            int ssl_is_path = 1,
            const char* ssl = NULL,
            const char* ssl_pwd = NULL);

        void play(std::string url);

    public:
        // Getter and Setter for _onPlayEvent
        playerEvent getOnPlayEvent() const
        {
            return _onPlayEvent;
        }

        void setOnPlayEvent(const playerEvent& event)
        {
            _onPlayEvent = event;
        }

        // Getter and Setter for _onShutDownEvent
        playerEvent getOnShutDownEvent() const
        {
            return _onShutDownEvent;
        }

        void setOnShutDownEvent(const playerEvent& event)
        {
            _onShutDownEvent = event;
        }

        // Getter and Setter for _onTrackFrameOut
        frameOutEvent getOnTrackFrameOut() const
        {
            return _onTrackFrameOut;
        }

        void setOnTrackFrameOut(const frameOutEvent& event)
        {
            _onTrackFrameOut = event;
        }

        // Getter and Setter for _onDecodeFrameOut
        frameEvent getOnDecodeFrameOut() const
        {
            return _onDecodeFrameOut;
        }

        void setOnDecodeFrameOut(const frameEvent& event)
        {
            _onDecodeFrameOut = event;
        }

    protected:

        /* void (API_CALL default_onPlayEvent)(void* user_data, int err_code, const char* err_msg,
            mk_track tracks [], int track_count);
        void API_CALL default_onShutDownEvent(void* user_data, int err_code, const char* err_msg, mk_track tracks [], int track_count);
        void API_CALL default_onTrackFrameOut(void* user_data, mk_frame frame);
        void API_CALL default_onDecodeFrameOut(void* user_data, mk_frame_pix frame); */
        // playerEvent default_onPlayEvent;
        // playerEvent  default_onShutDownEvent;
        // frameOutEvent default_onTrackFrameOut;
        // frameEvent default_onDecodeFrameOut;
        // void initDefault();

    private:
        Context _ctx;
        playerEvent _onPlayEvent;
        playerEvent _onShutDownEvent;
        frameOutEvent _onTrackFrameOut;
        frameEvent _onDecodeFrameOut;

    };


    void Player::init(int thread_num,
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
        mk_env_init2(thread_num,
            log_level,
            log_mask,
            log_file_path,
            log_file_days,
            ini_is_path,
            ini,
            ssl_is_path,
            ssl,
            ssl_pwd);

        memset(&_ctx, 0, sizeof(Context));
        _ctx.player = mk_player_create();
    }

    void Player::play(std::string url)
    {
        mk_player_set_on_result(_ctx.player, _onPlayEvent, &_ctx);
        mk_player_set_on_shutdown(_ctx.player, _onShutDownEvent, &_ctx);
        mk_player_play(_ctx.player, url.data());
    }



    void (API_CALL default_onPlayEvent)(void* user_data, int err_code, const char* err_msg,
        mk_track tracks [], int track_count)
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
                    mk_track_add_delegate(tracks[i], default_onTrackFrameOut, user_data);
                    // 设置解码回调
                    mk_decoder_set_cb(ctx->video_decoder, default_onDecodeFrameOut, user_data);
                }
            }
        }
        else
        {
            log_warn("play failed: %d %s", err_code, err_msg);
        }
    };

    void API_CALL default_onShutDownEvent(void* user_data, int err_code, const char* err_msg, mk_track tracks [], int track_count)
    {
        log_warn("play interrupted: %d %s", err_code, err_msg);
    }

    void API_CALL default_onTrackFrameOut(void* user_data, mk_frame frame)
    {
        Context* ctx = (Context*) user_data;
        mk_decoder_decode(ctx->video_decoder, frame, 1, 1);

        std::ofstream outFile;

        static onceToken token(
            [&outFile] ()
            {
                outFile.open("./output.h264", std::ios::out | std::ios::binary);
                outFile.close();
            }, nullptr
        );

        outFile.open("./output.h264", std::ios::app | std::ios::binary);
        outFile.write(mk_frame_get_data(frame), mk_frame_get_data_size(frame));
        outFile.close();

        // log_debug("version 1.0");
    }

    void API_CALL default_onDecodeFrameOut(void* user_data, mk_frame_pix frame)
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

        // mk_swscale_input_frame(ctx->swscale, frame, brg24);

        free(brg24);
    }

}


#endif /* MK_API_H_ */
