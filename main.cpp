#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

    //unsigned char recv_payload[32] = {0};
    //int recv_payload_len;
    //int i;
    //recv_payload_len = imx300_vg_get_g_value(uart_fd, recv_payload, sizeof(recv_payload));
    //for(i = 0; i < recv_payload_len; i++)
    //{
    //    //printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    //    printf("0x%02x, ", recv_payload[i]);
    //}

    unsigned char recv_payload[64] = {0};
    int recv_payload_len;
    int i;
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
        recv_payload_len = imx300_vg_read_all_data_to_frame(uart_fd, recv_payload, sizeof(recv_payload));
        //printf("Recv [%d] : ", recv_payload_len);
        //for(i = 0; i < recv_payload_len; i++)
        //{
        //    //printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
        //    printf("0x%02x, ", recv_payload[i]);
        //}
        imx300_vg_cal_recv_data(recv_payload, &vg_data);
        printf("\n");
        printf("\n");
    }

#if 0
    unsigned char recv_buff[48], recv_len;

    // 自动输出： 角度、加速度、角速度、四元数
    ret = imx300_vg_set_auto_output_type(uart_fd, AUTO_OUTPUT_TYPE_05);
    if(ret != 0)
    {
        printf("设置自动输出失败\n");
    }
    ret = imx300_vg_set_output_mode(uart_fd, OUTPUT_MODE_5HZ);
    if(ret != 0)
    {
        printf("设置自动输出失败\n");
    }
    // 接收自动数据
    //while(1)
    for(int i = 0 ; i < 10; i++)
    {
        recv_len = serial_recv(uart_fd, recv_buff, sizeof(recv_buff), timeout_s, timeout_us);
        cal_recv_data(recv_buff);
    }
    ret = imx300_vg_set_output_mode(uart_fd, OUTPUT_MODE_0HZ_CS);
#endif

    return 0;
}

void test_cmd_format(void)
{
#if 0
    unsigned char test_cmd_get_addr[] = {0x77, 0x04, 0x00, 0x1F, 0x23};
    unsigned char test_cmd_set_brate[] = {0x77, 0x05, 0x00, 0x0b, 0x02, 0x12};

    _check_imx300_vg_cmd(test_cmd_get_addr);
    _check_imx300_vg_cmd(test_cmd_set_brate);

    unsigned char gen_buff[128] = {0};
    unsigned char data = 0x02;
    _gen_imx300_vg_cmd(gen_buff, sizeof(gen_buff), 0x0b, &data, 1);
    _check_imx300_vg_cmd(gen_buff);
#endif
}

int main(int argc, char * argv[])
{
    int speed = 9600;

    if(argc >= 3)
    {
        speed = atoi(argv[2]);
    }
    if(argc != 0)
    {
        return exec_imx300_vg(argv[1], speed);
    }
    return 0;
}
