#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "pid.h"
#include "dir.h"

static char *__pid_file = NULL;

// 标记PID文件
static int __markCurPidFile(const char * pidFile)
{
    int maxLen;

    if(__pid_file)
    {
        free(__pid_file);
    }

    maxLen = strlen(pidFile) + 1;
    if(maxLen > MAX_PID_FILE_SIZE)
    {
        maxLen = MAX_PID_FILE_SIZE - 1;
    }

    __pid_file = calloc(sizeof(char), maxLen);
    if(__pid_file == NULL)
    {
        return 0;
    }

    snprintf(__pid_file, maxLen, "%s", pidFile);

    return 1; // OK
}


// 判断进程是否存在
//  pid ： 需要进程ID号
// 返回值：
//   存在则返回0
int IsProgressRuning(unsigned int pid)
{
    char path[64];
    sprintf(path, "/proc/%d", pid);
    return IsPathExist(path);
}

// 主动从runOnce中退出，清理PID文件
// 注意：如果已经成功调用过runOnce，那么在程序正常退出前，此函数会被自动调用
void quitFromRunOnce(void)
{
    FILE *fp = NULL;
    int pid;

    // 如果未曾登记到PID文件中，那么直接退出
    if(__pid_file == NULL)
    {
        goto early_quit;
    }

    fp = fopen(__pid_file, "r");
    /* 第一次运行，无其他程序 */
    // 如果未曾登记到PID文件中，那么直接退出
    if(fp == NULL)
    {
        goto early_quit;
    }

    // PID文件里面有登记的进程，需要检查是否为我们（如果是才可以）
    while(!feof(fp))
    {
        if(fscanf(fp, "%d", &pid)!= 1) {
            //printf("unenble to get pid !\n");
            //printf("As former\n");
        }
        // 判断pid文件内的进程是否正常
        // 1、如果pid对应的程序没有运行，那么PID文件是可以被删除的
        // 2、如果pid对应的程序居然是我们自己，那当然PID文件是可以被删除的
        if(IsProgressRuning(pid) == 0 || pid == getpid())
        {
            goto quit;
        }
    }
quit:
    if(fp != NULL)
    {
        fclose(fp);
    }
    // 如果PID文件里面的是当前进程
    remove(__pid_file);

    free(__pid_file);
    __pid_file  = NULL;

early_quit:
    return ;
}

// 保证只有一个程序启动
// pidFile  : 约定存放进程PID的文件路径
// killWho : 当发现重复运行时，要干掉哪个进程
//    -- RUNONCE_KILL_ME        : 当前进程
//    -- RUNONCE_KILL_ELDERSHIP : 之前的进程
// 返回值：
//    返回0代表成功，无需关心程序退出是否要清理PID文件，内部已经实现
//    独占时，返回0
//    非独占时：
//        -- RUNONCE_KILL_ME        : 若已有进程存在，调用者拿到异常值(-1)，应该停止继续执行
//        -- RUNONCE_KILL_ELDERSHIP : 若已有进程存在，杀死它，返回0
int runOnce(const char *pidFile, int killWho)
{
    FILE *fp;
    int pid;
    char tmp_str[64];

    if(pidFile == NULL)
    {
        return -2;
    }

    // 获取自身的PID
    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "%d", getpid());

    //printf("Checking pid file\n");

    fp = fopen(pidFile,"r");
    /* 第一次运行，无其他程序 */
    if(fp == NULL)
    {
        //printf("We are the former!\n");
        fp = fopen(pidFile,"w");

        fwrite(tmp_str, strlen(tmp_str), 1, fp);

        // 登记PID文件路径，待会正常退出程序时可以删除PID文件
        if(__markCurPidFile(pidFile))
        {
            atexit(quitFromRunOnce);
        }

        fclose(fp);
        return 0;
    }

    while(!feof(fp))
    {
        if(fscanf(fp, "%d", &pid)!= 1) {
            //printf("unenble to get pid !\n");
            //printf("As former\n");
        }
        /* 已经有程序在运行 */
        // 1. 如果策略是 禁止当前程序执行
        if(killWho == RUNONCE_KILL_ME)
        {
            // 判断pid文件内的进程是否有其他宿主
            // 1、如果pid对应的程序没有运行，那么我们还可以占用
            // 2、如果pid对应的程序是我们自己，那当然是我们自己占用的
            if(IsProgressRuning(pid) == 0 || pid == getpid())
            {
                //printf("PID mark without other host\n");
                goto asFormer;
            }
            fclose(fp);
            // 返回异常
            return -1;
        }
        // 2. 如果策略是 让已经运行的程序退出，让当前程序执行
        // 杀死老程序
        //printf("We are the latter! Killing formet.\n");
        /* send signal SIGTERM to kill */
        if(pid > 0) {
            //printf("Killing %d\n", pid);
            kill(pid, SIGTERM);
        }
    }
asFormer:
    fclose(fp);

    // 登记自己的PID到文件中
    fp = fopen(pidFile,"w");// write/tunc
    fwrite(tmp_str, strlen(tmp_str), 1, fp);
    fclose(fp);

    // 登记PID文件路径，待会正常退出程序时可以删除PID文件
    if(__markCurPidFile(pidFile))
    {
        atexit(quitFromRunOnce);
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

