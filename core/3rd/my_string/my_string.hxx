#include <vector>
#include <string>
using std::vector;
using std::string;

// 比较2串字符串的长度，返回其中大的那一串长度
static inline int MaxForString(const string &a, const string &b);
// 比较2串字符串的长度，返回其中小的那一串长度
static inline int MinForString(const string &a, const string &b);

// 分割字符串到vector容器中
// str : 源字符串
// split : 用于分割的字符串
// res : 用于接收结果的vector容器
void StringSplitToVector(const string& str, const string& split, vector<string>& res);

// 将 Hex-like string的多余部分去掉
// hexStr : 需要的string
// keepSpace: 是否保留空格(默认不保留)
// 返回值： 新的hexString
string StringHexTrim(string hexStr, int keepSpace = 0);

// 将 由分隔符 隔开的Hex-like 字符串文本转换为数组
// hexStr : 需要的string
// split  : 用于分割的字符串
// buff   : 用于接收的buff缓冲区
// buffMaxLen : buff 缓冲区最大的长度（防止写溢出）
// 返回值： 参数错误返回-1；正常状态下返回正整数
int StringHexToByteArray(string hexStr, unsigned char *buff, unsigned int buffMaxLen, string split = " ");
// 将 由分隔符 隔开的Hex-like 字符串文本转换为数组（分隔符为Space）
// hexStr : 需要的string
// buff   : 用于接收的buff缓冲区
// buffMaxLen : buff 缓冲区最大的长度（防止写溢出）
// 返回值： 参数错误返回-1；正常状态下返回正整数
int StringHexToByteArray_c(const char *hexStr, unsigned char *buff, unsigned int buffMaxLen);

/*
 函数说明：对字符串中所有指定的子串进行替换
 参数：
    srcStr  : 源字符串
    oldStr  : 被替换子串
    newStr  : 替换子串
    times   : 替换次数(默认为 -1)
         -1  全部替换
          0  不替换
         >0  指定的替换次数
返回值: 替换好以后的字符串
 */
string StringReplace(string srcStr, string oldStr, string newStr, int times = -1);

// 判断string是否以某段字符串开头
// str : 源字符串
// prefix : 是否以这段字符串开头
// start : 起始位置(默认为0)
// end: 结束位置（默认为无限制）
bool StringStartsWith(const std::string&str, const std::string&prefix, int start = 0, int end = -1);
// 判断string是否以某段字符串结尾
// str : 源字符串
// prefix : 是否以这段字符串结尾
// start : 起始位置(默认为0)
// end: 结束位置（默认为无限制）
bool StringEndsWith(const std::string&str, const std::string&suffix, int start = 0, int end = -1);
