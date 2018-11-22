#ifndef LHOG_H
#define LHOG_H
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <getopt.h>

#include <libubox/ustream.h>
#include <libubox/uloop.h>
#include <libubox/usock.h>
#include <libubox/blob.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>
#include <sys/mman.h>
#include <mosquitto.h>
#include <cjson/cJSON.h>
#include <sys/syslog.h>

/*
* this for service_util.c define
*/
typedef int (*service_init)(void);

struct service{
	struct list_head head;
	char *name;
	service_init init;

};

void register_service(char * name ,service_init init);

#define REG_SERVICE(name,init) \
static void __attribute__((constructor)) __reg_serive_##name() \
{ \
	register_service(#name,init); \
}

int init_all_service();

/*
* This is for mqtt_service.c  define
*/

#define MAX_MQTT_MESSAGE_SIZE 256*1024*1024
struct mqtt_msg{
	char *topic;
	char *payload;
	int msg_len;
	struct list_head list;
};

struct mqtt_context {
    struct mosquitto *mosq;
    struct list_head msgs;
    pthread_mutex_t mutex;
    int connected;
};

struct mqtt_msg *init_mqtt_msg(char *topic,char *payload,int len);
void free_mqtt_msg(struct mqtt_msg *msg);

enum {
	MQTT_QOS_0,
	MQTT_QOS_1,
	MQTT_QOS_2,
};
void mqtt_pub_msg(const char *msg, int  msg_len);
int mqtt_pub_file(const char *file);
int mqtt_tpub_msg(const char *topic, const char *msg, int  msg_len);
int mqtt_tpub_file(const char *topic, const char *file);
int start_mqtt_worker();

/*
* This is for config.c
*/
struct mqtt_config{
	char *dev_mac;
	char *server_host;
	short server_port;
	int keep_alive;
	char *mqtt_name;
	char *mqtt_passwd;
    char *mqtt_pub_topic;
    char *mqtt_sub_topic;
};

#define MAX_CONF_SIZE 4096


struct mqtt_config *get_mqtt_conf();
int init_mqtt_config(char *conf_path);
int deinit_mqtt_config();
void dispatch_proto_data(void);

/*
* This if for debug.c
*/

void iot_debug(int level, char *fmt, ...);
void set_log_debug(bool value);
#define LOGI(...) iot_debug(LOG_INFO,__VA_ARGS__)
#define LOGD(...) iot_debug(LOG_DEBUG,__VA_ARGS__)
#define LOGW(...) iot_debug(LOG_WARNING,__VA_ARGS__)
#define LOGE(...) iot_debug(LOG_ERR,__VA_ARGS__)

/*
* This is for run_script.c
*/
int do_shell_script(char *script ,int len);

/*
* This is for ubus_service.c
*/
typedef int (*ubus_cmd_handler)(char *cmd ,char *para ,struct blob_buf *b);

struct ubus_cmd_handler{
	struct list_head head;
	char *cmd;
	ubus_cmd_handler handler;
};

void register_ubus_handler(char *cmd , ubus_cmd_handler handler);

#define REG_UBUS_CMD_HANDLER(cmd,handler) \
static void __attribute__((constructor))__reg_handler_##cmd() \
{\
	register_ubus_handler(#cmd,handler); \
}

#endif