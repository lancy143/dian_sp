#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // 检查是否提供了足够的参数
    if (argc < 2) {
        printf("Usage: %s <option>\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help        Display this help message and exit\n");
        printf("  -V, --version     Display version information and exit\n");
        return 1; // 返回非零值表示错误
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
            color=1;
        }
        
        else {
            printf("Unknown option: %s\n", argv[i]);
            return 1; // 未知选项，返回错误
        }
    }

    // 如果没有找到匹配的选项，可以执行其他逻辑或返回错误
    printf("No action specified.\n");
    return 1; // 返回错误
}