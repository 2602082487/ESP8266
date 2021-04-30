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
#include "user_plug.h"
#include "espressif/esp_softap.h"
#include "esp_softap.h"
#include "driver/uart.h"



#include "httpd.h"
#include "cgiwifi.h"
#include "httpdespfs.h"
#include "user_cgi.h"



#define SSID "Plug"
#define PWD "Hisense"


int on_time=60;
int off_time=60;

xTaskHandle xHandle_plug_on_off_task = NULL;


//int   cgiRedirectApClientToHostname(HttpdConnData *connData);

#if HTTPD_SERVER
HttpdBuiltInUrl builtInUrls[]={
//	{"*", cgiRedirectApClientToHostname, "esp.nonet"},
	{"/", cgiIndex, "/index.html"},
	{"/counter", cgiCounter, NULL},
	{"/wifi/wifiscan.cgi", cgiIndex, NULL},
	{"/wifi/wifi.tpl", cgiIndex, tplWlan},
	{"/wifi/connect.cgi", cgiIndex, NULL},
	{"/wifi/connstatus.cgi", cgiIndex, NULL},
	{"/wifi/setmode.cgi", cgiIndex, NULL},

	{"/config", cgiIndex, NULL},
	{"/client", cgiIndex, NULL},
	{"/upgrade",cgiIndex, NULL},

//	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL} //end marker
};
#endif


/*
#if HTTPD_SERVER
HttpdBuiltInUrl builtInUrls[]={
	{"*", cgiRedirectApClientToHostname, "esp.nonet"},
	{"/", cgiRedirect, "/index.html"},
	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/connect.cgi", cgiWiFiConnect, NULL},
	{"/wifi/connstatus.cgi", cgiWiFiConnStatus, NULL},
	{"/wifi/setmode.cgi", cgiWiFiSetMode, NULL},

	{"/config", cgiEspApi, NULL},
	{"/client", cgiEspApi, NULL},
	{"/upgrade", cgiEspApi, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL} //end marker
};
#endif
*/

void Hardware_Init();
void plug_on_off_task();
void key_task();






/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
//	UART_SetBaudrate(UART0, 115200);
    printf("SDK version:%s\n", system_get_sdk_version());
    Hardware_Init();
    vTaskDelay(3000/portTICK_RATE_MS);
    //GPIO_OUTPUT(GPIO_Pin_12,0x01);
    GPIO_OUTPUT(GPIO_Pin_13,0x01);
    vTaskDelay(5000/portTICK_RATE_MS);
    if(GPIO_INPUT_GET(0)==0)
    {
    	os_printf("enter esp_platform**************************\r\n");
    	user_esp_platform_init();
    }
    else
    {
   // user_devicefind_start();
    //user_webserver_start();
    //captdnsInit();
    //espFsInit((void*)(webpages_espfs_start));
    	httpdInit(builtInUrls, 80);
    	xTaskCreate(plug_on_off_task,"plug_on_off_task",120,NULL,10,&xHandle_plug_on_off_task);
    }
}


/******************************************************************************
 * FunctionName : http_service
 * Description  : service of http
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void http_service()
{

}

void Hardware_Init()
{
/*
	struct softap_config ap_config;
	sprintf(ap_config.ssid,SSID);
	sprintf(ap_config.password,PWD);
	ap_config.authmode=AUTH_WPA_WPA2_PSK;
	ap_config.ssid_len=sizeof(SSID);
	ap_config.max_connection=2;
	wifi_set_opmode(STATIONAP_MODE);
	wifi_softap_set_config(&ap_config);
*/
	/*Init the GPIO of key*/
	GPIO_ConfigTypeDef GPIO_ConfigStructure;
	GPIO_ConfigStructure.GPIO_Mode=GPIO_Mode_Input;
	GPIO_ConfigStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_ConfigStructure.GPIO_Pullup=GPIO_PullUp_EN;
	gpio_config(&GPIO_ConfigStructure);

	/*Init LED*/
	GPIO_ConfigStructure.GPIO_Mode=GPIO_Mode_Output;
	//GPIO_ConfigStructure.GPIO_Pin=GPIO_Pin_12;
	GPIO_ConfigStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_ConfigStructure.GPIO_Pullup=GPIO_PullUp_EN;
	gpio_config(&GPIO_ConfigStructure);
	//GPIO_OUTPUT(GPIO_Pin_12,0x00);
	GPIO_OUTPUT(GPIO_Pin_13,0x00);

	/*Init relays*/
	GPIO_ConfigStructure.GPIO_Mode=GPIO_Mode_Output;
	//GPIO_ConfigStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_ConfigStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_ConfigStructure.GPIO_Pullup=GPIO_PullUp_EN;
	gpio_config(&GPIO_ConfigStructure);
	GPIO_OUTPUT(GPIO_Pin_14,0x00);
	//GPIO_OUTPUT(GPIO_Pin_2,0x00);
	
}

unsigned int counter=0;
void plug_on_off_task()
{
	counter=0;
	xTaskCreate(key_task,"key_task",120,NULL,6,NULL);
	while(1)
	{
#if PLUG_DEVICE
		GPIO_OUTPUT(GPIO_Pin_14,0x01);
		printf("status:0x01\r\n");
		vTaskDelay(on_time*1000/portTICK_RATE_MS);
		GPIO_OUTPUT(GPIO_Pin_14,0x00);
		printf("status:0x00\r\n");
		vTaskDelay(off_time*1000/portTICK_RATE_MS);
		counter++;
#endif
	}
}

char plug_on_off_task_status=1;
void key_task()
{
	char key_status=0;
	unsigned char key_counter=0;
#ifdef COM_DEBUG
	printf("enter key_task\r\n");
#endif
	while(1)
	{
		if(GPIO_INPUT_GET(0)==0)
		{
			key_status=1;
			if(key_counter<255)
			{
				key_counter++;
			}
		}
		if((GPIO_INPUT_GET(0)!=0)&&key_status==1)
		{
			if(key_counter<=60)
			{
				if(1==plug_on_off_task_status)
				{
					vTaskSuspend(xHandle_plug_on_off_task);
					GPIO_OUTPUT(GPIO_Pin_13,0x00);
					plug_on_off_task_status=0;
				}
				else
				{
					vTaskResume(xHandle_plug_on_off_task);
					GPIO_OUTPUT(GPIO_Pin_13,0x01);
					plug_on_off_task_status=1;
				}
				/*
				if(3!=eTaskGetState(xHandle_plug_on_off_task))
				{
					vTaskSuspend(xHandle_plug_on_off_task);
					GPIO_OUTPUT(GPIO_Pin_13,0x00);
				}
				else
				{
					vTaskResume(xHandle_plug_on_off_task);
					GPIO_OUTPUT(GPIO_Pin_13,0x01);
				}
				*/
			}
			else
			{

			}
			key_status=0;
			key_counter=0;
		}
		vTaskDelay(50/portTICK_RATE_MS);
	}
}
