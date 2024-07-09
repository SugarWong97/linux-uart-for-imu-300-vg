#ifdef __cplusplus
extern "C"{
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "os.h"
#include "imu_300_vg.h"
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

unsigned char g_dev_addr = 0x00;
void set_dev_addr(unsigned char addr)
{
    g_dev_addr = addr;
    printf("Dev Addr now is [%02x]\n", addr);
}

unsigned char get_dev_addr(void)
{
    return g_dev_addr;
}

// 检验报文是否符合协议规范
static int _check_imx300_vg_cmd(const unsigned char *cmd_buff, unsigned char cmd_code_except /*= RECV_DATA_VALUE_ERR*/)
{
    unsigned short checksum_cal = 0; // 没关系，先超出，只取低16bit
    unsigned char  checksum_recv = 0;
    unsigned char *checksum_zone;
    unsigned char data_len = 0;
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

#if 0
    unsigned char *payload_zone;
    // 5. 获取数据域
    //for( i = 0; i <  cmd_buff[PACKAGE_INDEX_2_LEN] - PACKAGE_INDEX_4_DATA; i++)
    payload_zone = (unsigned char *)&cmd_buff[ PACKAGE_INDEX_4_DATA];
    for( i = 0; i <  data_len - PACKAGE_INDEX_4_DATA; i++)
    {
        printf("Dump DATA [%d] is [%02x]\n", i, payload_zone[i]);
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
    //printf("%d\n", cmd_buff[PACKAGE_INDEX_2_LEN] + 1);
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
    msleep(1);

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

    payload_zone = &recv_buff[PACKAGE_INDEX_4_DATA];
    payload_len  = data_len - PACKAGE_INDEX_4_DATA;
    //printf("payload_len is %d\n", payload_len);
    for( i = 0; i < payload_len; i++)
    {
        if(max_recv_len == i)
        {
            break;
        }
        recv_data[i] = payload_zone[i];
    }

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

// 设置波特率(前提是需要老的波特率上正常通信)
// 设置通讯速率
// 发送命令： 77 05 xx 0B 02 yy
// 应答命令:  77 05 xx 8b 00 yy
int imx300_vg_change_brate(char * uart_dev_path, int uart_fd, int new_speed)
{
    int new_uart_fd = -1;
    unsigned char brate_value;
    unsigned char recv_payload[128] = {0};

    brate_value = brate_to_cmd_value(new_speed);
    if(brate_value == RECV_DATA_VALUE_ERR)
    {
        return -1;
    }
    // 每次变更通讯波特率成功之后，会以原波特率发送回应答命令，
    imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_SET_BTRATE, &brate_value, 1,
            CMD_RECV_SET_BTRATE, recv_payload, sizeof(recv_payload));

    serial_recv(uart_fd, recv_payload, sizeof(recv_payload), 2, 0);
    tcflush(uart_fd, TCIOFLUSH);
    msleep(500); //必须在open和tcflush（或ioctl）操作间进行延时操作（具体时间未做验证），否则没有清空效果，原因未知，可能跟Linux内核版本有关
    close(uart_fd);

    // 然后立即改变设备通信波特率。
    // 备注：如果需要高频输出，请将波特率设为 115200 或者 460800，修改波特率不需要发送保存命令，立即生效。
    new_uart_fd = serial_init(uart_dev_path, new_speed);
    if(new_uart_fd < 0)
    {
        return -1;
    }
    serial_recv(new_uart_fd, recv_payload, sizeof(recv_payload), 2, 0);
    tcflush(uart_fd, TCIOFLUSH);
    msleep(500); //必须在open和tcflush（或ioctl）操作间进行延时操作（具体时间未做验证），否则没有清空效果，原因未知，可能跟Linux内核版本有关
    printf("Using BTRATE! %d\n", new_speed);

    return new_uart_fd;
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
int imx300_vg_get_g_value(int uart_fd, unsigned char *recv_payload, int max_payload_len)
{
    int recv_payload_len;

    if(recv_payload == NULL)
    {
        goto err;
    }

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_GET_G, NULL, 0,
            CMD_RECV_GET_G, recv_payload, max_payload_len);
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    //注：数据域部分为俯仰、横滚、 Z 轴（垂直水平面）的 g 值大小，由 1 位符号位+1 位整数位+4 位小数位组成。
    //如返回值为 “77 0D 00 54 00 01 07 00 94 21 10 06 30 64”，
    //则分别为 0.0107g， 0.9421g， -0.0630g。
    // int i;
    //for(i = 0; i < recv_payload_len; i++)
    //{
    //    printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    //}

    return recv_payload_len;

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
    //int i;
    //for(i = 0; i < recv_payload_len; i++)
    //{
    //    printf("DATA [%d] is [%02x]\n", i, recv_payload[i]);
    //}

    return 0;

err:
    return -1;
}

// 自动输出数据类型设置
// 发送命令： 77 05 xx 56 mm yy
// 应答命令： 77 05 xx 56 00 yy
int imx300_vg_set_auto_output_type(int uart_fd, enum auto_output_type mode)
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

int imx300_vg_read_all_data(int uart_fd, unsigned char *recv_payload, int max_payload_len)
{
    int recv_payload_len;

    if(recv_payload == NULL)
    {
        goto err;
    }

    recv_payload_len = imx300_vg_cmd_send_and_recv(uart_fd,
            CMD_SEND_READ_ALL_DATA, NULL,   0,
            CMD_RECV_READ_ALL_DATA, recv_payload, sizeof(recv_payload));
    if(recv_payload_len <= 0)
    {
        goto err;
    }
    //if(recv_payload[0] != 0x01)
    //{
    //    printf("DATA is [%02x]\n", recv_payload[0]);
    //    goto err;
    //}

    return 0;

err:
    return -1;
}

int imx300_vg_read_all_data_to_frame(int uart_fd, unsigned char *recv_buff, int max_payload_len)
{
    unsigned char recv_len;
    unsigned char send_cmd_code = CMD_SEND_READ_ALL_DATA;
    unsigned char recv_cmd_code = CMD_RECV_READ_ALL_DATA;

    unsigned char send_buff[64], send_len;

    int timeout_s = 1;
    int timeout_us = 5000;

    if(uart_fd < 0)
    {
        return -1;
    }

    // 1. 发送请求
    send_len = _gen_imx300_vg_cmd(send_buff, sizeof(send_buff), send_cmd_code, NULL, 0);
    serial_send(uart_fd, send_buff, send_len);

    // 2. 查看是否有数据返回
    recv_len = serial_recv(uart_fd, recv_buff, max_payload_len, timeout_s, timeout_us);

    if (recv_len <= 0)
    {
        goto err;
    }

    // 3. 验证返回的数据对不对
    if(_check_imx300_vg_cmd(recv_buff, recv_cmd_code) != 0)
    {
        goto err;
    }

    return recv_len;

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

    int speeds [] = {9600, 19200, 115200, 38400, 57600, 460800,2400, 4800};

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
        speed_cur = speeds[i];
        printf("Using BTRATE %d\n", speeds[i]);
        break;
    }
    if(speed_cur != -1 && speed_cur != speed)
    {
        uart_fd = imx300_vg_change_brate(uart_dev_path, uart_fd, speed);
    }

    return uart_fd;
}

//////////////////////////////////////////////////////
const char hex_table[] = {
    '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
};

static void to_hex_str(unsigned char *s,int len,char* d)
{
    while(len--){
        *(d++)=hex_table[*s>>4];
        *(d++)=hex_table[*(s++)&0x0f];
    }
}


int imx300_vg_cal_recv_data(unsigned char * buffer, struct imu300_vg_data *vg_data)
{
    //unsigned char buffer[48]={
    //    0x77,0x2F,0x00,0x59,0x10,0x00,0x60,0x10,0x03,0x06,0x00,0x00,0x00,
    //    0x10,0x01,0x07,0x10,0x05,0x43,0x01,0x01,0x54,
    //    0x10,0x00,0x13,0x10,0x00,0x04,0x00,0x00,0x09, 0x10,0x87,0x06,0x35,0x00,0x01,0x76,0x91,0x00,0x02,0x06,0x94,0x00,0x49,0x11,0x75,0x5C
    //};
    char hexstr[97] = {0};

    //hexstr[96] = '\0';
    //msmeset(hexstr, 0, sizeof(a));
    to_hex_str(buffer, 48, hexstr);

    //rpy
    double yaw;
    yaw = (hexstr[9] - '0') * 100 + (hexstr[10] - '0') * 10 + (hexstr[11] - '0') + (hexstr[12] - '0') * 0.1 + (hexstr[13] - '0') * 0.01;
    yaw = (hexstr[8] == '1') ? (-1 * yaw) : yaw;

    double pitch = (hexstr[15] - '0') * 100 + (hexstr[16] - '0') * 10 + (hexstr[17] - '0') + (hexstr[18] - '0') * 0.1 + (hexstr[19] - '0') * 0.01;
    pitch = (hexstr[14] == '1') ? (-1 * pitch) : pitch;
    //cout << pitch << "Centigrade" << endl;

    double roll = (hexstr[21] - '0') * 100 + (hexstr[22] - '0') * 10 + (hexstr[23] - '0') + (hexstr[24] - '0') * 0.1 + (hexstr[25] - '0') * 0.01;
    roll = (hexstr[20] == '1') ? (-1 * roll) : roll;
    //cout << roll << "Centigrade" << endl;

    //x_y_z_acceleratio
    double x_acc = (hexstr[27] - '0') + (hexstr[28] - '0') * 0.1 + (hexstr[29] - '0') * 0.01 + (hexstr[30] - '0') * 0.001 + (hexstr[31] - '0') * 0.0001;
    x_acc = (hexstr[26] == '1') ? (-1 * x_acc) : x_acc;
    //cout << x_acc << "g" << endl;

    double y_acc = (hexstr[33] - '0') + (hexstr[34] - '0') * 0.1 + (hexstr[35] - '0') * 0.01 + (hexstr[36] - '0') * 0.001 + (hexstr[37] - '0') * 0.0001;
    y_acc = (hexstr[32] == '1') ? (-1 * y_acc) : y_acc;
    //cout << y_acc << "g" << endl;

    double z_acc = (hexstr[39] - '0') + (hexstr[40] - '0') * 0.1 + (hexstr[41] - '0') * 0.01 + (hexstr[42] - '0') * 0.001 + (hexstr[43] - '0') * 0.0001;
    z_acc = (hexstr[38] == '1') ? (-1 * z_acc) : z_acc;
    //cout << z_acc << "g" << endl;

    //x_y_z_angular_velocity
    double x_av = (hexstr[45] - '0') * 100 + (hexstr[46] - '0') * 10 + (hexstr[47] - '0') + (hexstr[48] - '0') * 0.1 + (hexstr[49] - '0') * 0.01;
    x_av = (hexstr[44] == '1') ? (-1 * x_av) : x_av;
    //cout << x_av << "du/s" << endl;

    double y_av = (hexstr[51] - '0') * 100 + (hexstr[52] - '0') * 10 + (hexstr[53] - '0') + (hexstr[54] - '0') * 0.1 + (hexstr[55] - '0') * 0.01;
    y_av = (hexstr[50] == '1') ? (-1 * y_av) : y_av;
    //cout << y_av << "du/s" << endl;

    double z_av = (hexstr[57] - '0') * 100 + (hexstr[58] - '0') * 10 + (hexstr[59] - '0') + (hexstr[60] - '0') * 0.1 + (hexstr[61] - '0') * 0.01;
    z_av = (hexstr[56] == '1') ? (-1 * z_av) : z_av;
    //cout << z_av << "du/s" << endl;

    //Quar
    double q1 = (hexstr[63] - '0') + (hexstr[64] - '0') * 0.1 + (hexstr[65] - '0') * 0.01 + (hexstr[66] - '0') * 0.001 + (hexstr[67] - '0') * 0.0001 + (hexstr[68] - '0') * 0.00001 + (hexstr[69] - '0') * 0.000001;
    q1 = (hexstr[62] == '1') ? (-1 * q1) : q1;
    //cout << q1 << " " << endl;

    double q2 = (hexstr[71] - '0') + (hexstr[72] - '0') * 0.1 + (hexstr[73] - '0') * 0.01 + (hexstr[74] - '0') * 0.001 + (hexstr[75] - '0') * 0.0001 + (hexstr[76] - '0') * 0.00001 + (hexstr[77] - '0') * 0.000001;
    q2 = (hexstr[70] == '1') ? (-1 * q2) : q2;
    //cout << q2 << " " << endl;

    double q3 = (hexstr[79] - '0') + (hexstr[80] - '0') * 0.1 + (hexstr[81] - '0') * 0.01 + (hexstr[82] - '0') * 0.001 + (hexstr[83] - '0') * 0.0001 + (hexstr[84] - '0') * 0.00001 + (hexstr[85] - '0') * 0.000001;
    q3 = (hexstr[78] == '1') ? (-1 * q3) : q3;
    //cout << q3 << " " << endl;

    double q4 = (hexstr[87] - '0') + (hexstr[88] - '0') * 0.1 + (hexstr[89] - '0') * 0.01 + (hexstr[90] - '0') * 0.001 + (hexstr[91] - '0') * 0.0001 + (hexstr[92] - '0') * 0.00001 + (hexstr[93] - '0') * 0.000001;
    q4 = (hexstr[86] == '1') ? (-1 * q4) : q4;
    //cout << q4 << " " << endl;
    if(vg_data)
    {
        vg_data->yaw = yaw;
        vg_data->pitch = pitch;
        vg_data->roll = roll;

        vg_data->x_acc = x_acc;
        vg_data->y_acc = y_acc;
        vg_data->z_acc = z_acc;

        vg_data->x_av = x_av;
        vg_data->y_av = y_av;
        vg_data->z_av = z_av;

        vg_data->q1 = q1;
        vg_data->q2 = q2;
        vg_data->q3 = q3;
        vg_data->q4 = q4;
        printf("\n-----\n");
        printf("Yaw : %f\n", yaw);
        printf("Pitch : %f\n", pitch);
        printf("Roll : %f\n", roll);

        printf("Acc x : %f\n", x_acc);
        printf("Acc y : %f\n", y_acc);
        printf("Acc z : %f\n", z_acc);

        printf("Av x : %f\n", x_av);
        printf("Av y : %f\n", y_av);
        printf("Av z : %f\n", z_av);

        printf("Q1 : %f\n", q1);
        printf("Q2 : %f\n", q2);
        printf("Q3 : %f\n", q3);
        printf("Q4 : %f\n", q4);
        printf("-----\n");
    }

    return 0;
}
