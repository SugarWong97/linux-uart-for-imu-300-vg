#include <string.h>
#include "uart.h"
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define app_dbgntf
#ifdef app_dbgntf
#define app_dbg(mesg, args...) fprintf(stderr, "DEBUG [%s:%d:] " mesg " ", __FILE__, __LINE__, ##args)
#define depri(mesg, args...) fprintf(stderr, "" mesg " ", ##args)
#else
#define app_dbg(mesg, args...)
#endif


int serial_open(char* ttyX)
{
    int fd;

    // 注意，有些时候串口收发有问题，可能是open的选项有问题，试试添加或者删除O_NDELAY试试。
    //  O_RDWR   ：可读可写
    //  O_NOCTTY ：该参数不会使打开的文件成为该进程的控制终端。如果没有指定这个标志，那么任何一个输入都将会影响用户的进程。
    //  O_NDELAY ：这个程序不关心DCD信号线所处的状态,端口的另一端是否激活或者停止。如果用户不指定了这个标志，则进程将会一直处在睡眠状态，直到DCD信号线被激活。
#if 1
    fd = open(ttyX, O_RDWR | O_NOCTTY | O_NDELAY);
    printf("Warning: 'open' with 'O_RDWR | O_NOCTTY | O_NDELAY'\n");
#else
    fd = open(ttyX, O_RDWR | O_NOCTTY);
    printf("Warning: 'open' with 'O_RDWR | O_NOCTTY'\n");
#endif
    printf("Warning: check the flag of 'open' when UART has not reaction, try a/d 'O_NDELAY'\n");

    if (-1 == fd)
    {
        perror("serial Can't Open Serial");
        return -1 ;
    }

    //恢复串口为阻塞状态
    if(fcntl(fd, F_SETFL, 0) < 0)
    {
        app_dbg("fcntl failed!\n");
        goto err;
    }

#if 0
    //测试是否为终端设备(对读写没有影响)
    if(0 == isatty(STDIN_FILENO))
    {
        app_dbg("standard input is not a terminal device\n");
    }
#endif

    return fd;
err:
    serial_close(fd);
    return -1;
}

void serial_close(int fd)
{
    if(fd > 0)
    {
        close(fd);
    }
}

int serial_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int   i;
    int   speed_arr[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int   name_arr[] = {   115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300};
    struct termios options;

    if( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial");
        return(-1);
    }
    app_dbg("Using speed %d\n", speed);

    //set buater rate
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
            app_dbg("Seting speed %d\n", speed);
        }
    }

    //set control model
    options.c_cflag |= CLOCAL;  //清bit位 关闭流控字符 0x11 0x13
    options.c_cflag |= CREAD;  //清bit位 关闭流控字符 0x11 0x13
    options.c_iflag &= ~(INLCR|ICRNL);//清bit位 关闭字符映射 0x0a 0x0d
    options.c_iflag &= ~(IXON);//清bit位 关闭流控字符 0x11 0x13

    //set flow control
    switch(flow_ctrl)
    {
        case 0 ://none
            options.c_cflag &= ~CRTSCTS;

            break;

        case 1 ://use hard ware
            options.c_cflag |= CRTSCTS;

            break;

        case 2 ://use sofware
            options.c_cflag |= IXON | IXOFF | IXANY;

            break;
    }

    //select data bit
    options.c_cflag &= ~CSIZE;

    switch (databits)
    {
        case 5    :
            options.c_cflag |= CS5;

            break;

        case 6    :
            options.c_cflag |= CS6;

            break;

        case 7    :
            options.c_cflag |= CS7;

            break;

        case 8:
            options.c_cflag |= CS8;

            break;

        default:
            fprintf(stderr,"Unsupported data size\n");
            return (-1);
    }

    //select parity bit
    switch (parity)
    {
        case 'n':
        case 'N'://无奇偶校验位
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;

            break;

        case 'o':
        case 'O'://设置为奇校验
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;

            break;

        case 'e':
        case 'E'://设置为偶校验
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;

            break;

        case 's':
        case 'S'://设置为空格
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;

            break;

        default:
            fprintf(stderr,"Unsupported parity\n");
            return (-1);
    }

    // set stopbit
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB; break;

        case 2:
            options.c_cflag |= CSTOPB; break;

        default:
            fprintf(stderr,"Unsupported stop bits\n");
            return (-1);
    }

    //修改输出模式，原始数据输出
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    //options.c_lflag &= ~(ISIG | ICANON);

    //set wait time   主要影响read函数

    /* options.c_cc[VTIME] = X;  　//设置从获取到1个字节后开始计时的超时时间
       options.c_cc[VMIN] = Y;   //设置要求等待的最小字节数

       在原始模式下对read()函数的影响：

       1、X=0，Y!=0。函数read()只有在读取了Y个字节的数据或者收到一个信号的时候才返回；
       2、X!=0，Y=0。即使没有数据可以读取，read()函数等待X时间量后返回；
       3、X!=0,Y!=0。第一个字节数据到时开始，最先满足收到Y个字节或达超时时间X任意一个条件，read()返回；
       4、X=0,Y=0。即使读取不到任何数据，函数read也会立即返回。 */

    options.c_cc[VTIME] = 1;    /* 读取一个字符等待1*(1/10)s */

    options.c_cc[VMIN] = 48;    /* 读取字符的最少个数为xx，单位是字节 */

    tcflush(fd,TCIFLUSH);

    // //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("com set error!\n");
        return -1;
    }

    return 0;
}

int serial_send(int fd, unsigned char *send_buf,int data_len)
{
    int len = 0;
    int i;

    len = write(fd,send_buf,data_len);

    if (len == data_len )
    {
        app_dbg("serial send Data len = %d\n",len);
        for(i = 0; i< len; i++)
        {
            depri("0x%02x ", send_buf[i]);
        }
        depri("\n");

        return len;
    }
    else
    {
        tcflush(fd, TCOFLUSH);
        return -1;
    }
}

int serial_recv(int fd, unsigned char *rcv_buf,int data_len, int timeout_s, int timeout_us)
{
    int len, fs_sel;
    int i;
    fd_set fs_read;
    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    if(timeout_s < 0)
    {
        time.tv_sec = 0;
    }else
    {
        time.tv_sec = timeout_s;
    }

    if(timeout_us < 0)
    {
        time.tv_usec = 0;
    } else
    {
        time.tv_usec = timeout_us;
    }

    // len = read(fd,rcv_buf,data_len);
    // select fdset

    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);

    if(fs_sel)
    {
        len = read(fd, rcv_buf,data_len);

        app_dbg("serial receive data len = %d, fs_sel = %d\n",len,fs_sel);
        for(i = 0; i < len; i++)
        {
            depri("0x%02x ", rcv_buf[i]);
        }
        depri("\n");

        return len;
    }
    else
    {
        //app_dbg("serial haven't data receive!\n");
        return -1;
    }
}
