#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "xargparse.h"
#include <libavformat/avformat.h>  
#include <libavcodec/avcodec.h>
#include "video_decoder.h"

//很有问题的池化
Frame resize_max(Frame frame,const int fitter,const int stride){//池化的窗口大小和步长
    Frame output;//定义池化后的图像
    //printf("**here**\n");
    int MAX_R,MAX_G,MAX_B;
    output.height=(frame.height-fitter)/stride+1;
    if(frame.height%fitter!=0) output.height++;//池化后图片height

    output.width=(frame.width-1-fitter)/stride+1;//公式计算池化后图片的width
    if(frame.width%fitter!=0) output.width++;
    
    output.linesize=output.width*3+1;//池化后的linesize
    output.data = (char*)malloc(output.linesize*output.height * sizeof(char));
    for(int i=0;i<output.height;i++){//i标记行数
        for(int j=1;j<=output.width;j++){//j跟踪每一行的元素,**从1开始**
        //遍历图像

            MAX_R=0;
            MAX_G=0;
            MAX_B=0;
            //初始化

            for(int step_h=0;step_h<fitter;step_h++){
                for(int step_w=0;step_w<fitter*3;step_w++){
                    //遍历窗口
                    int temp;
                 
                    //边界处理
                    if(j*stride+step_w>=frame.linesize) break;//横向越界跳出循环
                    if(i*stride>=frame.height) temp=0;//纵向越界补0;
                    else temp=frame.data[(i*stride+step_h)*frame.linesize+j*stride+step_w];
                    //frame.data[i*stride+step_h][j*stride+step_w];

                    switch ((step_w+1)%3)//判断是哪个分量
                    {
                    case 0:
                        if(temp>MAX_B) MAX_B=temp;
                        //printf("B");
                        break;
                    case 1:
                        if(temp>MAX_R) MAX_R=temp;
                        //printf("R");
                        break;
                    case 2:
                        if(temp>MAX_G) MAX_G=temp;
                        //printf("G");
                        break;
                    default:
                        break;
                    }
                }
            }

            output.data[i*output.linesize+j]=MAX_R;
            output.data[i*output.linesize+j+1]=MAX_G;
            output.data[i*output.linesize+j+2]=MAX_B; 
            printf("*R:%d,G:%d,B:%d\n",MAX_R,MAX_G,MAX_B);
            //printf("R:%c,G:%c,B:%c\n",output.data[i*output.linesize+j],output.data[i*output.linesize+j+1],output.data[i*output.linesize+j+2]);
            
        }
    }
    return output;
}

//等第一个写好了再直接改的池化
Frame resize_ave(Frame frame,int level){
}

void printf_rgb(Frame frame)
{
    int R,G,B,grey;//暂存
    int a=0,line=0;
    //int size_v=size_h*frame.height/frame.width;
        for(line=0;line<frame.height;line++){
            for(a=0;a<frame .linesize;a++){
                R=frame.data[(frame.linesize)*line+a+1];
                G=frame.data[(frame.linesize)*line+a+2];
                B=frame.data[(frame.linesize)*line+a+3];
                //printf("\033[38;2;%d;%d;%dm█\033[0m", R, G, B); //理论上可以打出rgb颜色的代码 
                int color_index = 16 + 36 * (R / 51) + 6 * (G / 51) + (B  / 51);   
                printf("\033[38;5;%dm█\033[0m", color_index);
            }
            printf("\n");
       }
}

//不是很稳定
void print_grey(Frame frame){
    int a=0,line=0;
    int R,G,B,grey;
   char butter[(frame.height/5+1)*(frame.linesize/5+1)];
    //char butter[frame.height*frame.linesize];
    //int size_v=2*(size_h*frame.height/frame.width);
    for(line=0;line<frame.height;line+=5){
            for(a=0;a<frame .linesize;a+=5){
                R=frame.data[(frame.linesize)*line+a+1];
                G=frame.data[(frame.linesize)*line+a+2];
                B=frame.data[(frame.linesize)*line+a+3];
                //printf("*R:%d,G:%d,B:%d\n",R,G,B);//
                grey=(R*76 + G*150 + B*30) >> 8;
                if(grey<20)
                    strcat(butter," ") ;  
                else if(grey<40) strcat(butter,".");
                else if(grey<80) strcat(butter,"-");
                else if(grey<127) strcat(butter,":");
                else if(grey<200) strcat(butter,"#");
                else strcat(butter,"@");
                
                }
            //printf("\n");
            strcat(butter,"\n");
            //建立缓存区（很有炸的潜质现在）
       }
       strcat(butter,"\0");
    printf("%s",butter);
    //free(butter);
}

int main(int argc, const char **argv){

    /*//一个关于颜色的测试
    printf("\033[44m█\033[0m\n"); // \033[ 是开始ANSI转义序列的标记，37是白色前景色，41是红色背景色，0是重置所有属性到默认值  
    printf_red("█");
    char *colorterm = getenv("COLORTERM");
    if (colorterm != NULL && strcmp(colorterm, "truecolor") == 0) {
        printf("终端支持True Color\n");
    } else {
        printf("终端不支持True Color\n");
    }
    */


//命令行编写
   int Have_color=0;//默认灰度图
    int frame_skipping=5;//默认帧率
    int stride=2;
    int fitter=2;//设置默认的池化数值.

    char *str, *filepath="/home/lancyx/dian/dragon.mp4";
    int *resize;
    argparse_option options[] = {
        //(bind, short_name, long_name, help_info, append_info, name)
        XBOX_ARG_BOOLEAN(NULL, "-h", "--help", "show help information", NULL, "help"),
        XBOX_ARG_BOOLEAN(NULL, "-v", "--version", "show version", NULL, "version"),
        //帮助信息和版本信息

        XBOX_ARG_BOOLEAN(NULL,"-c","-color","shoe RGB or Grey",NULL,"color"),
        //RGB or Grey（默认灰度图）

        XBOX_ARG_INT_GROUP(&frame_skipping,"s","--skip","fram skip",NULL,"skip"),
        XBOX_ARG_INTS_GROUP(&resize, "-r", "--resize", "resize by pooling", NULL, "resize"),
        //调整池化窗口大小（默认fitter=2，stride=2）
        
        XBOX_ARG_STR_GROUP(&filepath, "-f", NULL, "filepath", NULL, "filepath"),
        //选择要播放的文件的路径（含默认值）       
       
        XBOX_ARG_END()
    };
    
    XBOX_argparse parser;
    XBOX_argparse_init(&parser, options, 0);
    XBOX_argparse_describe(&parser,
                           "main",
                           "\nA brief description of what the program does and how it works.",
                           "\nAdditional description of the program after the description of the arguments.");
    XBOX_argparse_parse(&parser, argc, argv);
    if (XBOX_ismatch(&parser, "help")) {
        XBOX_argparse_info(&parser);
        return 0;
    }
    if (XBOX_ismatch(&parser, "version")) {
        printf("v0.0.1\n");
        return 0;
    }

    //颜色
    if (XBOX_ismatch(&parser, "color")) {
        Have_color=1;
    }

    //resize
    if (XBOX_ismatch(&parser, "resize")){
        int Is_legal = XBOX_ismatch(&parser, "resize");
        if(Is_legal!=2){
            printf("不合法输入");
            return -1;
        }
        fitter=resize[0];
        stride=resize[1];
    }       
    XBOX_free_argparse(&parser);
//命令行编写结束

    if(decoder_init(filepath)==0) //成功时返回 0 ，失败时返回 -1 
        printf("Find it!\n");
    else{
        printf("未找到视频流\n");
        return -1;
    }

    int n= get_total_frames();
    printf("total:%d\n",n);

//解码视频
    Frame frame;//读取全部

    // char progress[101];
    // int count_progress=100;//进度条编写
    // int i_progress=0;
    // memset (progress,'\0',sizeof(progress));
    clock_t start,end;
    while(1){
        start = clock();
        for(int j=0;j<frame_skipping;j++){
            Frame temp=decoder_get_frame();
            if(temp.data==NULL) break;
        }//减去一些帧
        frame=decoder_get_frame();
        if(frame.data==NULL) break;//循环终止
        if(Have_color)    printf_rgb(frame);
        else print_grey(frame);
        
        //print_grey(frame[i]);
        end = clock();
        sleep(((double) (end - start)) / CLOCKS_PER_SEC);

        // if(i%(n/100)==0){
        //     printf("[%-100s\r]",progress);
        //     fflush(stdout);
        //     progress[i_progress++]='=';
        // }

        //system("clear");
        
        //printf("Present index:%d",get_frame_index());
        // printf("The picture%d :\nheight:%d\nwidth:%d\nline:%d\ndata:\n"
        // ,i , frame[i].height , frame[i].width,frame[i].linesize);//打印读取到的视频的相关信息       
    }

    //int i;
    // for(int i=0;i<1;i++){ 
    //     //system("clear");//打印的速度赶不上清屏的速度,还没出现就润了
    //     Frame temp=frame[i];
    //     //Frame temp=resize_max(frame[i],fitter,stride);//max pooling
        
    //     //print_grey(frame[i]);
    //     printf("finish\n");
    // }
    
    decoder_close();

    
}
/*
1.经典算法：Gray = R*0.299 + G*0.587 + B*0.114  
2.整数算法：Gray = (R*299 + G*587 + B*114 + 500) / 1000
3.整数移位算法：Gray = (R*76 + G*150 + B*30) >> 8
4.平均值法：Gray=（R+G+B）/3;
5.仅取绿色：Gray=G；*/

/*
typedef struct _Frame{
 int width;
 int height;
 int linesize;
 unsigned char* data;
} Frame;
*/
/*int decoder_init(const char *filename)
初始化视频解码器并打开指定的视频⽂件。请在执⾏以下任何函数前，先初始化decoder。

• const char *filename :要打开的视频⽂件路径。

• 返回值:成功时返回 0 ，失败时返回 -1 。
*/
/*Frame decoder_get_frame()

从视频中获取下⼀帧。

• 返回值:成功时返回帧数据，没有更多帧或获取失败时返回内容为空的Frame结构体。

void decoder_close()

关闭视频解码器并释放所有资源。

double get_fps()

获取视频的帧率。注意：当视频为可变帧率时，该函数获取的帧率值可能错误。

• 返回值:视频的帧率。
int get_frame_index()

获取当前帧的索引。

• 返回值:当前帧的索引。

int get_total_frames()

获取视频的总帧数。注意：当视频为可变帧率时，该函数获取的总帧数可能错误。

• 返回值:视频的总帧数。*/