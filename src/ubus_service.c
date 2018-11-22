#include "lhog.h"
static struct ubus_context *ctx;
static struct blob_buf b;

static struct list_head cmd_handler_list = LIST_HEAD_INIT(cmd_handler_list);
void register_ubus_handler(char *cmd , ubus_cmd_handler handler)
{
	struct ubus_cmd_handler *uch = (struct ubus_cmd_handler *)malloc(sizeof(struct ubus_cmd_handler));
	if(uch != NULL){
		uch->cmd = strdup(cmd);
		uch->handler = handler;
		list_add_tail(&uch->head,&cmd_handler_list);
	}else{
		LOGD("malloc ubus_cmd_handler failed\n");
	}
}

enum {
	CMD_STRING,
	CMD_PARA,
	__CMD_MAX
};
static const struct blobmsg_policy do_cmd_policy[__CMD_MAX] = {
		[CMD_STRING] = {.name = "cmd", .type = BLOBMSG_TYPE_STRING},
		[CMD_PARA] = {.name = "para" , .type = BLOBMSG_TYPE_STRING},
};

static int iot_do_cmd(struct ubus_context *ctx, struct ubus_object *obj,
        struct ubus_request_data *req, const char *method,
        struct blob_attr *msg)
{
	struct blob_attr *tb[__CMD_MAX];
	char *cmd_str,*cmd_para;
	struct ubus_cmd_handler *uch = NULL;

	blobmsg_parse(do_cmd_policy,__CMD_MAX,tb,blob_data(msg),blob_len(msg));
	if(!tb[CMD_STRING]){
		return UBUS_STATUS_INVALID_ARGUMENT;
	}
	cmd_str = blobmsg_get_string(tb[CMD_STRING]);
	cmd_para = blobmsg_get_string(tb[CMD_PARA]);
	if(!cmd_str){
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
	if(!cmd_str){
		LOGD("cmd is %s ,cmd parameter is %s \n",cmd_str,cmd_para);
	}
	blob_buf_init(&b,0);
	//blobmsg_add_string(&b,"data",cmd_str);
	//blobmsg_add_u8(&b,"result",1);

	list_for_each_entry(uch,&cmd_handler_list,head){
		if(strncmp(cmd_str,uch->cmd,strlen(cmd_str)) == 0 &&
			strncmp(cmd_str,uch->cmd,strlen(uch->cmd)) == 0){
			uch->handler(cmd_str,cmd_para,&b);
		}
	}
	ubus_send_reply(ctx,req,b.head);
	return 0;
}


static const struct ubus_method iot_methods[] = {
		UBUS_METHOD("do_cmd",iot_do_cmd,do_cmd_policy),
		//UBUS_METHOD("get_status",iot_get_status,get_status_policy),
};

static struct ubus_object_type iot_object_type = UBUS_OBJECT_TYPE("lhog",iot_methods);
static struct ubus_object iaiot_object = {
		.name = "lhog",
		.type = &iot_object_type,
		.methods = iot_methods,
		.n_methods = ARRAY_SIZE(iot_methods),
};

int ubus_main(void)
{
	int ret;
	ret = ubus_add_object(ctx,&iaiot_object);
	if(ret){
		LOGE("Failed to add object: %s\n", ubus_strerror(ret));
	}
	return ret;
}

static void ubus_connect_cb(struct ubus_context *uctx)
{
	ctx = uctx;
	ubus_main();

}
static struct ubus_auto_conn conn;


int ubus_service_init()
{
	conn.cb = ubus_connect_cb;
	ubus_auto_connect(&conn);
	return 0;
}

REG_SERVICE(UBUS_SERVICE,ubus_service_init)
