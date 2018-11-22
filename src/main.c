#include "lhog.h"

char *conf_path = NULL;

static int command_line_handle(int argc, char **argv)
{
	char ch;
	int ret;
	if(argc < 2){
		return -1;
	}
	while((ch=getopt(argc,argv,"c:")) != -1){
		switch(ch){
		case 'c':
			conf_path = optarg;
			LOGD("configure file is %s \r\n",conf_path);
			ret = 0;
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
	//test_local_cmd();
	uloop_init();
	signal(SIGPIPE, SIG_IGN);
	ret = command_line_handle(argc,argv);
	if(ret < 0 ){
		usage(argv[0]);
	}
	set_log_debug(true);
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