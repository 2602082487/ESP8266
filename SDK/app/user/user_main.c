/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

//#include "espconn.h"
//#include "lwip/mem.h"
#include "json/cJSON.h"
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "espressif/espconn.h"
#include "espressif/airkiss.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "Led.h"
#include "UdpServer.h"
#include "EdpKit.h"

//#define server_ip "183.230.40.39"
//#define server_port 876

#define SERVER_IP "183.230.40.39"
#define SERVER_PORT 876

#define AP_SSID "fox"
#define AP_PASSWORD "woyushentongzai"

#define device_id "3314617"
#define api_key   "5ktVfVQLSGXZfj8szB6dDofdDmE="

uint8 mac[6]={0x0C, 0x51, 0x01, 0xCB, 0x38, 0x31};

int sta_socket;
xTaskHandle xHandle;


void recv_thread_func(void *pvParameters);


/**
 *FunctionName : put_data
 *Description  : none
 *Parmeters    : none
 *Return       : none
**/
void put_data(void *pvParameters)
{
	uint16 adc;
	double voltage;
	EdpPacket *pkg;
	while(1)
	{
		adc=536;//system_adc_read ();
		voltage= (double)adc/1024;
		pkg=PacketSavedataDouble(kTypeSimpleJsonWithoutTime,device_id,"addisplay",voltage,0,NULL);
		printf("start send");
		send(sta_socket,pkg->_data,pkg->_write_pos,0);
		vTaskDelay(5000);
	}
}


/**
 *FunctionName : OneNET_Heartbeat_task
 *Description  : none
 *Parmeters    : none
 *Return       : none
**/
void OneNET_Heartbeat_task(void *pvParameters)
{
	EdpPacket *pkg;
	int count;
	pkg=PacketPing();
	while(1)
	{
		printf("\r\nping\r\n");
		count = send(sta_socket,pkg->_data,pkg->_write_pos,0);
		vTaskDelay(5000);
	}
	
}




/**
 *FunctionName : OneNET_task
 *Description  : none
 *Parmeters    : none
 *Return       : none
**/
void OneNET_task(void *pvParameters)
{
	EdpPacket * pkg;
	struct sockaddr_in remote_ip;
	u8 sta;
	int count;
	int i;
	//unsigned long iMode=1;
	int iMode=1;
	pkg=PacketConnect1(device_id,api_key);
	while(1)
	{
		sta=wifi_station_get_connect_status ();
		if(sta != STATION_GOT_IP )
		{
			switch(sta)
			{
				case 0:
					printf("STATION_IDLE");
				case 2:
					printf("STATION_WRONG_PASSWORD");
				case 3: 
					printf("STATION_NO_AP_FOUND");
				case 4:
					printf("STATION_CONNECT_FAIL");
				case 5:
					printf("STATION_GOT_IP");
			}
		}
		else
		{
			sta_socket = socket(PF_INET, SOCK_STREAM, 0);
			if (-1 == sta_socket) 
			{
				close(sta_socket);
				vTaskDelay(1000 / portTICK_RATE_MS);
				printf("ESP8266 TCP client task > socket fail!\n");
				continue;
			}
			else
			{
				ioctlsocket(sta_socket, FIONBIO, &iMode);
				printf("ESP8266 TCP client task > socket ok!\n");
			}
	
			bzero(&remote_ip, sizeof(struct sockaddr_in));
			remote_ip.sin_family = AF_INET;
			remote_ip.sin_addr.s_addr = inet_addr(SERVER_IP);
			remote_ip.sin_port = htons(SERVER_PORT);
			if (0 != connect(sta_socket, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr))) 
			{	
				//close(sta_socket);
				vTaskDelay(1000 / portTICK_RATE_MS);
				printf("ESP8266 TCP client task > connect fail!\n");
			}
			else
			{
				printf("ESP8266 TCP client task > connect ok!\n");
			}
			count = send(sta_socket,pkg->_data,pkg->_write_pos,0);
			printf("\r\ncount=%d\r\n",count);
			if(count==pkg->_write_pos)
			{
				vTaskDelay(1000 / portTICK_RATE_MS);
				printf("ESP8266 TCP client task > send success\n");
				printf("%d\r\n",count);
			}
			else
			{
				//close(sta_socket);
				printf("ESP8266 TCP client task > send fail\n");
				printf("%d",count);
			}
			DeleteBuffer(&pkg);
		}
		vTaskDelay(1000);
	}
	DeleteBuffer(&pkg);
}


 



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	STATION_STATUS wifi_sta;
	struct station_config * config = (struct station_config *)zalloc(sizeof(struct station_config));
	user_uart_init( );
    printf("\r\nSDK version:%s\r\n", system_get_sdk_version());
	led_init( );
	
    /* need to set opmode before you set config */
    wifi_set_opmode(STATION_MODE);
    wifi_set_macaddr(0x00,mac);
	sprintf(config->ssid, AP_SSID);
	sprintf(config->password, AP_PASSWORD);
	wifi_station_set_config(config);
	free(config);
	xTaskCreate(OneNET_task,"OneNET_task",256,NULL,4,&xHandle);
	xTaskCreate(recv_thread_func,"recv_thread_func",512,NULL,0,NULL);

}



void recv_thread_func(void *pvParameters)
{    
	//int sockfd = *(int*)arg;
	
	int error = 0;
	int n, rtn;
	uint8 mtype, jsonorbin;
	char *buffer=malloc(1024);
	RecvBuffer* recv_buf = NewBuffer();
	EdpPacket* pkg;
	char* src_devid;
	char* push_data;
	uint32 push_datalen;
	cJSON* save_json;
	char* save_json_str;
	cJSON* desc_json;
	char* desc_json_str;
	char* save_bin;
	uint32 save_binlen;
	char* json_ack;
	char* cmdid;
	uint16 cmdid_len;
	char*  cmd_req=malloc(256);
	uint32 cmd_req_len;
	EdpPacket* send_pkg;
	char* ds_id; 
	double dValue = 0;
	int iValue = 0; 
	char* cValue = NULL;
	char* simple_str = NULL;
	char cmd_resp[] = "ok";  
	unsigned cmd_resp_len = 0;
	
	while(1)
	{
		printf("\r\nstart recv!\r\n");
		n=recv(sta_socket,buffer,1024,0);
		if(n<=0)
		{
			printf("\r\nrecv status: %d\r\n",n);
			vTaskDelay(50);
			continue;
		}
		printf("\r\nn>0 and n=%d\r\n",n);
		WriteBytes(recv_buf, buffer, n);
		if ((pkg = GetEdpPacket(recv_buf)) == 0)
		{                
		 	printf("need more bytes...\n"); 
		 	vTaskDelay(50);
		 	continue;            
		}
		mtype = EdpPacketType(pkg);
		UnpackCmdReq(pkg, &cmdid, &cmdid_len, &cmd_req,&cmd_req_len);
		switch(mtype)
		{
			case CONNRESP:
				//if(cmd_req[0]==0)
				{
					printf("\r\nrecv ok\r\n");
					vTaskSuspend(xHandle);
					xTaskCreate(OneNET_Heartbeat_task,"OneNET_Heartbeat_task",256,NULL,3,NULL);
					xTaskCreate(put_data,"put_data",512,NULL,3,NULL);
				}
				break;
			case PUSHDATA:
				break;
			case SAVEDATA:
				break;
			case SAVEACK:
				break;
			case CMDREQ:
				if(cmd_req[0]=='0')
					led_on();
				else
					led_off();
				
				break;
			case PINGRESP:
				break;
			default:
				break;
		}
		vTaskDelay(100);

	}
}
       