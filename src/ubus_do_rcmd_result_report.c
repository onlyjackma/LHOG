/*
 * ubus_do_rcmd_result_report.c
 *
 *  Created on: 2018年6月15日
 *      Author: xiaomage
 */

#include "lhog.h"

#define RESULT_F "/tmp/.result"


int ubus_do_rcmd_result_report_handler(char *cmd,char *para,struct blob_buf *b)
{
	int ret;
	struct mqtt_config *conf = get_mqtt_conf();

	ret = mqtt_tpub_file(conf->mqtt_pub_topic, para);
	LOGD("Report rcmd result %d\n",ret);
	return ret;
}

REG_UBUS_CMD_HANDLER(rcmd_report,ubus_do_rcmd_result_report_handler);
