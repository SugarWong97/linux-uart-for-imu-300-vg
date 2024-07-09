#ifndef MINILOG_H_
#define MINILOG_H_

#include <string>
#include <fstream>
#include <cassert>
#include <ctime>
#include <iostream>
using std::ios;
using std::cout;
using std::endl;

class MiniLog{
private:
	std::ofstream m_of;
	int m_minlevel;
    bool m_toScreen;
    bool m_toFile;
    unsigned int m_printCnt;
    unsigned int m_MaxLine;
    inline void args_print(void)
    { /*stop recursion*/
        if(m_toScreen)
        {
            std::cout  <<   std::endl;
        }

        if(m_toFile)
        {
            assert(this->m_of.is_open() && "Logfile write failed.");
            this->m_of  <<   std::endl;
        }
        if(f_actionAfter) f_actionAfter(m_actionArg);
    }
    /* 使用可变参数模板实现参数打印到输出流(ostream) (递归调用) */
    template<typename T,typename ...Args>
    inline void args_print(T first,Args...rest){
        if(m_toScreen)
        {
            std::cout  << " " <<  first;
        }

        if(m_toFile)
        {
            assert(this->m_of.is_open() && "Logfile write failed.");
            this->m_of << " " <<  first;
        }
        args_print(rest...); //剩余参数递归调用
    }
    void (*f_actionBefore)(void*);
    void (*f_actionAfter)(void*);
    void *m_actionArg;

public:
	enum Type {
		ADD = 0,
		OVER
	};
	enum Level {
        DEBUG = 0,
        INFO,
        WARNING,
        ERR
	};
	MiniLog(const int level) : m_minlevel(level) {
        m_toScreen = false;
        m_toFile = false;
        
        f_actionBefore = nullptr;
        f_actionAfter = nullptr;
        m_actionArg = nullptr;

		assert((MiniLog::ERR == level || MiniLog::INFO == level || MiniLog::DEBUG == level) && "Logfile create failed, please check the level(MiniLog::ERR or MiniLog::INFO or MiniLog::DEBUG.");
		return;
	}
	~MiniLog(){
		if (this->m_of.is_open()) {
			this->m_of.close();
		}
		return;
	}
    void setPrintToScreen(bool enable)
    {
        m_toScreen = enable;
    }
    void  setPrintToFile(bool enable, const std::string &logfile = "", unsigned int maxLine = 500, const int type = MiniLog::OVER)
    {
        m_toFile = enable;
        m_MaxLine = maxLine;
        m_printCnt = 0;
		if (this->m_of.is_open()) {
			this->m_of.close();
		}
        if(!m_toFile) return;
		if (type == this->ADD) {
			this->m_of.open(logfile.c_str(),std::ios_base::out|std::ios_base::app);
		} else if (type == this->OVER) {
			this->m_of.open(logfile.c_str(),std::ios_base::out|std::ios_base::trunc);
		} else {
			assert(0 && "Logfile create failed, please check the type(MiniLog::OVER or MiniLog::ADD).");
		}
		assert(this->m_of.is_open() && "Logfile create failed, please check the logfile's name and path.");
    }
    void setActionArg(void *arg)
    {
        m_actionArg = arg;
    }
    void setActionBefore(void (*func)(void*))
    {
        f_actionBefore = func;
    }
    void setActionAfter(void (*func)(void*))
    {
        f_actionAfter = func;
    }
    void setActions(void (*actionBefore)(void*), void (*actionAfter)(void*), void *actionArg)
    {
        setActionArg(actionArg);
        setActionBefore(actionBefore);
        setActionAfter(actionAfter);
    }
    template<typename T,typename ...Args>
	void W(const int level, T &first,Args...rest) {
		if (this->m_minlevel <= level)
		{
			time_t sectime = time(NULL);
            std::string strLevel;
			tm tmtime;
#ifdef _WIN32
#if _MSC_VER<1600
			tmtime = *localtime(&sectime);
#else
			localtime_s(&tmtime, &sectime);
#endif
#else
			localtime_r(&sectime, &tmtime);
#endif
            switch(level)
            {
                case MiniLog::ERR:     strLevel = "ERROR"; break;
                case MiniLog::WARNING: strLevel = "WARN "; break;
                case MiniLog::INFO:    strLevel = "INFO "; break;
                case MiniLog::DEBUG:   strLevel = "DEBUG"; break;
                default:
				    assert(0 && "Log write failed, please check the level(MiniLog::ERR or MiniLog::INFO or MiniLog::DEBUG.");
                    break;
            }
            if(f_actionBefore) f_actionBefore(m_actionArg);

            if(m_toScreen)
            {
                std::cout << tmtime.tm_year + 1900 << '-' << tmtime.tm_mon + 1 << '-' << tmtime.tm_mday << ' '
                          << tmtime.tm_hour << ':' << tmtime.tm_min << ':' << tmtime.tm_sec
                          << " [" << strLevel << "]: "  << first;
            }

            if(m_toFile)
            {
		        assert(this->m_of.is_open() && "Logfile write failed.");
                this->m_of << tmtime.tm_year + 1900 << '-' << tmtime.tm_mon + 1 << '-' << tmtime.tm_mday << ' '
                           << tmtime.tm_hour << ':' << tmtime.tm_min << ':' << tmtime.tm_sec
                           << " [" << strLevel << "]: " << first;
                /* RollBack */
                m_printCnt ++;
                if(m_printCnt >= m_MaxLine)
                {
                    this->m_of.seekp(0, ios::beg);
                    m_printCnt = 0;
                }
            }
            args_print(rest...);//剩余参数递归调用
		}
		return;
	}
};
#endif // MINILOG_H_ 

