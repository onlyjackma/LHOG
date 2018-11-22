/*
 * iot_do_test_debug.c
 *
 *  Created on: 2018��7��9��
 *      Author: xiaomage
 */

#include "lhog.h"

int ubus_do_debug_switch_handler(char *cmd,char *para,struct blob_buf *b)
{
	int debug_en = atoi(para);

	if(debug_en == 0){
		set_log_debug(false);
	}else{
		set_log_debug(true);
	}
	return 0;
}

REG_UBUS_CMD_HANDLER(debug,ubus_do_debug_switch_handler);

