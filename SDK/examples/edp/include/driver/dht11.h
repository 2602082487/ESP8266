#ifndef _DHT11_H
#define _DHT11_H


#define RETURN_OK						(0)
#define RETURN_ERR						(-1)

#define LOGIC_LL						(0)
#define LOGIC_HL						(1)

#define HUM_IO_NUM						(5)
#define HUM_IO_PIN						(GPIO_Pin_5)
#define HUM_DATA_SIZE					(4)

#define MAX_U16							(0xFFFF)




void DHT11_init();
int32 DHT11_read_temp_hum(uint8 *buf, uint32 len);



#endif
