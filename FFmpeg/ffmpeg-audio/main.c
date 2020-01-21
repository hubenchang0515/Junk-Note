#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_error.h>
    #include <libavformat/avformat.h>     // 封装格式解析
    #include <libavcodec/avcodec.h>       // 编解码
    #include <libswresample/swresample.h> // 音频重采样
    #include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif

typedef struct PCMData
{
    uint8_t* data;  // 数据
    int length;  // 总长度
    int pos;     // 当前位置
}PCMData;

/* SDL音频设备回调函数 */
void getData(void* userdata, Uint8* stream, int len);

int main(int argc, char* argv[])
{
    /* 参数检查 */
    if(argc != 2)
    {
        fprintf(stderr, "Usage : %s <file>\n", argv[0]);
        return 1;
    }
    
    /* 打开视频文件，读取封装格式 */
    AVFormatContext* formatContext = NULL;
    int ret = avformat_open_input(&formatContext, argv[1], NULL, NULL);
    if(ret != 0)
    {
        fprintf(stderr, "avformat_open_input() failed.\n");
        return 1;
    }
    
    /* 从封装格式中查找数据流的基本信息 */
    ret = avformat_find_stream_info(formatContext, NULL);
    if(ret < 0)
    {
        fprintf(stderr, "avformat_find_stream_info() failed.\n");
        return 1;
    }
    
    /* 打印封装格式的信息 */
    av_dump_format(formatContext, 0, argv[1], 0);
    
    /* 找到音频流的解码器参数 */
    AVCodecParameters* audioCodecParams = NULL;
    int audioIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if(audioIndex == -1)
    {
        fprintf(stderr, "Cannot find audio stream.\n");
        return 1;
    }
    audioCodecParams = formatContext->streams[audioIndex]->codecpar;
    
    /* 根据解码器参数找到解码器 */
    AVCodec* audioCodec = avcodec_find_decoder(audioCodecParams->codec_id);
    if(audioCodec == NULL)
    {
        fprintf(stderr, "avcodec_find_decoder failed.\n");
        return 1;
    }
    
    /* 为解码器分配内存，用来保存上下文 */
    AVCodecContext* audioCodecContext = avcodec_alloc_context3(audioCodec);
    if(audioCodecContext == NULL)
    {
        fprintf(stderr, "avcodec_alloc_context3 failed.\n");
        return 1;
    }
    
    /* 初始化上下文 */
    ret = avcodec_parameters_to_context(audioCodecContext, audioCodecParams);
    if(ret < 0)
    {
        fprintf(stderr, "avcodec_parameters_to_context failed.\n");
        return 1;
    }
    
    /* 打开解码器 */
    ret = avcodec_open2(audioCodecContext, audioCodec, NULL);
    if(ret < 0)
    {
        fprintf(stderr, "avcodec_open2 failed.\n");
        return 1;
    }

    /* 为packet分配内存 */
    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    if(packet == NULL)
    {
        fprintf(stderr, "av_malloc failed.\n");
        return 1;
    }
    av_init_packet(packet);

    /* 帧 */
    AVFrame *frame = av_frame_alloc();

    /* 重采样上下文 */
    SwrContext *swrContext = swr_alloc();

    /* 配置重采用并初始化 */
    swrContext = swr_alloc_set_opts(swrContext, 
        AV_CH_LAYOUT_STEREO,                      // 输出配置
        AV_SAMPLE_FMT_FLT,
        audioCodecContext->sample_rate,
        audioCodecContext->channel_layout,      // 输入配置
        audioCodecContext->sample_fmt,
        audioCodecContext->sample_rate,
        0,
        NULL);
    swr_init(swrContext);

    SDL_Log("%d %d", audioCodecContext->sample_rate, AV_SAMPLE_FMT_FLT);

    /* 重采样单次输出数据的大小 */
    SDL_Log("%d", audioCodecContext->frame_size);
    int outputSize = av_samples_get_buffer_size(NULL, 
        2, 
        audioCodecContext->frame_size, 
        AV_SAMPLE_FMT_FLT, 
        1);

    /* 初始化SDL2 */
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL_Init failed : %s .\n", SDL_GetError());
        return 1;
    }
    
    /* 创建窗口 */
    SDL_Window* window = SDL_CreateWindow("FFmpeg-Player",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          300,
                                          300,
                                          SDL_WINDOW_OPENGL);
    if(window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow failed : %s .\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    /* 创建Renderer */
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL)
    {
        fprintf(stderr, "SDL_CreateRenderer failed : %s .\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    /* 把屏幕刷黑 */
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);  

    /* 重采样输出 */
    uint8_t * buffer = (uint8_t *) av_malloc(2 * audioCodecContext->sample_rate);

    /* 回调参数 */
    PCMData pcm;
    pcm.data = malloc(100 * 1024 * 1024); // 100MB
    pcm.length = 0;
    pcm.pos = 0;

    /* 音频设备规格(设为和重采样的输出一致) */
    SDL_AudioSpec sdlAudioSpec;
    sdlAudioSpec.format = AUDIO_F32SYS;
    sdlAudioSpec.channels = audioCodecContext->channels; // STEREO
    sdlAudioSpec.freq = audioCodecContext->sample_rate;
    sdlAudioSpec.samples = audioCodecContext->frame_size;
    sdlAudioSpec.callback = getData;
    sdlAudioSpec.userdata = &pcm;

    ret = SDL_OpenAudio(&sdlAudioSpec,NULL); //打开声音设备
    SDL_PauseAudio(0); //开始播放
    
    /* 读取音频帧并显示到窗口上 */
    while(1)
    {
        /*  */
        if(av_read_frame(formatContext, packet) == 0)
        {
           

            /* 非音频流跳过 */
            if(packet->stream_index != audioIndex)
            {
                av_packet_unref(packet);
                continue;
            }

            /* 将音频流发送给解码器 */
            int ret = avcodec_send_packet(audioCodecContext, packet);
            if (ret != 0)
            {
                printf("avcodec_send_packet() failed.\n");
                continue;
            }
            
            /* 从解码器接收一个帧并进行重采样 */
            while(avcodec_receive_frame(audioCodecContext, frame) == 0)
            {
                /* 重采样转换 */
                swr_convert(swrContext, 
                    &buffer, 
                    2 * audioCodecContext->sample_rate, 
                    (const uint8_t **)(frame->data), 
                    frame->nb_samples);
            }

            /* 保存数据 */
            memcpy(pcm.data + pcm.length, buffer, outputSize);
            pcm.length += outputSize;

            av_packet_unref(packet);
        }
        

        /* 关闭窗口 */
        SDL_Event e;
        if(0 == SDL_PollEvent(&e)) //捕获事件
        {
            continue;  //没有检测到事件，循环继续
        }
        else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
        {
            break; // 按下ESC键
        }
        else if(e.type == SDL_QUIT)
        {
            break; //检测到退出事件，退出
        }
    }
    
    SDL_PauseAudio(1);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    swr_free(&swrContext);
    av_frame_free(&frame);
    avcodec_close(audioCodecContext);
    avformat_close_input(&formatContext);

    return 0;
}

void getData(void* userdata, Uint8* stream, int len)
{
    PCMData* pcm = (PCMData*)userdata;

    if(len > pcm->length - pcm->pos)
    {
        SDL_Log("to long.");
        return;
    }

    memset(stream, 0, len);
    SDL_MixAudio(stream, pcm->data + pcm->pos, len, SDL_MIX_MAXVOLUME);
    pcm->pos += len;
}