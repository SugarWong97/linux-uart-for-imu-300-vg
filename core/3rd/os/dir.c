#ifdef __cplusplus
extern "C"{
#endif
#include <unistd.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>

#include "dir.h"

//检查目录/文件(所有类型是否存在
// path ： 需要判断的路径
// 返回值：
//   存在则返回0
int IsPathExist(const char* path)
{
    return !access(path, F_OK);
}

//获取程序所在目录
// path ： 需要接收的buffer
// maxWrite ： 需要接收的buffer的最大长度
// 返回值：
//   参数错误时时返回-1；成功时返回路径长度（限制于maxWrite）
int getCurrentPath(char * path, int maxWrite)
{
    int min;
    int ret;

    if(path == NULL)
    {
        return -1;
    }
    min = maxWrite > MAX_DIR_LEN ? MAX_DIR_LEN : maxWrite;
    ret = readlink("/proc/self/exe", path, min);
    if(ret != -1)
    {
        path[ret] = '\0';
    }

    return ret;
}

#ifdef __cplusplus
}
#endif

