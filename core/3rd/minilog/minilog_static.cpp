#include <iostream>
#include "minilog.hxx"
#include "minilog_static.hxx"

class MiniLog *g = nullptr;

class MiniLog *getIns(void)
{
    return g;
}

void LogInit(int level, const string &file, int maxFileLine)
{
    static class MiniLog log1(level);
    log1.setPrintToScreen(true);
    if(file != "")
        log1. setPrintToFile(true, file, maxFileLine);
    g = &log1;
}

