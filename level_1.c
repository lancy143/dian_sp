#include <stdio.h>
#include "xargparse.h"
#include <libavformat/avformat.h>  
#include <libavcodec/avcodec.h>
#include "video_decoder.h"
Frame resize_ave(Frame frame,int STRIDE){//水平和垂直改变程度
    Frame output;
    int temp1,temp2,temp3;
    output.height=frame.height/STRIDE;//
    output.width=frame.width/STRIDE;
    output.linesize=frame.linesize/STRIDE;
    for(int i=0;i<frame.height;i+=STRIDE){//i标记行数
        for(int j=0;j<frame.width;j+=STRIDE){//j跟踪每一行的元素,j对output进行标记，j*3为原序列的索引
            for(int q=0;q<3;q++){//循环分别对R、G、B、三个数值进行池化
                 temp1=frame.data[i*frame.linesize+j*3+q]>frame.data[i*frame.linesize+j*3+q+3]?
                 frame.data[i*frame.linesize+j*3+q]:frame.data[i*frame.linesize+j*3+q+3];
                 temp2=frame.data[(i+1)*frame.linesize+j*3+q]>frame.data[(i+1)*frame.linesize+j*3+q+3]?
                 frame.data[(i+1)*frame.linesize+j*3+q]:frame.data[(i+1)*frame.linesize+j*3+q+3];
                 temp3 = temp1 > temp2 ? temp1 : temp2;
                 output.data[i/2*output.linesize+j+q]=temp3;
            }
            
        }
    }
}

Frame resize_max(Frame frame,int level){

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

    int stride=2;
    int fitter=2;
    //设置默认的池化数值.

    // int rheight=5;
    // int rwidth=5;
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
            fitter=*argv[i++]-'0';
            stride=*argv[i++]-'0';
            //调整池化相关参数
            
        }
        
        else {
            printf("Unknown option: %s\n", argv[i]);
            return 1; // 未知选项，返回错误
        }
    }



    int n= get_total_frames();
    printf("total:%d\n",n);

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