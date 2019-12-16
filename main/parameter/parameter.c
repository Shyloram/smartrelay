#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "mylist.h"

#define PARA_FILE     "/spiffs/RelayPara.json"
static const char *TAG = "PARA";
char DefauleJson[] = "{\"RelayId\":\"Defaule\",\"UserId\":\"Defaule\",\"FwVer\":\"Defaule\",\"HwVer\":\"Defaule\",\"Model\":\"Defaule\",\"MAC\":\"aa:bb:cc:dd:ee:ff\",\"Address\":\"Defaule\",\"ValveList\":[]}";
RPARA relay;

int saveASfile(char* data,int len)
{
	FILE *fp;
	int ret = 0;

	fp = fopen(PARA_FILE,"w");
	if(fp == NULL)
	{
		perror("fopen error!");
		return -1;
	}
	ret = fwrite(data,1,len,fp);
	if(ret != len)
	{
		ESP_LOGE(TAG, "ret:%d,len%d\n",ret,len);
	}
	fclose(fp);
	return 0;
}

char* read_file()
{
	FILE *file = NULL;
	long length = 0;
	char *content = NULL;
	struct stat st;

	/* stat file */
	if (!stat(PARA_FILE, &st) == 0) 
	{
		goto cleanup;
	}

	/* open in read binary mode */
	file = fopen(PARA_FILE, "r");
	if (file == NULL)
	{
		goto cleanup;
	}

	/* get the length */
	if (fseek(file, 0, SEEK_END) != 0)
	{
		goto cleanup;
	}
	length = ftell(file);
	if (length < 0)
	{
		goto cleanup;
	}
	if (fseek(file, 0, SEEK_SET) != 0)
	{
		goto cleanup;
	}

	/* allocate content buffer */
	content = (char*)malloc((size_t)length + sizeof(""));
	if (content == NULL)
	{
		goto cleanup;
	}

	/* read the file into memory */
	long read_chars = fread(content, sizeof(char), (size_t)length, file);
	if ((long)read_chars != length)
	{
		free(content);
		content = NULL;
		goto cleanup;
	}
	content[read_chars] = '\0';

cleanup:
	if (file != NULL)
	{
		fclose(file);
	}
	if(!content)
	{
		content = DefauleJson;
	}

	return content;
}

void InitSpiffs()
{
	size_t total = 0,used = 0;

	ESP_LOGI(TAG, "Initializing SPIFFS");
	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret != ESP_OK) 
	{
		if (ret == ESP_FAIL) 
		{
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND) 
		{
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}

	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) 
	{
		 ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	}
}

void InitPara()
{
	char *buf = NULL;
	char *json = NULL;

	InitSpiffs();

	json = read_file();
	LoadRParaFromJson(&relay,json);

	buf = DumpJsonFromRPara(&relay);
	ESP_LOGI(TAG, "buf:\n%s\n",buf);
	saveASfile(buf,strlen(buf));
	free(buf);
}
