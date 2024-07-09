#ifndef _UART_H_
#define _UART_H_


/*
 *描述  : 打开串口
 *参数  : 串口设备名 串口设备举例: /dev/ttyAMA1 /dev/ttyAMA2
 *返回值: 成功返回fd，失败返回-1
 *注意  ：无
*/
int serial_open(char* ttyX);

/*
 *描述  : 关闭串口
 *参数  : fd文件描述符
 *返回值: 成功返回fd，失败返回-1
 *注意  ：无
*/
void serial_close(int fd);

/*
 *描述  : 串口参数设置
 *参数  : fd: 文件描述符
 *        speed: 波特率.115200,19200,9600...
 *        flow_ctrl: 流控
 *        databits: 数据位 取值为5,6,7,8
 *        stopbits: 停止位 取值为1,2
 *        parity: 奇偶校验位  取值为N,W,O,S
 *返回值: 成功返回0，失败返回-1
 *注意  ：无
*/
int serial_set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);

/*
 *描述  : 串口发送
 *参数  : fd:文件描述符
 *        send_buf:发送buf
 *        data_len:数据长度
 *返回值: 成功返回数据长度，失败返回-1
 *注意  ：无
*/
int serial_send(int fd, unsigned char *send_buf,int data_len);

/*
 *描述  : 串口接收
 *参数  : fd:文件描述符
 *        send_buf:接收buf
 *        data_len:数据长度
 *        timeout_s:超时时间s
 *        timeout_us:超时时间us
 *返回值: 成功返回数据长度，失败返回-1
 *注意  ：无
*/
int serial_recv(int fd, unsigned char *rcv_buf,int data_len, int timeout_s, int timeout_us);

#endif /* _UART_H_ */

