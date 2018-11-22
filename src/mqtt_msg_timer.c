#include "lhog.h"

void do_plat_protocol_dispatcher();

struct uloop_timeout dtm = {
		.cb = do_plat_protocol_dispatcher,
};

extern int iot_mqtt_upgrade();
void do_plat_protocol_dispatcher()
{
	dispatch_proto_data();
	//iot_mqtt_upgrade();
	uloop_timeout_set(&dtm,1000);
}

int do_plat_proto_dispatch_init()
{
	return uloop_timeout_set(&dtm,1000);
}
REG_SERVICE(DO_PLAT_PROTO_DISPATCH,do_plat_proto_dispatch_init)