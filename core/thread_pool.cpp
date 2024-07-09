#include <iostream>
#include "minilog_static.hxx"
#include "thread_pool.hxx"

threadPool::threadPool(int thread_count,
                       int queue_size,
                       int stack_size,
                       int additionalBufSize)
    : m_maxThreadsCount(thread_count),
      m_maxQueuesCount(queue_size),
      m_stackSize(stack_size),
      m_additionalBufSize(additionalBufSize)
{
    m_pool = threadpool_create(m_maxThreadsCount,
                               m_maxQueuesCount,
                               m_stackSize,       /* Thread stack, 0 for default */
                               0,                 /* CPU Affinity*/
                               nullptr,
                               m_additionalBufSize);
    if(!m_pool)
    {
        LogE(__FILE__, __LINE__, "threadpool_create failed !");
        exit(EXIT_FAILURE);
    }
    m_curThreadsCount = 0;
}

int threadPool::addTask(void (*task)(thread_context_t *),
                        void *taskArg,     /* 任务独享的参数*/
                        void *globalArg    /* 整个线程池可见的共享参数 */,
                        int  globalArgSize /* 共享参数的大小*/)
{
    int err = threadpool_add(m_pool, task, taskArg, globalArg, globalArgSize, m_curThreadsCount + 1);
    if (err) {
        LogE(__FILE__, __LINE__, "addTask error", get_threadpool_error_messages(-err));
        return -err;
    }

    LogI(__FILE__, __LINE__, "addTask success");

    m_curThreadsCount ++;

    return 0;
}

void *getTaskArgFromContext(thread_context_t *thread_ctx)
{
    if(thread_ctx == NULL)
    {
        return NULL;
    }
    return thread_ctx->task->argument;
}

threadPool::~threadPool(void)
{
    threadpool_destroy(m_pool);
}
