#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "os.h"
#include "imu_300_vg.h"
#ifdef __cplusplus
}
#endif


int exec_imx300_vg(char * uart_dev_path, int speed)
{
    char uart_path_defalut [32] = "/dev/ttyS6";
    int uart_fd;
    int ret;


    if(uart_dev_path != NULL)
    {
        uart_fd = imx300_vg_uart_probe(uart_dev_path, speed);
    } else
    {
        uart_fd = imx300_vg_uart_probe(uart_path_defalut, speed);
    }

    if(uart_fd < 0)
    {
        printf("Dev Not Found\n");
        return -1;
    }

    ret = imx300_vg_set_output_mode(uart_fd, OUTPUT_MODE_0HZ_CS);
    if(ret != 0)
    {
        printf("设置自动输出失败\n");
    }

    // 自动输出： 角度、加速度、角速度、四元数
    ret = imx300_vg_set_auto_output_type(uart_fd, AUTO_OUTPUT_TYPE_05);
    if(ret != 0)
    {
        printf("设置自动输出失败\n");
    }

    struct imu300_vg_data vg_data;

    while(1)//for(i = 0 ; i < 100; i++)
    {
        //recv_payload_len = imx300_vg_read_all_data(uart_fd, recv_payload, sizeof(recv_payload));
        ret = imx300_vg_read_all_datas(uart_fd, &vg_data);
        if(ret != 0)
        {
            printf("请求数据失败\n");
        }
        printf("\n");
        printf("\n");
        msleep(500);
    }

    return 0;
}

int main(int argc, char * argv[])
{
#if 0
    int speed = 9600;
#else
    int speed = 115200;
#endif

    if(argc >= 3)
    {
        speed = atoi(argv[2]);
    }
    printf("Try using btrate %d\n", speed);
    if(argc != 0)
    {
        return exec_imx300_vg(argv[1], speed);
    }
    return 0;
}
