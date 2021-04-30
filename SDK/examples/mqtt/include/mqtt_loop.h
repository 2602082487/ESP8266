#include "esp_common.h"
#include "mqtt.h"


#ifdef WIN32
#error Not support Windows now.
#endif // WIN32




struct MqttSampleContext
{    
	uint32_t sendedbytes;    
	struct MqttContext mqttctx[1];    
	struct MqttBuffer mqttbuf[1];    
	const char *proid;    
	const char *devid;    
	const char *apikey;    
	int dup;    
	enum MqttQosLevel qos;    
	int retain;    
	uint32_t publish_state;    
	uint16_t pkt_to_ack;    
	char cmdid[MAX_MQTT_LEN];
};






#define DS_TO_PUBLISH "mqtt_msg"

#define DS_TO_PUBLISH_T "mqtt_msg_t"

#define DS_TO_PUBLISH_RH "mqtt_msg_rh"

#define TOPIC_TO_SUB "39484/nCVNXYCoX68IHG4DgpyNu5aTXfY=/769243/da_test_a"
#define PACK_FALG_UNSUB 11
#define TOPIC_TO_UNSUB "39484/nCVNXYCoX68IHG4DgpyNu5aTXfY=/769243/da_test_a"

#define TIME_OUT 1
#define EVENT 2

#define MQTT_DEVICE_PROJ_ID "73518"
#define MQTT_DEVICE_ID "3505516"
#define MQTT_DEVICE_API_KEY  "QnEkAhUXGpZ7XTF1jV7xhasd16c="
#define u8 unsigned char

void MQTT_Loop(void);
