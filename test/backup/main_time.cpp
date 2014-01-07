
#include "../Headdefine.h"

static const int days[4][13] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366},
};


#define DEFAULT_TIME_ZONE_OFFSET 8
#define LEAP_CHECK(n)   ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)
#define WRAP(a,b,m) ((a) = ((a) <  0  ) ? ((b)--, (a) + (m)) : (a))

long long base_pivot_time_t (const time_t * now, long long *_t)
{
    long long t;
    t = *_t;
    if (now && sizeof (time_t) == 4) {
        time_t _now;
        _now = *now;
        if (_now < 1106500000 /* Jan 23 2005 - date of writing */ )
            _now = 2147483647;
        if ((long long) t + ((long long) 1 << 31) < (long long) _now)
            t += (long long) 1 << 32;
    }
    return t;
}

static struct tm *_gmtime64_r (const time_t * now, long long *_t, struct tm *p)
{
    int v_tm_sec, v_tm_min, v_tm_hour, v_tm_mon, v_tm_wday, v_tm_tday;
    int leap;
    long long t;
    long m;
    t = base_pivot_time_t (now, _t);
    v_tm_sec = ((long long) t % (long long) 60);
    t /= 60;
    v_tm_min = ((long long) t % (long long) 60);
    t /= 60;
    v_tm_hour = ((long long) t % (long long) 24);
    t /= 24;
    v_tm_tday = t;
    WRAP (v_tm_sec, v_tm_min, 60);
    WRAP (v_tm_min, v_tm_hour, 60);
    WRAP (v_tm_hour, v_tm_tday, 24);
    if ((v_tm_wday = (v_tm_tday + 4) % 7) < 0)
        v_tm_wday += 7;
    m = (long) v_tm_tday;
    if (m >= 0) {
        p->tm_year = 70;
        leap = LEAP_CHECK (p->tm_year);
        while (m >= (long) days[leap + 2][12]) {
            m -= (long) days[leap + 2][12];
            p->tm_year++;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 0;
        while (m >= (long) days[leap][v_tm_mon]) {
            m -= (long) days[leap][v_tm_mon];
            v_tm_mon++;
        }
    } else {
        p->tm_year = 69;
        leap = LEAP_CHECK (p->tm_year);
        while (m < (long) -days[leap + 2][12]) {
            m += (long) days[leap + 2][12];
            p->tm_year--;
            leap = LEAP_CHECK (p->tm_year);
        }
        v_tm_mon = 11;
        while (m < (long) -days[leap][v_tm_mon]) {
            m += (long) days[leap][v_tm_mon];
            v_tm_mon--;
        }
        m += (long) days[leap][v_tm_mon];
    }
    p->tm_mday = (int) m + 1;
    p->tm_yday = days[leap + 2][v_tm_mon] + m;
    p->tm_sec = v_tm_sec, p->tm_min = v_tm_min, p->tm_hour = v_tm_hour,
        p->tm_mon = v_tm_mon, p->tm_wday = v_tm_wday;
    return p;
}

struct tm *base_gmtime64_r (const long long *_t, struct tm *p)
{
    long long t;
    t = *_t;
    return _gmtime64_r (NULL, &t, p);
}

struct tm *base_pivotal_gmtime_r (const time_t * now, const time_t * _t, struct tm *p)
{
    long long t;
    t = *_t;
    return _gmtime64_r (now, &t, p);
}

long long base_mktime64 (struct tm *t)
{
    int i, y;
    long day = 0;
    long long r;

    if (t->tm_year < 70) {
        y = 69;
        do {
            day -= 365 + LEAP_CHECK (y);
            y--;
        } while (y >= t->tm_year);
    } else {
        y = 70;
        while (y < t->tm_year) {
            day += 365 + LEAP_CHECK (y);
            y++;
        }
    }
    for (i = 0; i < t->tm_mon; i++)
        day += days[LEAP_CHECK (t->tm_year)][i];
    day += t->tm_mday - 1;
    t->tm_wday = (int) ((day + 4) % 7);
    r = (long long) day *86400;
    r += ( t->tm_hour - DEFAULT_TIME_ZONE_OFFSET ) * 3600;
    r += t->tm_min * 60;
    r += t->tm_sec;

    return r;
}

static struct tm *_localtime64_r (const time_t * now, long long *_t, struct tm *p)
{
    long long tl;
    time_t t;
    struct tm tm, tm_localtime, tm_gmtime;
    struct tm *ptm_localtime ,*ptm_gmtime ;
    //_gmtime64_r(now,_t,p) ;
    _gmtime64_r (now, _t, &tm);
    if (tm.tm_year > (2037 - 1900))
        tm.tm_year = 2037 - 1900;
    else
    {
        t = *_t ;
        memcpy(p,localtime(&t),sizeof(struct tm)) ;
        return p ;
    }
    t = base_mktime64 (&tm);
#ifdef WIN32
    ptm_localtime = localtime(&t) ;
    memcpy(&tm_localtime,ptm_localtime,sizeof(struct tm)) ;
    ptm_gmtime = gmtime(&t) ;
    memcpy(&tm_gmtime,ptm_gmtime,sizeof(struct tm)) ;
#else
    ptm_localtime = localtime_r (&t, &tm_localtime);
    ptm_gmtime = gmtime_r (&t, &tm_gmtime);
#endif
    tl = *_t;
    tl += (base_mktime64 (&tm_localtime) - base_mktime64 (&tm_gmtime));
    _gmtime64_r (now, &tl, p);
    //p->tm_isdst = -1 ;
    p->tm_isdst = tm_localtime.tm_isdst;
    return p;
}

struct tm *base_pivotal_localtime_r (const time_t * now, const time_t * _t, struct tm *p)
{
    long long tl;
    tl = *_t;
    return _localtime64_r (now, &tl, p);
}

struct tm *base_localtime64_r (const long long *_t, struct tm *p)
{
    long long tl;
    tl = *_t;
    return _localtime64_r (NULL, &tl, p);
}


int main()
{
    long long now1 = time(NULL);
    time_t now2 = time(NULL);

    struct tm t;
    base_localtime64_r(&now1, &t);
    printf("base_localtime64_r\t%d-%d-%d %d:%d:%d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    base_gmtime64_r(&now1, &t);
    printf("base_gmtime64_r\t%d-%d-%d %d:%d:%d\n", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    struct tm * t2 = localtime(&now2);
    printf("localtime\t%d-%d-%d %d:%d:%d\n", t2->tm_year, t2->tm_mon, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);
    t2 = gmtime(&now2);
    printf("gmtime\t%d-%d-%d %d:%d:%d\n", t2->tm_year, t2->tm_mon, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec);


}
