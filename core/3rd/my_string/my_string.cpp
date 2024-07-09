#include <vector>
#include <string>

using std::vector;
using std::string;
#include "my_string.hxx"

#ifdef __cplusplus
extern "C"{
#endif
// C code...
#include <string.h>

#ifdef __cplusplus
}
#endif


// 比较2串字符串的长度，返回其中大的那一串长度
static inline int MaxForString(const string &a, const string &b)
{
    if(a.length() > b.length())
        return a.length();
    return b.length();
}

// 比较2串字符串的长度，返回其中小的那一串长度
static inline int MinForString(const string &a, const string &b)
{
    if(a.length() < b.length())
        return a.length();
    return b.length();
}


// 分割字符串到vector容器中
// str : 源字符串
// split : 用于分割的字符串
// res : 用于接收结果的vector容器
void StringSplitToVector(const string& str, const string& split, vector<string>& res)
{
    char* strc = new char[str.size() + 1];
    strcpy(strc, str.c_str());   // 将str拷贝到 char类型的strc中
    char* temp = strtok(strc, split.c_str());
    while (temp != NULL)
    {
        res.push_back(string(temp));
        temp = strtok(NULL, split.c_str()); // 下一个被分割的串
    }
    delete[] strc;
}

// 将 Hex-like string的多余部分去掉
// hexStr : 需要的string
// keepSpace: 是否保留空格(默认不保留)
// 返回值： 新的hexString
string StringHexTrim(string hexStr, int keepSpace)
{
    string str(hexStr);

    // 从0xaa bb ccH ddh eeff字符串转换为 aabbccddeeff 这样子的形式
    // 仅仅保留 0~f 的字符
    str = StringReplace(str, "H", "h");
    str = StringReplace(str, "h", "");
    str = StringReplace(str, "X", "x");
    str = StringReplace(str, "0x", "");

    if(keepSpace == 0)
    {
        str = StringReplace(str, " ", "");
    }
    // 保持复数长度
    if ((str.length()) % 2 != 0)
    {
        str = "0" + str;
    }
    return str;
}

// 将 由分隔符 隔开的Hex-like 字符串文本转换为数组
// hexStr : 需要的string
// buff   : 用于接收的buff缓冲区
// buffMaxLen : buff 缓冲区最大的长度（防止写溢出）
// split  : 用于分割的字符串（“默认是空格”）
// 返回值： 参数错误返回-1；正常状态下返回正整数
int StringHexToByteArray(string hexStr, unsigned char *buff, unsigned int buffMaxLen, string split)
{
    vector<string> strList;
    string str;
    string temp;
    unsigned int tc;

    if(buff == NULL)
    {
        return -1;
    }

    // 如果分隔符是Space，那么过滤HEX的时候不能去掉Space，防止无法识别
    if(split == " ")
    {
        str = StringHexTrim(hexStr, 1);
    }else
    {
        str = StringHexTrim(hexStr, 0);
    }

    // 将子串存放到strList中
    StringSplitToVector(str, split, strList);
    //for (auto s : strList)
    for (long unsigned int i = 0; i < strList.size(); i++)
    {
        if(i >= buffMaxLen)
        {
            return buffMaxLen;
        }
        temp = strList[i];
        sscanf(temp.c_str(), "%x", &tc);
        tc = tc & 0xff;
        buff[i] = (unsigned char )tc;
    }

    return strList.size();
}

// 将 由分隔符 隔开的Hex-like 字符串文本转换为数组
// hexStr : 需要的string
// buff   : 用于接收的buff缓冲区
// buffMaxLen : buff 缓冲区最大的长度（防止写溢出）
// 返回值： 参数错误返回-1；正常状态下返回正整数
int StringHexToByteArray_c(const char *hexStr, unsigned char *buff, unsigned int buffMaxLen)
{
    char *p = (char *)hexStr;
    char high = 0, low = 0;
    unsigned int tmplen = strlen(p), cnt = 0;
    tmplen = strlen(p);
    while(cnt < (tmplen / 2))
    {
        if(*p == ' ')
        {
            p++;
        }
        if(cnt >= buffMaxLen)
        {
            goto end;
        }

        high = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
        low = (*(++ p) > '9' && ((*p <= 'F') || (*p <= 'f'))) ? *(p) - 48 - 7 : *(p) - 48;
        buff[cnt] = ((high & 0x0f) << 4 | (low & 0x0f));
        p ++;
        cnt ++;
    }

    if(cnt >= buffMaxLen)
    {
        goto end;
    }

    if(tmplen % 2 != 0)
    {
        buff[cnt] = ((*p > '9') && ((*p <= 'F') || (*p <= 'f'))) ? *p - 48 - 7 : *p - 48;
    }

end:
    return tmplen / 2 + tmplen % 2;
}

/*
 函数说明：对字符串中所有指定的子串进行替换
 参数：
    srcStr  : 源字符串
    oldStr  : 被替换子串
    newStr  : 替换子串
    times   : 替换次数
         -1  全部替换
          0  不替换
         >0  指定的替换次数
返回值: 替换好以后的字符串
 */
string StringReplace(string srcStr, string oldStr, string newStr, int times)
{
    string::size_type pos = 0;
    int i = 0;
    while((pos = srcStr.find(oldStr)) != string::npos)   //替换所有指定子串
    {
        if(i == times)
        {
            break;
        }
        srcStr.replace(pos, oldStr.length(), newStr);
        i++;
    }
    return srcStr;
}


// 内部函数，用于判断字符串的头部/尾部 是否和 substr完全匹配
static int _string_tailmatch(const std::string&self, const std::string&substr, int start, int end, int direction)
{
#define ADJUST_INDICES(start, end, len)     \
    if (end > len)                          \
        end = len;                          \
    else if (end < 0) {                     \
        end += len;                         \
        if (end < 0)                        \
        end = 0;                            \
    }                                       \
    if (start < 0) {                        \
        start += len;                       \
        if (start < 0)                      \
        start = 0;                          \
    }

    int selflen = (int)self.size();
    int slen = (int)substr.size();

    const char* str = self.c_str();
    const char* sub = substr.c_str();

    //对输入的范围进行校准
    ADJUST_INDICES(start, end, selflen);

    //字符串头部匹配（即startswith）
    if (direction < 0)
    {
        if (start + slen>selflen)
        return 0;
    }
    //字符串尾部匹配（即endswith）
    else
    {
        if (end - start<slen || start>selflen)
        return 0;
        if (end - slen > start)
            start = end - slen;
    }
    if (end - start >= slen)
        //mcmcmp函数用于比较buf1与buf2的前n个字节
        return !memcmp(str + start, sub, slen);
    return 0;

}

// 判断string是否以某段字符串开头
// str : 源字符串
// prefix : 是否以这段字符串开头
// start : 起始位置(默认为0)
// end: 结束位置（默认为无限制）
bool StringStartsWith(const std::string&str, const std::string&prefix, int start, int end)
{
    //调用＿string＿tailmatch函数，参数-1表示字符串头部匹配
    int result = _string_tailmatch(str, prefix, start, end, -1);
    return static_cast<bool>(result);
}

// 判断string是否以某段字符串结尾
// str : 源字符串
// prefix : 是否以这段字符串结尾
// start : 起始位置(默认为0)
// end: 结束位置（默认为无限制）
bool StringEndsWith(const std::string&str, const std::string&suffix, int start, int end)
{
    //调用＿string＿tailmatch函数，参数+1表示字符串尾部匹配
    int result = _string_tailmatch(str, suffix, start, end, +1);
    return static_cast<bool>(result);
}
