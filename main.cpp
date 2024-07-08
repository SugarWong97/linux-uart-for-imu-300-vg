#ifdef __cplusplus
extern "C"{
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "uart.h"
#ifdef __cplusplus
}
#endif

int serial_init(char * uart_port, int speed)
{
    int fd;
    int SerialSpeed = speed;

    printf("Open [%s] with [%d]\n", uart_port, speed);

    fd = serial_open(uart_port);
    if(fd == -1)
    {
        printf("open fail\n");
        return -1; // 失败
    }
    if (serial_set(fd, SerialSpeed, 0, 8, 1, 'N') != 0)
    {
        return -1;
    }
    return fd;
}

/*标示符(1) 数据长度(1) 地址码(1) 命令字(1) 数据域(n) 校验和(1)*/
//////////// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ : 数据长度
//////////// ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ : 校验和
// 标示符：固定为 77；
#define PACKAGE_INDEX_1_HEAD   0x00
/**/#define   PACKAGE_HEAD   0x77

// 数据长度：从数据长度到校验和（包括数据长度、校验和）的长度；
#define PACKAGE_INDEX_2_LEN    0x01

// 地址码：采集模块的地址，默认为 00
#define PACKAGE_INDEX_3_ADDR   0x02

// 命令字
#define PACKAGE_INDEX_4_CMD    0x03
/**/#define CMD_SEND_SET_BTRATE             0x0b   //2.1 设置通讯速率 发送命令
/**/#define CMD_RECV_SET_BTRATE             0x8b   //2.1 设置通讯速率 回复命令

/**/#define CMD_SEND_SET_DEV_ADDR           0x0f   //2.2 设置模块地址 发送命令
/**/#define CMD_RECV_SET_DEV_ADDR           0x8f   //2.2 设置通讯速率 回复命令

/**/#define CMD_SEND_GET_DEV_ADDR           0x1f   //2.3 查询当前地址 发送命令
/**/#define CMD_RECV_GET_DEV_ADDR           0x1f   //2.3 查询当前地址 回复命令

/**/#define CMD_SEND_GET_G                  0x54   // 2.4 查询重力加速度G值 发送命令
/**/#define CMD_RECV_GET_G                  0x54   // 2.4 查询重力加速度G值 回复命令

/**/#define CMD_SEND_GET_W                  0x50   // 2.5 查询角速度ω 发送命令
/**/#define CMD_RECV_GET_W                  0x50   // 2.5 查询角速度ω 回复命令

/**/#define CMD_SEND_SET_OUTPUT_MODE        0x0c   // 2.6 设置输出角度模式 发送命令
/**/#define CMD_RECV_SET_OUTPUT_MODE        0x8c   // 2.6 设置输出角度模式 回复命令

/**/#define CMD_SEND_SET_AUTO_OUTPUT_TYPE   0x56   // 2.7 设置输出角度模式 发送命令
/**/#define CMD_RECV_SET_AUTO_OUTPUT_TYPE   0x56   // 2.7 设置输出角度模式 回复命令

/**/#define CMD_SEND_GYROSCOPE_CALIB        0x52   // 2.8 陀螺仪校准 发送命令
/**/#define CMD_RECV_GYROSCOPE_CALIB        0xa5   // 2.8 陀螺仪校准 回复命令

/**/#define CMD_SEND_SAVE_CONFIG            0x0a   // 2.9 陀螺仪校准 发送命令
/**/#define CMD_RECV_SAVE_CONFIG            0x8a   // 2.9 陀螺仪校准 回复命令

// 数据域：根据命令字不同内容和长度相应变化；
#define PACKAGE_INDEX_4_DATA   0x04
/**/#define RECV_DATA_VALUE_OK   0x00
/**/#define RECV_DATA_VALUE_ERR   0xff

unsigned char g_dev_addr = 0x00;
void set_dev_addr(unsigned char addr)
{
    g_dev_addr = addr;
}

unsigned char get_dev_addr(void)
{
    return g_dev_addr;
}

// 检验报文是否符合协议规范
static int _check_imx300_vg_cmd(const unsigned char *cmd_buff, unsigned char cmd_code_except = RECV_DATA_VALUE_ERR)
{
    unsigned short checksum_cal = 0; // 没关系，先超出，只取低16bit
    unsigned char  checksum_recv = 0;
    unsigned char *checksum_zone;
    unsigned char data_len = 0;
    unsigned char *payload_zone;
    unsigned char cmd_code = 0;
    int i;

    if(cmd_buff == NULL)
    {
        return -1;
    }

    // 1. 判断标识符
    if(cmd_buff[PACKAGE_INDEX_1_HEAD] != PACKAGE_HEAD)
    {
        printf("错误的帧头 [%02x]\n", cmd_buff[PACKAGE_INDEX_1_HEAD]);
        return -1;
    }
    // 2. 获取数据长度
    data_len = cmd_buff[PACKAGE_INDEX_2_LEN];

    // 3. 判断地址码
    if(cmd_buff[PACKAGE_INDEX_3_ADDR] != get_dev_addr())
    {
        printf("这条数据可能不是发给我们的\n");
        //return -1;
    }

    // 4. 获取命令字
    cmd_code = cmd_buff[PACKAGE_INDEX_4_CMD];
    if(cmd_code_except != RECV_DATA_VALUE_ERR)
    {
        if(cmd_code != cmd_code_except)
        {
            printf("错误命令字 [%02x]\n", cmd_code);
        }
    }

#if 1
    // 5. 获取数据域
    //payload_zone = &cmd_buff[ cmd_buff[PACKAGE_INDEX_2_LEN] - 1];
    //for( i = 0; i <  cmd_buff[PACKAGE_INDEX_2_LEN] - PACKAGE_INDEX_4_DATA; i++)
    payload_zone = (unsigned char *)&cmd_buff[ data_len - 1];
    for( i = 0; i <  data_len - PACKAGE_INDEX_4_DATA; i++)
    {
        printf("DATA [%d] is [%02x]\n", i, payload_zone[i]);
    }
#endif

    // 6. 判断校验和
    checksum_zone = (unsigned char *)&cmd_buff[PACKAGE_INDEX_2_LEN];

    checksum_cal = 0;
    for(i = 0; i < data_len - 1/*不需要把校验和也算进去*/; i++)
    {
        checksum_cal += checksum_zone[i];
        checksum_cal = checksum_cal & 0xff;
        //printf("[%02x] ", checksum_cal);
    }
    //printf("\n");

    checksum_recv =  cmd_buff[data_len];
    if(checksum_cal != checksum_recv)
    {
        printf("错误的校验和\n");
        return -1;
    }

    return 0;
}

// 根据指定的数据、命令字、数据域填充一条合法的报文
int _gen_imx300_vg_cmd(unsigned char *cmd_buff, unsigned char max_send_len,
        unsigned char cmd_code, unsigned char *data, unsigned char data_len)
{
    unsigned short checksum_cal = 0; // 没关系，先超出，只取低16bit
    unsigned char *checksum_zone ;
    unsigned char *payload_zone;
    int i;

    if(cmd_buff == NULL)
    {
        return 0;
    }
    // 0. 清零
    memset(cmd_buff, 0, max_send_len);

    // 1. 填充标识符
    cmd_buff[PACKAGE_INDEX_1_HEAD] = PACKAGE_HEAD;

    // 2. 填充数据长度
    cmd_buff[PACKAGE_INDEX_2_LEN] = data_len + PACKAGE_INDEX_4_DATA;

    // 3. 填充地址码
    if(cmd_code == CMD_SEND_GET_DEV_ADDR)
    {
        // 查询设备地址时，只能填入这个
        cmd_buff[PACKAGE_INDEX_3_ADDR] = 0x00;
    }else
    {
        cmd_buff[PACKAGE_INDEX_3_ADDR] = get_dev_addr();
    }

    // 4. 填充命令字
    cmd_buff[PACKAGE_INDEX_4_CMD] = cmd_code;

    // 5. 填充数据域
    payload_zone = &cmd_buff[PACKAGE_INDEX_4_DATA];
    for( i = 0; i < data_len; i++)
    {
        if(data == NULL)
        {
            break;
        }
        payload_zone[i] = data[i];
        //printf("Fill Data [%d] is [%02x]\n", i, payload_zone[i]);
    }

    // 6. 填充校验和
    checksum_zone = &cmd_buff[PACKAGE_INDEX_2_LEN];

    for(i = 0; i < data_len + PACKAGE_INDEX_4_DATA; i++)
    {
        checksum_cal += checksum_zone[i];
        checksum_cal = checksum_cal & 0xff;
        //printf("[%02x] ", checksum_cal);
    }

    cmd_buff[PACKAGE_INDEX_4_DATA + data_len] = checksum_cal;

    /*
    // 打印生成的报文，调试用
    for(i = 0; i < cmd_buff[PACKAGE_INDEX_2_LEN] + 1; i++)
    {
        printf("DUMP [%02x] \n", cmd_buff[i]);
    }
    printf("\n");
    */
    printf("%d\n", cmd_buff[PACKAGE_INDEX_2_LEN] + 1);
    return cmd_buff[PACKAGE_INDEX_2_LEN] + 1;
}

// 发送请求并接收回复
// 只需要关心要发什么 命令字，数据域和长度即可
// 待会就可以在recv_data里面收到对应的数据域
// 返回值是recv_data所写的数
int imx300_vg_cmd_send_and_recv(int uart_fd,
        unsigned char send_cmd_code, unsigned char *send_data, unsigned char send_data_len,
        unsigned char recv_cmd_code, unsigned char *recv_data, unsigned char max_recv_len)
{
    unsigned char data_len;
    unsigned char *payload_zone;
    unsigned char payload_len;

    unsigned char send_buff[128], send_len;
    unsigned char recv_buff[128], recv_len;

    int timeout_s = 1;
    int timeout_us = 5000;
    int i;

    if(uart_fd < 0)
    {
        return -1;
    }
    // 1. 发送请求
    send_len = _gen_imx300_vg_cmd(send_buff, sizeof(send_buff), send_cmd_code, send_data, send_data_len);
    serial_send(uart_fd, send_buff, send_len);

    // 2. 查看是否有数据返回
    recv_len = serial_recv(uart_fd, recv_buff, sizeof(recv_buff), timeout_s, timeout_us);

    if (recv_len <= 0)
    {
        goto err;
    }

    // 3. 验证返回的数据对不对
    if(_check_imx300_vg_cmd(recv_buff, recv_cmd_code) != 0)
    {
        goto err;
    }
    if(recv_data == NULL)
    {
        return 0;
    }
    //recv_buff[]
    memset(recv_data, 0, max_recv_len);

    // 3. 截取数据域进行返回
    data_len = recv_buff[PACKAGE_INDEX_2_LEN];

    payload_zone = &recv_buff[ data_len - 1];
    payload_len  = data_len - PACKAGE_INDEX_4_DATA;
    for( i = 0; i < payload_len; i++)
    {
        if(max_recv_len == i)
        {
            break;
        }
        recv_data[i] = payload_zone[i];
    }
    //for(i = 0; i < )

    return payload_len;

err:
    return -1;
}


//注：
//0x00 表示 2400，
//0x01 表示 4800，
//0x02 表示 9600，
//0x03 表示 19200，
//0x04 表示 115200，
//0x05 表示 38400，
//0x06 表示 57600，
//0x07 表示 460800，
//默认值为 0x02:9600，
//每次变更通讯波特率成功之后，会以原波特率发送回应答命令， 然后立即改变设备通信波特率。
//备注：如果需要高频输出，请将波特率设为 115200 或者 460800，修改波特率不需要发送保存命令，立即生效
static unsigned char brate_to_cmd_value(int speed)
{
    switch(speed)
    {
        /* */ case   2400: return 0x00;
        break;case   4800: return 0x01;
        break;case   9600: return 0x02;
        break;case  19200: return 0x03;
        break;case 115200: return 0x04;
        break;case  38400: return 0x05;
        break;case  57600: return 0x06;
        break;case 460800: return 0x07;
    }
    return RECV_DATA_VALUE_ERR;
}

// 设置波特率(前提是需要知道老的波特率是多少)
// 设置通讯速率
// 发送命令： 77 05 xx 0B 02 yy
// 应答命令:  77 05 xx 8b 00 yy
int imx300_vg_change_brate(int uart_fd, int new_speed)
{
    unsigned char brate_value;
    unsigned char recv_payload[6] = {0};
    int recv_payload_len;

    brate_value = brate_to_cmd_value(new_speed);
    if(brate_value == RECV_DATA_VALUE_ERR)
    {
        goto err;
    }
    // 1. 发送 设置波特率的请求
    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SET_BTRATE, &brate_value, 1,
            CMD_RECV_SET_BTRATE, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    // 2. 判断返回值是否成功
    if(recv_payload[0] == RECV_DATA_VALUE_ERR)
    {
        goto err;
    }

    return 0;

err:
    return -1;
}

// 设置模块地址（需要提前知道正确的地址）
// 发送命令： 77 05 XX 0F 01 15
// 应答命令： 77 05 ZZ 8f ZZ yy
int imx300_vg_change_dev_addr(int uart_fd, unsigned char new_addr)
{
    unsigned char tmp_value;
    unsigned char recv_payload[6] = {0};
    int recv_payload_len;

    tmp_value = new_addr;
    // 1. 发送请求
    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SET_DEV_ADDR, &tmp_value,   1,
            CMD_RECV_SET_DEV_ADDR, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    // 2. 判断返回值是否成功
    if(recv_payload[0] == RECV_DATA_VALUE_ERR)
    {
        goto err;
    }
    set_dev_addr(new_addr);

    return 0;

err:
    return -1;
}

// 查询当前地址
// 发送命令： 77 04 00 1F 23
// 应答命令： 77 05 XX 1F XX yy
int imx300_vg_get_dev_addr(int uart_fd)
{
    unsigned char recv_payload[6] = {0};
    int recv_payload_len;

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_GET_DEV_ADDR, NULL, 0,
            CMD_RECV_GET_DEV_ADDR, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    set_dev_addr(recv_payload[0]);

    return 0;

err:
    return -1;
}

// 查询重力加速度 G 值
// 发送命令： 77 04 XX 54 58
// 应答命令： 77 0d XX 54 [9] yy
int imx300_vg_get_g_value(int uart_fd)
{
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;
    int i;

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_GET_G, NULL, 0,
            CMD_RECV_GET_G, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    //注：数据域部分为俯仰、横滚、 Z 轴（垂直水平面）的 g 值大小，由 1 位符号位+1 位整数位+4 位小数位组成。
    //如返回值为 “77 0D 00 54 00 01 07 00 94 21 10 06 30 64”，
    //则分别为 0.0107g， 0.9421g， -0.0630g。
    for(i = 0; i < recv_payload_len; i++)
    {
        printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    }

    return 0;

err:
    return -1;
}

// 查询角速度ω
// 发送命令： 77 04 XX 50 54
// 应答命令： 77 0d XX 50 [9] yy
int imx300_vg_get_w_value(int uart_fd)
{
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;
    int i;

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_GET_W, NULL, 0,
            CMD_RECV_GET_W, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    // 注：数据域部分为俯仰、横滚、 Z 轴（垂直水平面）角速度的大小，由 1 位符号位+3 位整数位+2 位小数位组成。
    // 例如返回值为 “77 0D 00 50 10 93 76 12 98 87 00 14 03 BE”
    // 则数据域部分分别为：
    //   俯仰轴角速度： -93.76°/s ，
    //   横滚轴角速度： -298.87°/s ，
    //   Z 轴角速度： +14.03°/s。
    for(i = 0; i < recv_payload_len; i++)
    {
        printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    }

    return 0;

err:
    return -1;
}

//发送的数据域 XX 为自动输出频率选项：
//  00 表示应答模式
//  01 表示 5Hz 自动输出数据
//  02 表示 10Hz 自动输出数据
//  03 表示 20Hz 自动输出数据
//  04 表示 25Hz 自动输出数据
//  05 表示 50Hz 自动输出数据
//  06 表示 100HZ 自动输出数据
//  07 表示 200HZ 自动输出数据
//  08 表示 500HZ 自动输出数据
// 注： 自动输出数据类型参数根据数据类型选择命令决定，默认是自动输出角速度。
enum output_mode {
    OUTPUT_MODE_5HZ = 1,
    OUTPUT_MODE_10HZ = 2,
    OUTPUT_MODE_20HZ = 3,
    OUTPUT_MODE_25HZ = 4,
    OUTPUT_MODE_50HZ = 5,
    OUTPUT_MODE_100HZ = 6,
    OUTPUT_MODE_200HZ = 7,
    OUTPUT_MODE_500HZ = 8,
    OUTPUT_MODE_BUTT,
};

//unsigned char output_mode_to_cmd_value(int mode)
//{
//    switch(mode)
//    {
//        /* */ case   5: return  OUTPUT_MODE_5HZ  ;
//        break;case  10: return  OUTPUT_MODE_10HZ ;
//        break;case  20: return  OUTPUT_MODE_20HZ ;
//        break;case  25: return  OUTPUT_MODE_25HZ ;
//        break;case  50: return  OUTPUT_MODE_50HZ ;
//        break;case 100: return  OUTPUT_MODE_100HZ;
//        break;case 200: return  OUTPUT_MODE_200HZ;
//        break;case 500: return  OUTPUT_MODE_500HZ;
//    }
//    return RECV_DATA_VALUE_ERR;
//}

// 设置输出角度模式
// 发送命令： 77 05 xx 0C 00 yy
// 应答命令： 77 05 XX 8c 00 yy
int imx300_vg_set_output_mode(int uart_fd, enum output_mode mode)
{
    unsigned char tmp_value;
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;
    int i;

    if(mode >= OUTPUT_MODE_BUTT)
    {
        return -1;
    }
    tmp_value = mode;
    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SET_OUTPUT_MODE, &tmp_value,   1,
            CMD_RECV_SET_OUTPUT_MODE, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    for(i = 0; i < recv_payload_len; i++)
    {
        printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    }

    return 0;

err:
    return -1;
}

// 注：数据域 XX 为对应输出数据类型，需在自动输出模式下进行：
// 0x00:自动输出时，输出参数为三轴角速度数据，输出格式参考命 2.5；
// 0x01:自动输出时，输出参数为三轴加速度值，输出格式参考命令 2.4；
// 0x02:自动输出时，输出参数由三轴角速度、加速度组成，
// 例如返回值为
//   “77 16 00 84 10 93 76 12 98 87 00 14 03 00 01 07 00 94 21 10 06 30 FE”
//   则数据域部分分别为：
//   X 轴角速度： -93.76°/s ，
//   Y 轴角速度： -298.87°/s ，
//   Z 轴角速度： +14.03°/s，
//   X 轴加速度： 0.0107g，
//   Y 轴加速度： 0.9421g，
//   Z 轴加速度： -0.0630g 。
enum auto_output_type {
    AUTO_OUTPUT_TYPE_00 = 1,
    AUTO_OUTPUT_TYPE_01 = 2,
    AUTO_OUTPUT_TYPE_02 = 3,
    AUTO_OUTPUT_TYPE_BUTT,
};

// 自动输出数据类型设置
// 发送命令： 77 05 xx 56 mm yy
// 应答命令： 77 05 xx 56 00 yy
int imx300_vg_set_output_mode(int uart_fd, enum auto_output_type mode)
{
    unsigned char tmp_value;
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;
    int i;

    if(mode >= AUTO_OUTPUT_TYPE_BUTT)
    {
        return -1;
    }
    tmp_value = mode;
    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SET_AUTO_OUTPUT_TYPE, &tmp_value,   1,
            CMD_RECV_SET_AUTO_OUTPUT_TYPE, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    for(i = 0; i < recv_payload_len; i++)
    {
        printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    }

    return 0;

err:
    return -1;
}

// 陀螺仪校准
// 发送命令： 77 04 XX 52 yy
// 应答命令： 77 05 XX a5 01 yy
//注：当传感器出现动态环境下表现性能不佳或者在静止时读取陀螺仪角速度值不在零位时，可以发送该命令矫正陀螺仪零偏。
//该命令只能在传感器绝对静止时使用以获取最佳校准效果。
int imx300_vg_do_gyroscope_calib(int uart_fd)
{
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_GYROSCOPE_CALIB, NULL,   0,
            CMD_RECV_GYROSCOPE_CALIB, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    if(recv_payload[0] != 0x01)
    {
        printf("DATA is [%02x]\n", recv_payload[0]);
        goto err;
    }

    return 0;

err:
    return -1;
}

// 保存设置
// 发送命令： 77 04 XX 0a 0e yy
// 应答命令： 77 04 XX 8a 00 yy
// 注：设置波特率不需要保存设置，其他设置项均需要发送保存设置。
int imx300_vg_save_config(int uart_fd)
{
    unsigned char recv_payload[32] = {0};
    int recv_payload_len;

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SAVE_CONFIG, NULL,   0,
            CMD_RECV_SAVE_CONFIG, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    if(recv_payload[0] == RECV_DATA_VALUE_ERR)
    {
        goto err;
    }

    return 0;

err:
    return -1;
}


// 尝试以不同的波特率和设备建立通信
// uart_dev_path : 设备所在的串口总线上
// speed : 期望使用的波特率通信速率
//
// 返回值：成功返回 可用于uart读写的fd，失败返回-1；
// 同时会更新成功收到的地址码
int imx300_vg_uart_probe(char * uart_dev_path, int speed)
{
    int uart_fd = -1;

    int speeds [] = {4800, 9600, 19200, 115200, 38400, 57600, 460800,2400};

    int ret;
    int speed_cur = -1;

    unsigned int i;

    if(uart_dev_path == NULL)
    {
        return -1;
    }

    for(i = 0; i < sizeof(speeds)/sizeof(speeds[0]); i++)
    {
        // 1. 以某个波特率打开串口
        uart_fd = serial_init(uart_dev_path, speeds[i]);
        if(uart_fd < 0)
        {
            return -1;
        }

        // 2. 通过 查询当前设备地址 的命令验证这个波特率能否跑通
        ret = imx300_vg_get_dev_addr(uart_fd);
        if (ret != 0)
        {
            close(uart_fd);
            continue;
        }
        printf("Using BTRATE %d\n", speeds[i]);
        break;
    }
    if(speed_cur != -1)
    {
        //imx300_vg_change_brate(uart_fd, speed);
    }

    return uart_fd;
}

int uart_demo(char * uart_dev_path, int speed)
{
    // 打开串口
    char uart_path_defalut [32] = "/dev/ttyS6";
    int uart_fd;

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

    // 接收uart
    unsigned char recv_buff[256];
    int timeout_s = 0;
    int timeout_us = 3000;
    int len;

    printf("Start Echo.\n");
    while(1)
    {
        len = serial_recv(uart_fd, recv_buff, sizeof(recv_buff), timeout_s, timeout_us);
        if (len <= 0)
        {
            continue;
        }
        // 将收到的数据发送出去
        serial_send(uart_fd, recv_buff, len);
    }

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

    if(argc >= 2)
    {
        speed = atoi(argv[2]);
    }
    if(argc != 0)
    {
        return uart_demo(argv[1], speed);
    }
    return 0;
}
