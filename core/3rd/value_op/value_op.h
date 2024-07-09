#ifndef __VAL_OP__H__
#define __VAL_OP__H__
#include <stdio.h>

// 置某位为高
#define setBit(value, bit)     ((value)  |  (1 << (bit)))
// 清某位为低
#define clrBit(value, bit)     ((value)  & ~(1 << (bit)))
// 获取某位
#define getBit(value, bit)     ((((value)>>(bit)) &  0x01)==1)
// 置某几位为高
#define setByte(value, byte)   ((value)  |  (byte))
// 清某几位为低
#define clrByte(value, byte)   ((value)  & ~(byte))
// 判断某几位是否置位
#define isByteSet(value, byte)   (((value) &  (byte)) == (byte) )


// 将4个单字节组合成为32位数
//   0xaa 0xbb 0xcc 0xdd --> 0xaabbccdd
#define   word32From4Bytes(one,two,three,four) (((unsigned int)(one) << 24) | ((unsigned int)(two) << 16) | ((unsigned short)(three) << 8) | ((unsigned char)(four)))
// 将2个单字节组合成为16位数
//   0xaa 0xbb --> 0xaabb
#define   halfWord16From2Bytes(h,l)           ((((unsigned short)(h)) << 8) | (unsigned char)(l))
// 将16位数的高低位互换
//   0xaab --> 0xbbaa
#define   halfWord16FlipEndian(h) ((((unsigned short)(h) << 8)&0xFF00) | ((unsigned short)(h) >> 8))
// 将32位数的高低位互换
//   0xaabbccdd --> 0xddccbbaa
#define   Word32FlipEndian(h) ( (((unsigned int)(h) << 24)&0xFF000000) | (((unsigned int)(h) << 8)&0x00FF0000) | (((unsigned int)(h) >> 8)&0x0000FF00) | (((unsigned int)(h) >> 24)&0x000000FF) )

/*
 * 由于处理位的时候不知道是哪种数据类型，所以用宏会比用 inline 函数好
 *
 * */

//消去x最后一位的1  :   x&(x-1)
#define dislodgeLastSetBit(x) ((x)&((x) - 1))

/*  用途
 *可以用来检测一个数是不是2的幂次。如果一个数x是2的幂次，那么x>0且x的二进制中只有一个1,所以用x&(x-1)把1消去，应该返回0，如果返回了非0值，证明不是2的幂次

计算一个整数二进制中1的个数
因为1可以不断的通过x&(x-1)这个操作消去，所以当最后的值变成0的时候，也就求出了二进制中1的个数

如果将整数A转换成整数B,需要改变多少个比特位.
思考将整数A转换为B，如果A和B在第i（0<=i<32）个位上相等，则不需要改变这个BIT位，如果在第i位上不相等，则需要改变这个BIT位。所以问题转化为了A和B有多少个BIT位不相同。联想到位运算有一个异或操作，相同为0，相异为1，所以问题转变成了计算A异或B之后这个数中1的个数
 * */

// 判断奇偶
#define isOdd(x) ((x)&1)

// 求平均数（只适用于整数）
#define integerAverage(x, y) ((x)&(y))+(((x)^(y))>>1)

// 将所有的位 置一
#define setAllBit(x) ((x)+(~x))

// 求绝对值
#define ABS_INT64(x)        ((x) ^ ((x) >> 63)) - ((x) >> 63)
#define ABS_INT32(x)        ((x) ^ ((x) >> 31)) - ((x) >> 31)
#define ABS_INT16(x)        ((x) ^ ((x) >> 15)) - ((x) >> 15)
#define ABS_INT8(x)         ((x) ^ ((x) >>  7)) - ((x) >>  7)


// 优化： 取模操作优化（仅适用于除数是2的幂次方）
#define modForTwoPow(a,b) (a)&((b)-1)

// 移位实现整数除法
int integerDivision(int y,int x);

// 求 bits 中 1的个数
unsigned int countBitsInBytes(unsigned int a);

// 获取连续的bit(截取变量的部分连续位)
#define   getBits(x, end, start)   ( (x & ~(~(0U)<<((end)-(start) + 1))<<(start)) >> (start) )

/*
// 向上整数倍取整
#ifndef ROUNDUP
//得到1066向上取整100的倍数，最后结果为1100。
//ROUNDUP(1066, 100) -- > 1100
#define ROUNDUP(x, y) (                 \
{                           \
    typeof(y) __y = y;              \
    (((x) + (__y - 1)) / __y) * __y;        \
}                           \
)
#endif
*/
#ifndef ROUNDUP
// 向上整数倍取整
//得到1066向上取整100的倍数，最后结果为1100。
#define ROUNDUP iROUNDUP
#endif
static inline int iROUNDUP(int ori_value, int mult_value)
{
    int __y = mult_value;
    return (((ori_value) + (__y - 1)) / __y) * __y;
}

#endif/*__VAL_OP__H__*/

