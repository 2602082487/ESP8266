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
#include "EdpKit.h"
#include "driver/gpio.h"
#include "user_task.h"


xTaskHandle Handle_network_task;
xTaskHandle Handle_ping_task;
xTaskHandle Handle_recv_task;
xTaskHandle Handle_send_task;
xTaskHandle Handle_light_task;
xTaskHandle Handle_dht11_task;
xTaskHandle Handle_smartconfig_task;
xSemaphoreHandle wifi_sta;
xSemaphoreHandle driver;
xQueueHandle send_queue;



/******************************************************************************
 * FunctionName : GPIO_Init
 * Description  : Init GPIO
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void GPIO_Init()
{
	GPIO_ConfigTypeDef GPIO_ConfigInitStructure;
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	GPIO_ConfigInitStructure.GPIO_IntrType=GPIO_PIN_INTR_DISABLE;
	GPIO_ConfigInitStructure.GPIO_Mode=GPIO_Mode_Input;
	GPIO_ConfigInitStructure.GPIO_Pin=GPIO_Pin_4;
	GPIO_ConfigInitStructure.GPIO_Pullup=GPIO_PullUp_EN;
	gpio_config(&GPIO_ConfigInitStructure);
	
}




/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
    printf("SDK version:%s\n", system_get_sdk_version());
    GPIO_Init();
    vSemaphoreCreateBinary(wifi_sta);
    vSemaphoreCreateBinary(driver);
    send_queue=xQueueCreate(5,sizeof(EdpPacket *));
    xTaskCreate(smartconfig_task,"smartconfig_task",256,NULL,10,&Handle_smartconfig_task);
	xTaskCreate(network_task, "network_task", 200, NULL, 9, &Handle_network_task);
	xTaskCreate(send_task,"send_task",400,NULL,8,&Handle_send_task);
}



