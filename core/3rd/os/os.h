#ifndef __OS_H__
#define __OS_H__

#ifdef __cplusplus
extern "C"{
#endif

// 获取文件大小
long getFileSize(const char *fileName);

// 通过管道执行命令
// cmd : 执行命令
// buf : 接收cmd执行结果的缓冲区
// buffsz : 接收缓冲区的大小（防止写溢出）
// 返回值: 为命令实际的返回值
int runCmd(char * cmd ,char *buf, int buffsz);

// 毫秒级别的sleep
int msleep(unsigned int ms);

#ifdef __cplusplus
}
#endif


#endif
