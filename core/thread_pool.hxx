#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "timeut.h"
#include "misc.h"

#include "threadpool.h"

#ifdef __cplusplus
}
#endif


class threadPool
{
public:
    threadPool(int thread_count = 20,
               int queue_size = 512,
               int stack_size = 0,
               int additionalBufSize = 0);

    int addTask(void (*task)(thread_context_t *),
                void *taskArg,               /* 任务独享的参数*/
                void *globalArg = nullptr,   /* 整个线程池可见的共享参数 */
                int  globalArgSize  = 0      /* 共享参数的大小*/
                );
    ~threadPool (void);

private:
    /* 线程池实例 */
    threadpool_t * m_pool;
    // 工作线程数量
    int m_maxThreadsCount;
    // 从任务队列（maxQueuesCount）中取出一个任务交给一个线程去执行
    int m_maxQueuesCount;
    // 线程栈大小（默认为0）
    int m_stackSize;
    // 额外的数据大小
    int m_additionalBufSize;
    int m_curThreadsCount;
};

void *getTaskArgFromContext(thread_context_t *thread_ctx);

#endif /* ifndef THREAD_POOL_H */

