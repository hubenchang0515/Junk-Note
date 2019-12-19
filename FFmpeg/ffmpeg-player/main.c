#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    #include <SDL2/SDL.h>
    #include <libavformat/avformat.h>  // 封装格式解析
    #include <libavcodec/avcodec.h>    // 编解码
    #include <libswscale/swscale.h>    // 比例缩放及色彩映射
    #include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif

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
    
    /* 找到第一个视频流的解码器参数 */
    AVCodecParameters* codecParams = NULL;
    int videoIndex = 0;
    for(int i = 0; i < formatContext->nb_streams; i++)
    {
        if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            codecParams = formatContext->streams[i]->codecpar;
            videoIndex = i;
            break;
        }
    }
    if(codecParams == NULL)
    {
        fprintf(stderr, "Cannot find video stream.\n");
        return 1;
    }
    
    /* 根据解码器参数找到解码器 */
    AVCodec* codec = avcodec_find_decoder(codecParams->codec_id);
    if(codec == NULL)
    {
        fprintf(stderr, "avcodec_find_decoder failed.\n");
        return 1;
    }
    
    /* 为解码器分配内存，用来保存上下文 */
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    if(codecContext == NULL)
    {
        fprintf(stderr, "avcodec_alloc_context3 failed.\n");
        return 1;
    }
    
    /* 初始化上下文 */
    ret = avcodec_parameters_to_context(codecContext, codecParams);
    if(ret < 0)
    {
        fprintf(stderr, "avcodec_parameters_to_context failed.\n");
        return 1;
    }
    
    /* 打开解码器 */
    ret = avcodec_open2(codecContext, codec, NULL);
    if(ret < 0)
    {
        fprintf(stderr, "avcodec_open2 failed.\n");
        return 1;
    }
    
    /* 创建帧 */
    AVFrame* rawFrame = av_frame_alloc(); // 原始数据
    if(rawFrame == NULL)
    {
        fprintf(stderr, "av_frame_alloc failed.\n");
        return 1;
    }
    AVFrame* yuvFrame = av_frame_alloc(); // YUV格式数据
    if(yuvFrame == NULL)
    {
        fprintf(stderr, "av_frame_alloc failed.\n");
        return 1;
    }
    
    /* 为解码后的YUV格式数据分配内存 */
    int bufSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                           codecContext->width,
                                           codecContext->height,
                                           1);
    const uint8_t* buffer = (const uint8_t*)av_malloc(bufSize);
    if(buffer == NULL)
    {
        fprintf(stderr, "av_malloc failed.\n");
        return 1;
    }
    ret = av_image_fill_arrays(yuvFrame->data,
                         yuvFrame->linesize,
                         buffer,
                         AV_PIX_FMT_YUV420P,
                         codecContext->width,
                         codecContext->height,
                         1);
    if(ret < 0)
    {
        fprintf(stderr, "av_image_fill_arrays failed.\n");
        return 1;
    }
    
    /* 创建SWS上下文，用于图像转换 */
    struct SwsContext* swsContext = sws_getContext(codecContext->width,   // 源参数
                                         codecContext->height,
                                         codecContext->pix_fmt,
                                         codecContext->width,   // 目标参数
                                         codecContext->height,
                                         AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC,
                                         NULL,
                                         NULL,
                                         NULL);
                                         
    if(swsContext == NULL)
    {
        fprintf(stderr, "av_malloc failed.\n");
        return 1;
    }
    
    
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
                                          codecContext->width,
                                          codecContext->height,
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
    
    /* 创建Texture */
    SDL_Texture* texture = SDL_CreateTexture(renderer, 
                                    SDL_PIXELFORMAT_IYUV,   // 格式应当与前面FFmpeg转换出的格式匹配
                                    SDL_TEXTUREACCESS_STREAMING,
                                    codecContext->width,
                                    codecContext->height);  
    if(texture == NULL)
    {
        fprintf(stderr, "SDL_CreateTexture failed : %s .\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    /* 为packet分配内存 */
    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    if(packet == NULL)
    {
        fprintf(stderr, "av_malloc failed.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    SDL_Rect rect = {0, 0, codecContext->width, codecContext->height};
    
    /* 读取视频帧并显示到窗口上 */
    while(av_read_frame(formatContext, packet) == 0)
    {
        /* 非视频流跳过 */
        if(packet->stream_index != videoIndex)
        {
            av_packet_unref(packet);
            continue;
        }
        
        /* 将视频流发送给解码器 */
        ret = avcodec_send_packet(codecContext, packet);
        if (ret != 0)
        {
            printf("avcodec_send_packet() failed.\n");
            return 1;
        }
        
        /* 从解码器接收一个帧 */
        ret = avcodec_receive_frame(codecContext, rawFrame);
        if (ret != 0 && ret != AVERROR(EAGAIN))
        {
            printf("avcodec_receive_frame() failed %d.\n", ret);
            return 1;
        }
        
        /* 将该帧转换为YUV格式 */
        sws_scale(swsContext,             // sws context
                  (const uint8_t * const*)rawFrame->data,        // 源参数
                  rawFrame->linesize,
                  0,
                  codecContext->height,
                  (uint8_t * const*)yuvFrame->data,        // 目标参数
                  yuvFrame->linesize);
                  
        /* 更新Texture */
        SDL_UpdateYUVTexture(texture,
                             &rect,                   // 矩形范围
                             yuvFrame->data[0],      // y plane
                             yuvFrame->linesize[0],  // y pitch
                             yuvFrame->data[1],      // u plane
                             yuvFrame->linesize[1],  // u pitch
                             yuvFrame->data[2],      // v plane
                             yuvFrame->linesize[2]   // v pitch
                             );
                             
        /* 将Texture的内容拷贝到窗口上 */
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        
        /* 显示 */
        SDL_RenderPresent(renderer);  

        // 延迟，非精确
        SDL_Delay(40);
        av_packet_unref(packet);

        /* 关闭窗口 */
        SDL_Event e;
        if(0 == SDL_PollEvent(&e)) //捕获事件
        {
            continue;  //没有检测到事件，循环继续
        }
        else if(e.type == SDL_QUIT)
        {
            break; //检测到退出事件，退出
        }
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    sws_freeContext(swsContext);
    //av_free(buffer);
    av_frame_free(&yuvFrame);
    av_frame_free(&rawFrame);
    avcodec_close(codecContext);
    avformat_close_input(&formatContext);

    return 0;
}