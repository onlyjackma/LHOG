#include "lhog.h"

struct mosquitto *mosq = NULL;
struct mqtt_config *conf;
static bool mqtt_connected;
struct mqtt_context *mqtt_ctx = NULL;

struct mqtt_msg *init_mqtt_msg(char *topic,char *payload,int len)
{
	struct mqtt_msg *mq = calloc(1,sizeof(struct mqtt_msg));;

	if(mq == NULL || topic == NULL ||payload == NULL )
		return NULL;

	mq->topic = strdup(topic);
	mq->payload = strdup(payload);
	if(len == 0){
		len = strlen(payload);
	}
	mq->msg_len = len;

	return mq;
}

void dispatch_proto_data(void)
{
	struct mqtt_msg *msg = NULL;
	struct mqtt_msg *n = NULL;
	if(mqtt_ctx != NULL){
		list_for_each_entry_safe(msg,n,&mqtt_ctx->msgs,list){
			do_shell_script(msg->payload,msg->msg_len);
			free(msg->payload);
			free(msg->topic);
			list_del(&msg->list);
		}
	}

}

void free_mqtt_msg(struct mqtt_msg *msg)
{
	msg->msg_len = 0;
	if(msg->topic)
		free(msg->topic);
	if(msg->payload)
		free(msg->payload);
	free(msg);
}

static bool is_mqtt_alive(){
	return mqtt_connected;
}


static int _mqtt_pub_msg(const char *topic ,const char *msg, int  msg_len)
{
	int ret;
	if(!is_mqtt_alive()){
		LOGD("mqtt is offline !\n");
	}

	if(!topic || !msg || !msg_len){
		LOGD("Please check your parameters\n");
	}

	ret = mosquitto_publish(mqtt_ctx->mosq, NULL, topic, msg_len, msg,MQTT_QOS_1,false);
	if(ret<0){
		LOGD("mosquitto_publish failed! cached\n");
	}
	if(ret == 0){
		LOGD("msg successfully published!\n");
	}
    return ret;
}

void mqtt_pub_msg(const char *msg, int  msg_len)
{
	if(!msg&&msg_len == 0){
		msg_len = strlen(msg);
	}
	//LOGD("pub %s %d\n",msg,msg_len);
	_mqtt_pub_msg(conf->mqtt_pub_topic,msg,msg_len);
}

int mqtt_tpub_msg(const char *topic, const char *msg, int  msg_len)
{
	if(!msg&&msg_len == 0){
		msg_len = strlen(msg);
	}
	return _mqtt_pub_msg(topic,msg,msg_len);

}

static int _mqtt_pub_file(const char *topic ,const char *file)
{
	int fd ,ret = -1;
	int flen;
	struct stat f_stat;
	void *msg;

	if(!is_mqtt_alive()){
		LOGD("mqtt is offline !\n");
		return -1;
	}
	fd  = open(file,O_RDONLY);
	if(fd < 0){
		LOGD("open file %s faild, error:%s ",file,strerror(errno));
		goto out;
	}
	ret = fstat(fd,&f_stat);
	if(ret < 0){
		LOGD("fstat file %s faild, error:%s ",file,strerror(errno));
		goto out;
	}
	flen = f_stat.st_size;
	if(flen == 0 || flen > MAX_MQTT_MESSAGE_SIZE){
		LOGD("file %s size %dis too big or is zero!\n",file,flen);
		goto out;
	}

	msg=mmap(NULL,flen,PROT_READ,MAP_PRIVATE,fd,0);
	if(msg == MAP_FAILED){
		LOGD("mmap file %s faild, error:%s \n",file,strerror(errno));
		goto out;
	}

	_mqtt_pub_msg(topic,msg,flen);
	munmap(msg,flen);

out:
	if(fd > 0){
		close(fd);
	}
	return ret;
}

int mqtt_pub_file(const char *file)
{
	return	_mqtt_pub_file(conf->mqtt_pub_topic,file);
}

int mqtt_tpub_file(const char *topic, const char *file)
{
	return	_mqtt_pub_file(topic,file);
}

void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	struct mqtt_msg *mqmsg;

	if(message->payloadlen){
		LOGD("%s === %s\n", message->topic, (char *)message->payload);
		mqmsg = init_mqtt_msg(message->topic,message->payload,message->payloadlen);
		if(mqmsg){
			list_add_tail(&mqmsg->list,&mqtt_ctx->msgs);
		}else{
			LOGD("init mqtt msg faild!\n");
		}

	}else{
		LOGD("%s (null)\n", message->topic);
	}
	fflush(stdout);
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	int ret;
	int mmid = 1;
    LOGD("connect result is %d\n",result);
    mqtt_connected = true;
	if(!result){
        ret = mosquitto_subscribe(mqtt_ctx->mosq,&mmid,conf->mqtt_sub_topic,1);
        if(ret < 0 ){
            LOGE("subscribe failed %s\n",strerror(errno));
        }
	}else{
		LOGE("Connect failed\n");
	}
}

void my_disconnect_callback(struct mosquitto *mosq, void *userdata, int result)
{
	LOGD("mqtt is disconnected! %d %s\n", result,userdata);
	if(result){
		mqtt_connected = false;
		LOGD("mqtt disconnet from the server!");
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	LOGD("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		LOGD(", %d", granted_qos[i]);
	}
	LOGD("\n");
}


void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	//LOGD("level %d %s\n", level,str);
}


int mqtt_service_init(){
    conf = get_mqtt_conf();
	char *host = conf->server_host;
	int port = conf->server_port;
	int keepalive = conf->keep_alive;
	bool clean_session = true;
	int ret = 0;
	mqtt_ctx = malloc(sizeof(struct mqtt_context));
	if(!mqtt_ctx){
		LOGD("malloc mqtt_ctx failed\n");
		return -1;
	}
	mosquitto_lib_init();
	mqtt_ctx->mosq = mosquitto_new(conf->dev_mac, clean_session, NULL);
	if(!mqtt_ctx->mosq){
		LOGE("Error: Out of memory.\n");
		return 1;
	}

	mosquitto_log_callback_set(mqtt_ctx->mosq, my_log_callback);
	mosquitto_connect_callback_set(mqtt_ctx->mosq, my_connect_callback);
	mosquitto_disconnect_callback_set(mqtt_ctx->mosq,my_disconnect_callback);
	mosquitto_message_callback_set(mqtt_ctx->mosq, my_message_callback);
	mosquitto_subscribe_callback_set(mqtt_ctx->mosq, my_subscribe_callback);


	if((ret=mosquitto_connect_async(mqtt_ctx->mosq, host, port, keepalive)) != 0){
		LOGE("Unable to connect.will retry later\n");
	}


	if(conf->mqtt_name && conf->mqtt_passwd ){
        if(strlen(conf->mqtt_name) !=0 && strlen(conf->mqtt_passwd) != 0){
            LOGD("set mqtt username and password as %s %s\n",conf->mqtt_name,conf->mqtt_passwd);
		    mosquitto_username_pw_set(mqtt_ctx->mosq,conf->mqtt_name,conf->mqtt_passwd);
        }
	}
	INIT_LIST_HEAD(&mqtt_ctx->msgs);
	return mosquitto_loop_start(mqtt_ctx->mosq);

}

void stop_mqtt_worker(){
	if(mqtt_ctx->mosq){
		mosquitto_loop_stop(mqtt_ctx->mosq,false);
		mosquitto_disconnect(mqtt_ctx->mosq);
		mosquitto_destroy(mqtt_ctx->mosq);
	}
	mosquitto_lib_cleanup();

}
REG_SERVICE(MQTT_SERVICE,mqtt_service_init)
