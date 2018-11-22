#include "lhog.h"
#define CMD_LINE_LEN 256

int do_shell_script(char *script ,int len)
{
    char *cmd = "/bin/sh %s &";
    char *pub_cmd= " >/tmp/.RESULT;ubus call lhog do_cmd \'{\"cmd\":\"rcmd_report\",\"para\":\"/tmp/.RESULT\"}\' >/dev/null ;rm $0";
    int  fd;
    char fn[64];
    char cmdline[CMD_LINE_LEN] = {0};
    int ret;

    /* 1. Create temp file */
    sprintf(fn, "/tmp/.exec_XXXXXX");
    fd=mkstemp(fn);
    if (fd<0){
        LOGE("Failed to open temp file\n");
        return -1;
    }
    if (write(fd, script, len)<len){
        close(fd);  
       LOGE("Failed to write temp file\n");
        unlink(fn);
        return -2;
    }
    ret = write(fd,pub_cmd,strlen(pub_cmd));
    close(fd);
    /* 2. Run */
    sprintf(cmdline,cmd, fn);
    ret = system(cmdline);
    return ret;

}