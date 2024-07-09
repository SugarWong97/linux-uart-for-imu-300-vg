#ifndef __IMU300_VG_H__
#define __IMU300_VG_H__

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

/**/#define CMD_SEND_READ_ALL_DATA          0x59   // 2.14 同时读取角度，加速度计，陀螺仪，四元数（MINS VG 系列）
/**/#define CMD_RECV_READ_ALL_DATA          0x59   // 2.14 同时读取角度，加速度计，陀螺仪，四元数

/**/#define CMD_SEND_GYROSCOPE_CALIB        0x52   // 2.8 陀螺仪校准 发送命令
/**/#define CMD_RECV_GYROSCOPE_CALIB        0xa5   // 2.8 陀螺仪校准 回复命令

/**/#define CMD_SEND_SAVE_CONFIG            0x0a   // 2.9 陀螺仪校准 发送命令
/**/#define CMD_RECV_SAVE_CONFIG            0x8a   // 2.9 陀螺仪校准 回复命令

// 数据域：根据命令字不同内容和长度相应变化；
#define PACKAGE_INDEX_4_DATA   0x04
/**/#define RECV_DATA_VALUE_OK   0x00
/**/#define RECV_DATA_VALUE_ERR   0xff

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
    AUTO_OUTPUT_TYPE_00 = 0, // 三轴角度
    AUTO_OUTPUT_TYPE_01 = 1, // 三轴加速度
    AUTO_OUTPUT_TYPE_02 = 2, // 三轴角速度
    AUTO_OUTPUT_TYPE_03 = 3, // 三轴角度
    AUTO_OUTPUT_TYPE_04 = 4, // 四元数
    AUTO_OUTPUT_TYPE_05 = 5, // 角度、加速度、角速度、四元数
    AUTO_OUTPUT_TYPE_BUTT,
};


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
    OUTPUT_MODE_0HZ_CS = 0,
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



struct imu300_vg_data
{
    //rpy
    double yaw;   // 航向
    double pitch; // 俯仰角
    double roll;  // 翻滚角

    //x_y_z_acceleratio
    double x_acc;
    double y_acc;
    double z_acc;

    //x_y_z_angular_velocity
    double x_av;
    double y_av;
    double z_av;

    //Quar
    double q1;
    double q2;
    double q3;
    double q4;
};

int imx300_vg_uart_probe(char * uart_dev_path, int speed);
int imx300_vg_read_all_datas(int uart_fd, struct imu300_vg_data *p_vg_data);
//int imx300_vg_get_roll(int uart_fd);
int imx300_vg_cal_recv_data(unsigned char * buffer, struct imu300_vg_data *vg_data);
int imx300_vg_do_gyroscope_calib(int uart_fd);
int imx300_vg_set_output_mode(int uart_fd, enum output_mode mode);
int imx300_vg_set_auto_output_type(int uart_fd, enum auto_output_type mode);
#endif/*__IMU300_VG_H__*/
