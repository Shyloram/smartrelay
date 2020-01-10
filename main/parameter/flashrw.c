#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "esp_spi_flash.h"
#include "esp_log.h"

static const char *TAG = "FLASHRW";
#define INFO_SEC 144
#define RELAY_DATA_SEC 145
#define RELAY_DATA_SEC_MAX 150
#define FLASH_SEC_SIZE 4096
#define GET_ALIGN_STRING_LEN(len)    ((len + 3) & ~3)
char DefauleJson[] = "{\"RID\":\"RYER191127000001\",\"UID\":\"Defaule\",\"FV\":\"V0.0.00.001\",\"HV\":\"V0.0.00.001\",\"MOD\":\"unicon\",\"MAC\":\"aa:bb:cc:dd:ee:ff\",\"ADD\":\"Defaule\",\"VL\":[]}";

int SaveDate(char* data, int len)
{
	int slen = GET_ALIGN_STRING_LEN(len);
	int rlen = 0;
	int nlen = 0;
	int sec_num = 0;
	char head[4] = {'P','R','P','R'};
	char src_buf[1024] = {};

	if(data == NULL || len == 0)
	{
		ESP_LOGE(TAG,"input data is NULL \n");
		return -1;
	}

	sec_num = RELAY_DATA_SEC;

	spi_flash_erase_sector(INFO_SEC);
	spi_flash_write(INFO_SEC * FLASH_SEC_SIZE, head, sizeof(head));
	spi_flash_write(INFO_SEC * FLASH_SEC_SIZE + 4, &slen, sizeof(int));

	spi_flash_erase_sector(sec_num);
	while(slen)
	{
		//printf("len:%d,slen:%d,rlen:%d,nlen:%d,sec_num:%d\n",len,slen,rlen,nlen,sec_num);
		if(slen >= 1024)
		{
			spi_flash_write(sec_num * FLASH_SEC_SIZE + rlen, data + nlen, 1024);
			slen -= 1024;
			nlen += 1024;
			rlen += 1024;
		}
		else
		{
			strncpy(src_buf,data + nlen,slen);
			spi_flash_write(sec_num * FLASH_SEC_SIZE + rlen, src_buf, 1024);
			slen = 0;
		}
		if(rlen == 4096)
		{
			rlen = 0;
			sec_num++;
			spi_flash_erase_sector(sec_num);
		}
		if(sec_num == RELAY_DATA_SEC_MAX)
		{
			ESP_LOGE(TAG,"it is too long!\n");
			return -1;
		}
	}
	//printf("len:%d,slen:%d,rlen:%d,nlen:%d,sec_num:%d\n",len,slen,rlen,nlen,sec_num);
	return 0;
}

char* LoadDate()
{
	char *pdata = NULL;
	char head[4] = {};
	int len = 0;
	int rlen = 0;
	int nlen = 0;
	int sec_num = 0;

	sec_num = RELAY_DATA_SEC;

	spi_flash_read(INFO_SEC * FLASH_SEC_SIZE, head, sizeof(head));
	spi_flash_read(INFO_SEC * FLASH_SEC_SIZE + 4, &len, sizeof(head));
	if(strncmp(head,"PRPR",4))
	{
		ESP_LOGW(TAG,"PARA not init, init now!\n");
		pdata = DefauleJson;
		return pdata;
	}

	ESP_LOGI(TAG,"LoadData len :%d\n",len);
	pdata = (char *)malloc(len + 1);
	if(!pdata)
	{
		ESP_LOGE(TAG,"malloc error\n");
		abort();
	}
	*(pdata+len) = 0;
	while(len)
	{
		//printf("len:%d,rlen:%d,nlen:%d,sec_num:%d\n",len,rlen,nlen,sec_num);
		if(len >= 1024)
		{
			spi_flash_read(sec_num * FLASH_SEC_SIZE + rlen, pdata + nlen, 1024);
			len -= 1024;
			rlen += 1024;
			nlen += 1024;
		}
		else
		{
			spi_flash_read(sec_num * FLASH_SEC_SIZE + rlen, pdata + nlen, len);
			len = 0;
		}
		if(rlen == 4096)
		{
			rlen = 0;
			sec_num++;
		}
		if(sec_num == RELAY_DATA_SEC_MAX)
		{
			ESP_LOGE(TAG,"it is too long!\n");
			return NULL;
		}
	}
	//printf("len:%d,rlen:%d,nlen:%d,sec_num:%d\n",len,rlen,nlen,sec_num);
	return pdata;
}
