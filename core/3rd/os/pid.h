#ifndef __PID__H__
#define __PID__H__

#ifdef __cplusplus
extern "C"{
#endif
// PID 文件最大长度
#define MAX_PID_FILE_SIZE  1024

// 禁止当前程序执行
#define RUNONCE_KILL_ME 0
// 让已经运行的程序退出，让当前程序执行
#define RUNONCE_KILL_ELDERSHIP 1

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
int runOnce(const char *pidFile, int killWho);

// 主动从runOnce中退出，清理PID文件
// 注意：如果已经成功调用过runOnce，那么在程序正常退出前，此函数会被自动调用
void quitFromRunOnce(void);

// 判断进程是否存在
//  pid ： 需要进程ID号
// 返回值：
//   存在则返回0
int IsProgressRuning(unsigned int pid);

#ifdef __cplusplus
}
#endif

#endif/*__PID__H__*/
