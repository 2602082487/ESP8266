#include "esp_common.h"
#include "driver/i2c_master.h"
#include "driver/light.h"
#include "EdpKit.h"
#include "lwip/sockets.h"
#include "driver/dht11.h"
#include "driver/gpio.h"
#include "airkiss.h"
#include "user_task.h"




/*********************varible define*************************/
int s;
//char ssid[]="LieBaoWiFi625";
//char password[]="12345613";
char ssid[]="HUAWEI";
char password[]="huawei1234";
char buffer[512];




/************************************************************/
void smartconfig_done(sc_status status, void *pdata)
{
	switch(status) 
	{
		case SC_STATUS_WAIT:
	    		os_printf("SC_STATUS_WAIT\n");
	    		break;
				
		case SC_STATUS_FIND_CHANNEL:
	    		os_printf("SC_STATUS_FIND_CHANNEL\n");
	    		break;
				
		case SC_STATUS_GETTING_SSID_PSWD:
	    		os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
				
	    		sc_type *type = pdata;
		    	if (*type == SC_TYPE_ESPTOUCH) 
			{
		      		os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
		    	} 
			else 
			{
		        	os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
			}
			break;
			
		case SC_STATUS_LINK: 
		{
			os_printf("SC_STATUS_LINK\n");
			struct station_config *sta_conf = pdata;

			wifi_station_set_config(sta_conf);
			wifi_station_disconnect();
			wifi_station_connect();
		}
			break;
			
		case SC_STATUS_LINK_OVER: {
			os_printf("SC_STATUS_LINK_OVER\n");
			smartconfig_stop();
			delay_ms(10);
			//system_restart();
			break;
		}
	}

}


//void smartconfig_done(sc_status status, void *pdata)
//{
//    switch(status) {
//        case SC_STATUS_WAIT:
//            printf("SC_STATUS_WAIT\n");
//            break;
//        case SC_STATUS_FIND_CHANNEL:
//            printf("SC_STATUS_FIND_CHANNEL\n");
//            break;
//        case SC_STATUS_GETTING_SSID_PSWD:
//            printf("SC_STATUS_GETTING_SSID_PSWD\n");
//            sc_type *type = pdata;
//            if (*type == SC_TYPE_ESPTOUCH) {
//                printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
//            } else {
//                printf("SC_TYPE:SC_TYPE_AIRKISS\n");
//            }
//            break;
//        case SC_STATUS_LINK:
//            printf("SC_STATUS_LINK\n");
//            struct station_config *sta_conf = pdata;
//	
//	        wifi_station_set_config(sta_conf);
//	        wifi_station_disconnect();
//	        wifi_station_connect();
//            break;
//        case SC_STATUS_LINK_OVER:
//            printf("SC_STATUS_LINK_OVER\n");
//            if (pdata != NULL) {
//				//SC_TYPE_ESPTOUCH
//                uint8 phone_ip[4] = {0};

//                memcpy(phone_ip, (uint8*)pdata, 4);
//                printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
//            } else {
//            	//SC_TYPE_AIRKISS - support airkiss v2.0
//				airkiss_start_discover();
//			}
//            smartconfig_stop();
//            break;
//    }
//	
//}



void smartconfig_task()
{
	if(GPIO_INPUT_GET(4)==0)
	{
		smartconfig_start(smartconfig_done);
	}
	vTaskDelete(NULL);
}




void send_task()
{
	EdpPacket *pkg;
	struct sockaddr_in remote_ip;
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = inet_addr(SERVER_IP);
	remote_ip.sin_port = htons(SERVER_PORT);
	xSemaphoreTake(wifi_sta,0);
	while(xSemaphoreTake(wifi_sta,0)!=pdTRUE)
	{
		vTaskDelay(100);
	}
//	vTaskDelay(1000);
	s=socket(AF_INET,SOCK_STREAM,0);
	if(connect(s,(struct sockaddr *)(&remote_ip), sizeof(struct sockaddr))!=0)
		os_printf("\r\nsocket connect errro\r\n");
	xTaskCreate(recv_task,"recv_task",200,NULL,7,&Handle_recv_task);
	xTaskCreate(ping_task,"ping_task",200,NULL,2,&Handle_ping_task);
	while(1)
	{
		if(xQueueReceive(send_queue,&pkg,0)==pdTRUE)
		{
			int i;
			send(s,pkg->_data,pkg->_write_pos,0);
			os_printf("\r\nESP8266@GOD:socket send ping\r\n");
			for(i=0;i<pkg->_write_pos;i++)
				os_printf(" %X",(pkg->_data)[i]);
			DeleteBuffer(&pkg);
		}
		else
		{
//			os_printf("\r\nESP8266@GOD:receive fail\r\n");
		}
		vTaskDelay(20);
	}
}


void light_task()
{
	uint16 light_value;
	char str_value[30];
	EdpPacket *pkg;
	bh1750_init();
#ifdef DEBUG_PRINT
	os_printf("\r\nESP8266@GOD:bh1750 init finish\r\n");
#endif
	while(1)
	{
		while(xSemaphoreTake(driver,0)!=pdTRUE)
		{
			vTaskDelay(100);
		}
		light_value=bh1750_GetLightValue();
#ifdef DEBUG_PRINT
		os_printf("\r\nESP8266@GOD:light value:%d\r\n",light_value);
#endif
		sprintf(str_value,",;light,%d",light_value);
		pkg=PacketSavedataSimpleString(device_id, str_value);
		xQueueSend(send_queue,&pkg,0);
		xSemaphoreGive(driver);
		vTaskDelay(5000);
	}
}


void dht11_task()
{
	float hum,temp;
	uint8 temp_hum[HUM_DATA_SIZE]={0};
	uint8 str[30];
	EdpPacket *pkg;
	DHT11_init();
	while(1)
	{
		while(xSemaphoreTake(driver,0)!=pdTRUE)
		{
			vTaskDelay(100);
		}
		DHT11_read_temp_hum(temp_hum,HUM_DATA_SIZE);
		//temp=temp_hum[2]+temp_hum[3]
		sprintf(str, ",;temp,%d.%d;hum,%d.%d", temp_hum[2], temp_hum[3],temp_hum[0],temp_hum[1]);
		printf("temperature is:%d.%d,humidity is:%d.%d",temp_hum[2],temp_hum[3],temp_hum[0],temp_hum[1]);
		pkg=PacketSavedataSimpleString(device_id, str);
		xQueueSend(send_queue,&pkg,0);
		xSemaphoreGive(driver);
		vTaskDelay(5000);
		
	}
}


void ping_task()
{
	EdpPacket *pkg;
	pkg=PacketConnect1(device_id,api_key);
	if(xQueueSend(send_queue,&pkg,5)==pdTRUE)
		os_printf("\r\nESP8266@GOD:Queue send ok!\r\n");
	else
		os_printf("\r\nESP8266@GOD:Queue send fail!\r\n");
	vTaskDelay(20);
	xTaskCreate(light_task,"light_task",200,NULL,6,&Handle_light_task);
	xTaskCreate(dht11_task,"dht11_task",200,NULL,7,&Handle_dht11_task);
	while(1)
	{	
		pkg=PacketPing();
		os_printf("\r\nping task erter\r\n");
		if(xQueueSend(send_queue,&pkg,0)==pdTRUE)
			os_printf("\r\nESP8266@GOD:Queue send ok!\r\n");
		else
			os_printf("\r\nESP8266@GOD:Queue send fail!\r\n");
		vTaskDelay(5000);
	}
}


void recv_task()
{
	int i,n;
	uint8 mtype;
	RecvBuffer* recv_buf = NewBuffer();
	EdpPacket *pkg;
	char* cmdid;
    uint16 cmdid_len;
    char*  cmd_req;
    uint32 cmd_req_len;
	while(1)
	{
		n=recv(s,buffer,512,MSG_DONTWAIT);
		if(n>0)
		{
			printf("ESP8266@GOD:recived data:\r\n");
#ifdef DEBUG_PRINT
			for(i=0;i<n;i++)
			{
				printf("%X ",buffer[i]);
			}
			printf("\r\n");
#endif
		WriteBytes(recv_buf, buffer, n);
		if((pkg=GetEdpPacket(recv_buf))!=0)
		{
			mtype=EdpPacketType(pkg);
			switch(mtype)
			{
				case CONNRESP:
					os_printf("\r\nCONNRESP\r\n");
					break;
				case PUSHDATA:
					os_printf("\r\nPUSHDATA\r\n");
					break;
				case SAVEDATA:
					os_printf("\r\nSAVEDATA\r\n");
					break;
				case SAVEACK:
					os_printf("\r\nSAVEDATA\r\n");
					break;
				case CMDREQ:
					os_printf("\r\nCMDREQ\r\n");
					break;
				case PINGRESP:
					os_printf("\r\nPINGRESP\r\n");
				default:
					os_printf("\r\ndefault:recv failde!\r\n");
			}
			DeleteBuffer(&pkg);
		}
		}
		vTaskDelay(10);
	}
}


void network_task()
{
	struct station_config * config = (struct station_config *)zalloc(sizeof(struct station_config));
	sprintf(config->ssid,ssid);
	sprintf(config->password, password);
	config->bssid_set=0;
	wifi_set_opmode(STATION_MODE);
	wifi_station_set_config(config);
	while(1)
	{
		switch(wifi_station_get_connect_status ())
		{
			case STATION_GOT_IP:
				printf("ESP8266@GOD:Get ip\r\n");
				xSemaphoreGive(wifi_sta);
				vTaskSuspend(Handle_network_task);
				break;
			case STATION_CONNECT_FAIL:
				printf("ESP8266@GOD:Connect failed\r\n");
				break;
			case STATION_WRONG_PASSWORD:
				printf("ESP8266@GOD:Password:%s is wrong\r\n",password);
				break;
			case STATION_NO_AP_FOUND:
				printf("ESP8266@GOD:Not find %s\r\n",ssid);
				break;
			case STATION_CONNECTING:
				printf("ESP8266@GOD:Connecting now\r\n");
				break;
			case STATION_IDLE:
				printf("ESP8266@GOD:WiFi idle\r\n");
				break;
			default:
				break;
		}
		vTaskDelay(100);
	}
}





