### FFmpeg转化流程
1. 开始
2. 注册复用器，编码器等 `av_register_all`
3. 打开多媒体文件 `avformat_open_input`
4. 根据流信息查找和打开解码器
5. 根据解码信息获取解码器类型
6. 查找解码器`avcodec_find_decoder`
7. 打开解码器`avcodec_open2`
8. 读取视频帧`av_read_frame`
9. 解码视频帧`avcodec_decode_video2`
10. 图像数据格式转换`sws_scale`可以使用libswscale的sws_scale()函数将帧从原始格式转换为RGB,以便后续处理或显示。
11. 输出图像`SDL_RenderPresent`
