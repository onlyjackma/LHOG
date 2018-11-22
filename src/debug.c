#include "lhog.h"

static bool log_enable = false;
static bool debug = true;

static void log_LOGD_output(int level,char *buf)
{
    switch (level) {
    case LOG_DEBUG:
        printf("[DEBUG]:");
        break;
    case LOG_WARNING:
        printf("[WARNING]:");
        break;
    case LOG_ERR:
        printf("[ERROR]:");
        break;
    }
    printf("%s\n", buf);

}

static void log_syslog_ouput(int level ,char *buf)
{
	syslog(level,"%s",buf);
}

void iot_debug(int level, char *fmt, ...)
{
	va_list ap;
	char buf[8192] = {0};

	if(!debug && level == LOG_DEBUG){
		return;
	}
	va_start(ap,fmt);
	vsnprintf(buf,sizeof(buf),fmt,ap);
	va_end(ap);
	if(log_enable){
		log_syslog_ouput(level,buf);
	}else{
		log_LOGD_output(level,buf);
	}
}

void set_log_debug(bool value)
{
	debug = value;
}