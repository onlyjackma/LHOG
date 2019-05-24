#include "lhog.h"

char *conf_path = NULL;
int daemo = 0;
int debug = 0;

static int command_line_handle(int argc, char **argv)
{
	char ch;
	int ret;
	if(argc < 2){
		return -1;
	}
	while((ch=getopt(argc,argv,"c:bd")) != -1){
		switch(ch){
		case 'c':
			conf_path = optarg;
			LOGD("configure file is %s \r\n",conf_path);
			ret = 0;
			break;
        case 'b':
            daemo = 1;
            break;
        case 'd':
            debug = 1;
			break;
		default:
			ret = -1;
			break;
		}

	}
	return ret;
}
void usage(char *pname){
	printf("%s -c path/confg.json\n",pname);
	exit(1);
}

int main(int argc ,char *argv[])
{
	int ret;
    
	uloop_init();
	signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

	ret = command_line_handle(argc,argv);

	if(ret < 0 ){
		usage(argv[0]);
	}
    if(debug){
        set_log_debug(true);
    }else{
        set_log_debug(false);
    }
	if(daemo){
        ret = daemon(0,0);
    }
	LOGD("init config......\n");
	init_mqtt_config(conf_path);
	if(ret < 0 ){
		LOGD("parse config file failed\n");
		exit(1);
	}
	init_all_service();
	uloop_run();
	uloop_done();
	deinit_mqtt_config();
	return 0;
}