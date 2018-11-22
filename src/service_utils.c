#include "lhog.h"

static struct list_head service_list = LIST_HEAD_INIT(service_list);

void register_service(char * name ,service_init init)
{
	struct service *service = (struct service *)malloc(sizeof(struct service));
	if(service != NULL){
		service->name = strdup(name);
		service->init = init;
		list_add_tail(&service->head,&service_list);
	}else{
		LOGD("register %s service failed\n",name);
	}
}

int init_all_service()
{
	struct service *iot_s = NULL;
	int ret = 0;
	list_for_each_entry(iot_s,&service_list,head){
		ret = iot_s->init();
		LOGD("init iot %s service result is %d\n",iot_s->name,ret);
//		if(ret != 0){
//			return -1;
//		}
	}
	return ret;
}
