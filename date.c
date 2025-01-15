#include <stdio.h>
#include <sys/time.h>
#include <time.h>

void get_current_date_time(void)
{
    struct timeval tv;
    struct timezone tz;
    struct tm *t;

    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    printf("%04d-%02d-%02d %02d:%02d:%02d.%06ld\n", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);
}
