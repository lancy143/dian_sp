#include <stdio.h>
#include "xargparse.h"
#include <libavformat/avformat.h>  
#include <libavcodec/avcodec.h>
#include "video_decoder.h"

//边界待处理(补0)
Frame resize_max(Frame frame,const int fitter,const int stride){//池化的窗口大小和步长
    Frame output;//定义池化后的图像
    int MAX_R,MAX_G,MAX_B;
    output.height=frame.height/fitter;
    if(frame.height%fitter!=0) output.height++;//池化后图片height

    output.width=(frame.width-fitter)/stride+1;//公式计算池化后图片的width
    if(frame.width%fitter!=0) output.width++;
    
    output.linesize=output.width*3+1;//池化后的linesize

    for(int i=0;i<frame.height;i+=fitter){//i标记行数
        for(int j=1;j<frame.width;j+=stride){//j跟踪每一行的元素,**从1开始**
        //遍历图像

            MAX_R=0;
            MAX_G=0;
            MAX_B=0;
            //初始化

            for(int step_h=0;step_h<fitter;step_h++){
                for(int step_w=0;step_w<stride;step_w++){
                    //遍历窗口
                    int temp;
                    
                    //边界处理
                    if(j*stride+step_w>frame.linesize) break;//横向越界跳出循环
                    if(i*fitter>frame.height) temp=0;//纵向越界补0;
                    else temp=frame.data[(i*output.height+step_h)*frame.linesize+j*stride+step_w];
                    //frame.data[i*output.height+step_h][j*stride+step_w];

                    switch (step_w%3)//判断是哪个分量
                    {
                    case 0:
                        if(temp>MAX_B) MAX_B=temp;
                        break;
                    case 1:
                        if(temp>MAX_R) MAX_R=temp;
                        break;
                    case 2:
                        if(temp>MAX_G) MAX_G=temp;
                        break;
                    default:
                        break;
                    }
                }
            }
            output.data[i*output.linesize+j+1]=MAX_R;
            output.data[i*output.linesize+j+2]=MAX_G;
            output.data[i*output.linesize+j+3]=MAX_B; 
            
        }
    }
}

Frame resize_ave(Frame frame,int level){

}

void printf_rgb(Frame frame,int size_h)
{
    int R,G,B,grey;//暂存
    int a=0,line=0;
    int size_v=size_h*frame.height/frame.width;
        for(line=0;line<frame.height;line+=size_v){
            for(a=0;a<frame .linesize;a+=size_h){
                R=frame.data[(frame.linesize)*line+a+1]-'0';
                G=frame.data[(frame.linesize)*line+a+2]-'0';
                B=frame.data[(frame.linesize)*line+a+3]-'0';
                //printf("\033[38;2;%d;%d;%dm█\033[0m", R, G, B); //理论上可以打出rgb颜色的代码 
                int color_index = 16 + 36 * (R / 51) + 6 * (G / 51) + (B  / 51);   
                printf("\033[38;5;%dm█\033[0m", color_index);
            }
            printf("\n");
       }
}

//不知道为什么但是灰度图的打印不是很稳定
void print_grey(Frame frame ,int size_h){
    int a=0,line=0;
    int R,G,B,grey;
    int size_v=2*(size_h*frame.height/frame.width);
    for(line=0;line<frame.height;line+=size_v){
            for(a=0;a<frame .linesize;a+=size_h){
                R=frame.data[(frame.linesize)*line+a+1]-'0';
                G=frame.data[(frame.linesize)*line+a+2]-'0';
                B=frame.data[(frame.linesize)*line+a+3]-'0';

                grey=(R*76 + G*150 + B*30) >> 8;
                if(grey<20)
                    printf(" ");
                else if(grey<40) printf(".");
                else if(grey<127) printf(":");
                else if(grey<200) printf("#");
                else printf("@");
                }
            printf("\n");
       }
    
}


int main(int argc, char *argv[]){

    //一个关于颜色的测试
    //printf("\033[44m█\033[0m\n"); // \033[ 是开始ANSI转义序列的标记，37是白色前景色，41是红色背景色，0是重置所有属性到默认值  
    //printf_red("█");
    char *colorterm = getenv("COLORTERM");
    if (colorterm != NULL && strcmp(colorterm, "truecolor") == 0) {
        printf("终端支持True Color\n");
    } else {
        printf("终端不支持True Color\n");
    }


    if(decoder_init("/home/lancyx/dian/dragon.mp4")==0) //成功时返回 0 ，失败时返回 -1 
        printf("Find it!\n");
    else{
        printf("未找到视频流\n");
        return -1;
    }

    int Have_color=0;

    // int stride=2;
    // int fitter=2;
    //设置默认的池化数值.

    int rheight=5;
    int rwidth=5;
    if (argc < 2) {
        printf("Usage: %s <option>\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help        Display this help message and exit\n");
        printf("  -V, --version     Display version information and exit\n");
        //return 1; // 返回非零值表示错误
    }

    // 处理命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -h, --help        Display this help message and exit\n");
            printf("  -V, --version     Display version information and exit\n");
            return 0; // 成功执行帮助命令，返回0
        } 
        else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("MyProgram Version 1.0 (WSL Edition)\n");
            return 0; // 成功执行版本命令，返回0
        } 
        else if(strcmp(argv[i], "-c") == 0||strcmp(argv[i], "--color") == 0){
            Have_color=1;
        }
        else if(strcmp(argv[i],"-r")==0||strcmp(argv[i],"--resize")==0){
            if(argc<i+2){
                printf("%s","输入错误,长宽变量缺失");
                return -1;
            }
            // fitter=*argv[i++]-'0';
            // stride=*argv[i++]-'0';
            //调整池化相关参数
            
        }
        
        else {
            printf("Unknown option: %s\n", argv[i]);
            return 1; // 未知选项，返回错误
        }
    }



    int n= get_total_frames();
    printf("total:%d\n",n);

//解码视频
    Frame frame[3];//测试3张
    for(int i=0 ; i<3 ;i++){
        frame[i]=decoder_get_frame();
        printf("The picture%d :\nheight:%d\nwidth:%d\nline:%d\ndata:\n"
        ,i , frame[i].height , frame[i].width,frame[i].linesize);
        
    }
        
    for(int i=0;i<3;i++){  
        if(Have_color)    printf_rgb(frame[i],rwidth);
        else print_grey(frame[i],rwidth);
        printf("\n");
    }
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