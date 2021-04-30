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
#include "esp_common.h"
#include "mqtt_loop.h"
#include "uart.h"
#include "lwip/Sockets.h"
#include "gpio.h"


#define SERVER_IP "183.230.40.33"
#define SERVER_PORT 80

int s;
char rq[]="GET /devices/5273738/datapoints HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\n\r\n";
char low[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:17\r\n\r\n{\"sw1\":0,\"sw2\":0}";
char high[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:17\r\n\r\n{\"sw1\":1,\"sw2\":1}";
char low1[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:9\r\n\r\n{\"sw1\":0}";
char low2[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:9\r\n\r\n{\"sw2\":0}";
char high1[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:9\r\n\r\n{\"sw1\":1}";
char high2[]="POST /devices/5273738/datapoints?type=3 HTTP/1.1\r\napi-key:n0TmIPxUOZNMwmD1pekSEiATtLU=\r\nHost:api.heclouds.com\r\nConnection: close\r\nContent-Length:9\r\n\r\n{\"sw2\":1}";




char buffer[1024];
char h[512];
char button1=0,button2=0;

void key_task(void *pvParameters)
{
	struct sockaddr_in remote_ip;
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = inet_addr(SERVER_IP);
	remote_ip.sin_port = htons(SERVER_PORT);
	while(1)
	{
		if(GPIO_INPUT_GET(13)==0)
		{
			button1=1;
			printf("\r\n.......................................................\r\n");
		}
		if(GPIO_INPUT_GET(2)==0)
		{
			button2=1;
			printf("\r\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\r\n");
		}
		vTaskDelay(10);
	}
}




void http_task(void *pvParameters)
{
	printf("http run.....................................................\r\n");
	int len;
	struct sockaddr_in remote_ip;
	//int iMode=1;
	//s=socket(AF_INET,SOCK_STREAM,0);
	//ioctlsocket(s, FIONBIO, &iMode);
	remote_ip.sin_family = AF_INET;
	remote_ip.sin_addr.s_addr = inet_addr(SERVER_IP);
	remote_ip.sin_port = htons(SERVER_PORT);
	while(1)
	{
		if(wifi_station_get_connect_status ()==STATION_GOT_IP)
		{
			s=socket(AF_INET,SOCK_STREAM,0);
			connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
			send(s,rq,strlen(rq),0);
			len=recv(s,buffer,1024,0);
			close(s);
			buffer[len]='\0';
			printf("\r\n%s\r\n",buffer);
			printf("\r\n%c\r\n",buffer[272]);
			printf("\r\n%c\r\n",buffer[344]);
			printf("\r\n%c\r\n",buffer[415]);
			if(buffer[272]=='1')
			{
				s=socket(AF_INET,SOCK_STREAM,0);
				connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
				if((GPIO_INPUT_GET(4))&&(GPIO_INPUT_GET(5)))
				{
					GPIO_OUTPUT(GPIO_Pin_14,0);
					GPIO_OUTPUT(GPIO_Pin_12,0);
					send(s,high,strlen(high),0);
					recv(s,h,512,0);
				}
				else
				{
					GPIO_OUTPUT(GPIO_Pin_14,1);
					GPIO_OUTPUT(GPIO_Pin_12,1);
					send(s,low,strlen(low),0);
					recv(s,h,512,0);
				}
				close(s);
			}
			else
			{
				if(buffer[344]=='1')
				{
					GPIO_OUTPUT(GPIO_Pin_14,0);
				}
				else
				{
					GPIO_OUTPUT(GPIO_Pin_14,1);
				}
				if(buffer[415]=='1')
				{
					GPIO_OUTPUT(GPIO_Pin_12,0);
				}
				else
				{
					GPIO_OUTPUT(GPIO_Pin_12,1);
				}
				if((button1==1)&&(GPIO_INPUT_GET(13)==1))
				{
					printf("\r\n###################################################\r\n");
					if(buffer[344]=='1')
					{
						GPIO_OUTPUT(GPIO_Pin_14,1);
						s=socket(AF_INET,SOCK_STREAM,0);
						connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
						send(s,low1,strlen(low1),0);
						recv(s,h,512,0);
						close(s);
						printf("\r\n?????????????????????????????????????????????\r\n");
					}
					else
					{
						GPIO_OUTPUT(GPIO_Pin_14,0);
						s=socket(AF_INET,SOCK_STREAM,0);
						connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
						send(s,high1,strlen(high1),0);
						recv(s,h,512,0);
						close(s);
					}
					button1=0;
				}
				if(button2==1&&GPIO_INPUT_GET(2)==1)
				{
					if(buffer[415]=='1')
					{
						GPIO_OUTPUT(GPIO_Pin_12,1);
						s=socket(AF_INET,SOCK_STREAM,0);
						connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
						send(s,low2,strlen(low2),0);
						recv(s,h,512,0);
						close(s);
					}
					else
					{
						GPIO_OUTPUT(GPIO_Pin_12,0);
						s=socket(AF_INET,SOCK_STREAM,0);
						connect(s, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr));
						send(s,high2,strlen(high2),0);
						recv(s,h,512,0);
						close(s);
					}
					button2=0;
				}
			}
		}
		else
		{
			printf("\r\nNot connected wifi............................\r\n");
		}
		vTaskDelay(100);
		
	}
	
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
	GPIO_ConfigTypeDef  GPIOConfig;
    STATION_STATUS wifi_sta;
	struct station_config * config = (struct station_config *)zalloc(sizeof(struct station_config));
	user_uart_init();
    printf("\r\nSDK version:%s\r\n", system_get_sdk_version());
    printf("run ok\r\n");
	
    /* need to set opmode before you set config */
    wifi_set_opmode(STATION_MODE);
    //wifi_set_macaddr(0x00,mac);
	sprintf(config->ssid,"zyddyz");
	sprintf(config->password, "ZYDDYZ01");
	wifi_station_set_config(config);
	GPIO_AS_INPUT(GPIO_Pin_9);
	GPIO_AS_INPUT(GPIO_Pin_10);
	GPIO_AS_INPUT(GPIO_Pin_13);
	GPIO_AS_INPUT(GPIO_Pin_2);
	/*
	GPIO_AS_OUTPUT(GPIO_Pin_14);
	GPIO_AS_OUTPUT(GPIO_Pin_12);
	GPIO_OUTPUT(GPIO_Pin_14,0);
	GPIO_OUTPUT(GPIO_Pin_14,1);
	*/
	GPIOConfig.GPIO_Pin = GPIO_Pin_14;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Output;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIOConfig.GPIO_Pin = GPIO_Pin_12;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Output;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIOConfig.GPIO_Pin = GPIO_Pin_5;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Input;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIOConfig.GPIO_Pin = GPIO_Pin_4;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Input;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIOConfig.GPIO_Pin = GPIO_Pin_13;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Input;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIOConfig.GPIO_Pin = GPIO_Pin_2;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Input;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
	GPIO_OUTPUT(GPIO_Pin_14,0);
	GPIO_OUTPUT(GPIO_Pin_12,0);
   	xTaskCreate(http_task,"http_task",256,NULL,3,NULL);
   	xTaskCreate(key_task,"key_task",256,NULL,2,NULL);
   	printf("task creat ok\r\n");  
}



