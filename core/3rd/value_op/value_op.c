#include <stdio.h>
#include "value_op.h"

// 移位实现整数除法
int integerDivision(int y,int x)
{
    int sum=0;
    int i=0;
    while( y > x) //向左移位直到 x >= y
    {
        x = x << 1;
        i++;      //记录左移的次数
    }

    x = x>> 1;    // 使 x <= y
    i--;

    while(i >= 0)
    {
        if(y >= x)
        {
            sum += 1 << i;
            y -= x;
        }
        x = x >> 1;
        --i;
    }

    return sum;
}

// 求 bits 中 1的个数
unsigned int countBitsInBytes(unsigned int a)
{
    a = ((a & 0xAAAA) >>1) + (a & 0x5555);
    a = ((a & 0xCCCC) >>2) + (a & 0x3333);
    a = ((a & 0xF0F0) >>4) + (a & 0x0F0F);
    a = ((a & 0xFF00) >>8) + (a & 0x00FF);
    return a;
}

// 获取连续的bit(截取变量的部分连续位)
// 另外一种实现
/*
unsigned int getBits(unsigned int bit, int end, int start)
{
    unsigned int b;
    int i,bi;

    for(i = 0, b = 0, bi = 0; i < ((sizeof(bit))<< 3); i++)
    {
        if(i <= end && i >= start)
        {
            b |= ((bit >> i) & 1) << bi;
            bi++;
        }
    }

    return b;
}
*/



#if 0
int main(int argc, char *argv[])
{
    unsigned char  x = 0x11;
    unsigned short a = 34520;

    printf("\n位操作\n");
    printf("x = 0x%02x\n", x);
    x = dislodgeLastSetBit(x);
    printf("dislodgeLastSetBit(x) : 0x%02x\n", x);
    if(isOdd(x))
    {
        printf("isOdd(%d) Yes\n", x);
    }
    x = clrBit(x, 5);
    printf("clrBit(x, 5) : 0x%02x\n", x);
    x = setBit(x, 2);
    printf("setBit(x, 2) : 0x%02x\n", x);
    printf("getBit(x, 2) : %d\n", getBit(x, 2));
    x = setByte(x, 0xf0);
    printf("setByte(x, 0xf0) : 0x%02x\n", x);

    printf("isByteSet(x, 0xf0) : %d\n", isByteSet(x, 0xf0));
    printf("isByteSet(x, 0x0f) : %d\n", isByteSet(x, 0x0f));

    x = clrByte(x, 0xf0);
    printf("clrByte(x, 0xf0) : 0x%02x\n", x);

    x = setAllBit(x);
    printf("setAllBit(x) = 0x%x\n", x);
    printf("countBitsInBytes(0xff) : %d\n", countBitsInBytes(0xff));


    printf("\n整数运算优化\n");
    // 优化 求平均数
    printf("integerAverage(1,35) : %d\n", integerAverage(1,35));
    // 优化 取模
    printf("mod : 20 %% 16 = %d, modForTwoPow(20,16) = %d\n",20%16, modForTwoPow(20,16) );
    // 优化整数除法
    printf(" 90 / 5 = %d \n", integerDivision(90,5));
    printf(" 89 / 5 = %d \n", integerDivision(89,5));

    printf("\n拆分/组合字节\n");
    printf("getBits(0x11223344, 31, 24): [0x%02x]\n",  getBits(0x11223344, 31, 24));
    printf("getBits(0x11223344, 23, 16): [0x%02x]\n",  getBits(0x11223344, 23, 16));
    printf("getBits(0x11223344, 15,  8): [0x%02x]\n",  getBits(0x11223344, 15,  8));
    printf("getBits(0x11223344,  7,  0): [0x%02x]\n",  getBits(0x11223344,  7,  0));

    printf("word32From4Bytes(0xaa, 0xbb, 0xcc, 0xdd) : [0x%08x]\n", word32From4Bytes(0xaa, 0xbb, 0xcc, 0xdd));
    printf("halfWord16From2Bytes(0xaa, 0xbb) : [0x%04x]\n", halfWord16From2Bytes(0xaa, 0xbb));

    printf("\n端序处理\n");
    printf("halfWord16FlipEndian(0xaabb) : [0x%04x]\n", halfWord16FlipEndian(0xaabb));
    printf("Word32FlipEndian(0xaabbccdd) : [0x%04x]\n", Word32FlipEndian(0xaabbccdd));

    return 0;
}
#endif

