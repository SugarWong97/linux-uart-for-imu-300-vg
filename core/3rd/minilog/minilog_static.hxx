#ifndef MINILOG_STATIC_APP_H_
#define MINILOG_STATIC_APP_H_

#include <string>
using std::string;
#include "minilog.hxx"

void LogInit(int level, const string &file = "", int maxFileLine = 200);
class MiniLog *getIns(void);

template<typename T,typename ...Args>
void LogD( T first,Args...rest)
{
    if(getIns())
    getIns()->W(MiniLog::DEBUG, first, rest...);
}

template<typename T,typename ...Args>
void LogI( T first,Args...rest)
{
    if(getIns())
    getIns()->W(MiniLog::INFO, first, rest...);
}

template<typename T,typename ...Args>
void LogW( T first,Args...rest)
{
    if(getIns())
    getIns()->W(MiniLog::WARNING, first, rest...);
}

template<typename T,typename ...Args>
void LogE( T first,Args...rest)
{
    if(getIns())
    getIns()->W(MiniLog::ERR, first, rest...);
}

#endif // MINILOG_STATIC_H_ 
