#include "lhog.h"

struct mqtt_config *mqtt_config = NULL;

char *mqtt_server = "test.iovyw.com";
short mqtt_port = 1883;

struct mqtt_config *get_mqtt_conf()
{
    return mqtt_config;

}
static char *gen_dev_mac()
{
    char *mac = malloc(13);

    srand(time(NULL));

    snprintf(mac,13,"AG%02x%02x%02x%02x%02x",rand()%123,rand()%122,rand()%125,rand()%128,rand()%126);
    return mac;
}

int init_mqtt_config(char *conf_path)
{
	FILE *cf = fopen(conf_path,"rb");
	long c_len = 0;
	int nread = 0;
	char *c_buf = NULL;
	mqtt_config = malloc(sizeof(struct mqtt_config));
    cJSON *root = NULL;
    cJSON *item = NULL;
    char tmp[48] = {0};



	if(cf == NULL || mqtt_config == NULL){
		LOGD("Invalid config path\n");
		return -1;
	}

	fseek(cf,0,SEEK_END);
	c_len = ftell(cf);
	fseek(cf, 0, SEEK_SET);

	if(c_len > MAX_CONF_SIZE){
		LOGD("config file too big\n");
		fclose(cf);
		return -1;
	}

	c_buf = malloc(c_len+1);
	nread = fread(c_buf,c_len,1,cf);
	if(nread <= 0){
		LOGD("read config file error\n");
		fclose(cf);
		return -1;
	}

	fclose(cf);
	c_buf[c_len] = '\0';
	root = cJSON_Parse(c_buf);

	if(root != NULL){
		item = cJSON_GetObjectItem(root,"server_host");
    	if(item != NULL){
    		mqtt_config->server_host = strdup(item->valuestring);
    	}else{
			mqtt_config->server_host = strdup(mqtt_server);
		}
        if(mqtt_config->server_host == NULL){
            mqtt_config->server_host = strdup(mqtt_server);
        }
        if(strlen(mqtt_config->server_host) == 0){
            mqtt_config->server_host = strdup(mqtt_server);
        }

		item = cJSON_GetObjectItem(root,"server_port");
    	if(item != NULL){
    		mqtt_config->server_port = item->valueint;
    	}else{
			mqtt_config->server_port = mqtt_port;
		}
        if(mqtt_config->server_port == 0){
            mqtt_config->server_port = mqtt_port;
        }

		item = cJSON_GetObjectItem(root,"keep_alive");
    	if(item != NULL){
    		mqtt_config->keep_alive = item->valueint;
    	}else{
			mqtt_config->keep_alive = 30;
		}
        if(mqtt_config->keep_alive == 0){
            mqtt_config->keep_alive = 30;
        }

		item = cJSON_GetObjectItem(root,"mqtt_name");
    	if(item != NULL){
    		mqtt_config->mqtt_name = strdup(item->valuestring);
    	}else{
    		mqtt_config->mqtt_name = NULL;
		}

		item = cJSON_GetObjectItem(root,"mqtt_passwd");
    	if(item != NULL){
    		mqtt_config->mqtt_passwd = strdup(item->valuestring);
    	}else{
    		mqtt_config->mqtt_passwd = NULL;
		}

        item = cJSON_GetObjectItem(root,"dev_mac");
    	if(item != NULL){
    		mqtt_config->dev_mac = strdup(item->valuestring);
    	}else{
			mqtt_config->dev_mac = gen_dev_mac();
		}
        if(mqtt_config->dev_mac == NULL){
            mqtt_config->dev_mac = gen_dev_mac();
        }
        if(strlen(mqtt_config->dev_mac) == 0){
            mqtt_config->dev_mac = gen_dev_mac();
        }

        snprintf(tmp,48,"/dev/%s/lhog",mqtt_config->dev_mac);
        
        mqtt_config->mqtt_sub_topic = strdup(tmp);
        memset(tmp,0,48);
        snprintf(tmp,48,"/plat/%s/lhog",mqtt_config->dev_mac);
        mqtt_config->mqtt_pub_topic = strdup(tmp);
    
	}else{
    	LOGD("Error before: [%s]\n",cJSON_GetErrorPtr());
    	return -1;
	}


	LOGD("dev_mac %s\n server_host %s \n server_port %d\n  keep_alive %d\n sub_topic %s\n pub_topic %s", \
			mqtt_config->dev_mac, mqtt_config->server_host,mqtt_config->server_port,mqtt_config->keep_alive, \
            mqtt_config->mqtt_sub_topic,mqtt_config->mqtt_pub_topic);

	cJSON_Delete(root);
	return 0;
}

int deinit_mqtt_config()
{

	if(mqtt_config != NULL){
		if(mqtt_config->dev_mac != NULL){
			free(mqtt_config->dev_mac);
		}

		if(mqtt_config->server_host != NULL){
			free(mqtt_config->server_host);
		}

		if(mqtt_config->mqtt_name != NULL){
			free(mqtt_config->mqtt_name);
		}
		if(mqtt_config->mqtt_passwd != NULL){
			free(mqtt_config->mqtt_passwd);
		}

		free(mqtt_config);
	}

	return 0;

}
