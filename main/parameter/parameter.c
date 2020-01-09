#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "mylist.h"
#include "parameter.h"
#include "flashrw.h"

static const char *TAG = "PARA";
RPARA relay;

char* SetRelayFactory(cJSON *payload)
{
	cJSON *item = NULL;
	char *rjson = NULL;
	char *buf = NULL;

	item = cJSON_GetObjectItemCaseSensitive(payload,"RelayId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.RelayId,item->valuestring);
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"FwVer");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.FwVer,item->valuestring);
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"HwVer");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.HwVer,item->valuestring);
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Model");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.Model,item->valuestring);
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"MAC");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.MAC,item->valuestring);
	}

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* SetRelayAddress(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;

	item = cJSON_GetObjectItemCaseSensitive(payload,"Address");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(relay.Address,item->valuestring);
	}

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* AddValve(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	VELEM velem = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.ValveId,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"FwVer");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.FwVer,item->valuestring);
	}
	else
	{
		printf("FwVer error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"HwVer");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.HwVer,item->valuestring);
	}
	else
	{
		printf("HwVer error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Model");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.Model,item->valuestring);
	}
	else
	{
		printf("Model error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"MAC");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.MAC,item->valuestring);
	}
	else
	{
		printf("MAC error!\n");
		return NULL;
	}

	AddElemIntoVList(relay.pvlist,&velem,NULL);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* SetValveChildLock(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	VELEM velem = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.ValveId,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	GetElemFromVList(relay.pvlist,&velem);

	item = cJSON_GetObjectItemCaseSensitive(payload,"ChildLock");
	if(cJSON_IsNumber(item))
	{
		velem.ChildLock = item->valueint;
	}

	SetElemFromVList(relay.pvlist,&velem);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* SetValveManual(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	VELEM velem = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(velem.ValveId,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	GetElemFromVList(relay.pvlist,&velem);

	item = cJSON_GetObjectItemCaseSensitive(payload,"ManualEnable");
	if(cJSON_IsNumber(item))
	{
		velem.ManualEnable = item->valueint;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"ManualDuration");
	if(cJSON_IsNumber(item))
	{
		velem.ManualDuration = item->valueint;
	}

	SetElemFromVList(relay.pvlist,&velem);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* DelValve(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	char id[20] = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(id,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	DelElemFromVList(relay.pvlist,id);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* AddSchedule(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	char id[20] = {0};
	SELEM selem = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(id,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Id");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(selem.Id,item->valuestring);
	}
	else
	{
		printf("Id error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Enable");
	if(cJSON_IsNumber(item))
	{
		selem.Enable = item->valueint;
	}
	else
	{
		printf("Enable error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Duration");
	if(cJSON_IsNumber(item))
	{
		selem.Duration = item->valueint;
	}
	else
	{
		printf("Duration error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Time");
	if(cJSON_IsNumber(item))
	{
		selem.Time = item->valueint;
	}
	else
	{
		printf("Time error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Week");
	if(cJSON_IsNumber(item))
	{
		selem.Week = item->valueint;
	}
	else
	{
		printf("Week error!\n");
		return NULL;
	}

	AddElemIntoVListSchedule(relay.pvlist,id,&selem);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* SetSchedule(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	char id[20] = {0};
	SELEM selem = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(id,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Id");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(selem.Id,item->valuestring);
	}
	else
	{
		printf("Id error!\n");
		return NULL;
	}

	GetElemFromVListSchedule(relay.pvlist,id,&selem);

	item = cJSON_GetObjectItemCaseSensitive(payload,"Enable");
	if(cJSON_IsNumber(item))
	{
		selem.Enable = item->valueint;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Duration");
	if(cJSON_IsNumber(item))
	{
		selem.Duration = item->valueint;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Time");
	if(cJSON_IsNumber(item))
	{
		selem.Time = item->valueint;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Week");
	if(cJSON_IsNumber(item))
	{
		selem.Week = item->valueint;
	}

	SetElemFromVListSchedule(relay.pvlist,id,&selem);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* DelSchedule(cJSON *payload)
{
	cJSON *item = NULL;
	char* rjson = NULL;
	char *buf = NULL;
	char id[20] = {0};
	char sid[20] = {0};

	item = cJSON_GetObjectItemCaseSensitive(payload,"ValveId");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(id,item->valuestring);
	}
	else
	{
		printf("valve id error!\n");
		return NULL;
	}

	item = cJSON_GetObjectItemCaseSensitive(payload,"Id");
	if(cJSON_IsString(item) && (item->valuestring != NULL))
	{
		strcpy(sid,item->valuestring);
	}
	else
	{
		printf("Id error!\n");
		return NULL;
	}

	DelElemFromvListSchedule(relay.pvlist,id,sid);

	buf = DumpJsonFromRPara(&relay);
	SaveDate(buf,strlen(buf));
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* PrintRelay()
{
	char *buf = NULL;
	char* rjson = NULL;

	buf = DumpJsonFromRPara(&relay);
	ESP_LOGI(TAG, "buf:\n%s\n",buf);
	free(buf);

	rjson = (char*)malloc(10);
	if(rjson)
	{
		sprintf(rjson,"{\"ret\":\"ok\"}");
	}
	return rjson;
}

char* SetPara(char* json)
{
	cJSON *root = NULL;
	cJSON *type = NULL;
	cJSON *payload = NULL;
	char* rjson = NULL;
	int stype = -1;

	root = cJSON_Parse(json);
	if(root == NULL)
	{
		printf("json parse error!\n");
		return NULL;
	}

	type = cJSON_GetObjectItemCaseSensitive(root,"Type");
	if(cJSON_IsNumber(type))
	{
		stype = type->valueint;
	}
	payload = cJSON_GetObjectItemCaseSensitive(root,"Payload");
	if(!cJSON_IsObject(payload))
	{
		printf("payload is not object!\n");
		return NULL;
	}

	switch(stype)
	{
		case SET_RELAY_FACTORY:
			rjson = SetRelayFactory(payload);
			break;
		case SET_RELAY_ADDRESS:
			rjson = SetRelayAddress(payload);
			break;
		case ADD_VALVE:
			rjson = AddValve(payload);
			break;
		case SET_VALVE_CHILDLOCK:
			rjson = SetValveChildLock(payload);
			break;
		case SET_VALVE_MANUAL:
			rjson = SetValveManual(payload);
			break;
		case DEL_VALVE:
			rjson = DelValve(payload);
			break;
		case ADD_SCHEDULE:
			rjson = AddSchedule(payload);
			break;
		case SET_SCHEDULE:
			rjson = SetSchedule(payload);
			break;
		case DEL_SCHEDULE:
			rjson = DelSchedule(payload);
			break;
		case PRINT_RELAY:
			rjson = PrintRelay();
			break;
		case -1:
			printf("type error!\n");
			break;
	}
	cJSON_Delete(root);
	return rjson;
}

char* DumpRelayPara()
{
	return DumpJsonFromRPara(&relay); 
}

char* GetRelayId()
{
	return relay.RelayId;
}

void SetUserId(char *userid)
{
	strncpy(relay.UserId,userid,sizeof(relay.UserId));
}

void InitPara()
{
	char *buf = NULL;
	char *json = NULL;
	json = LoadDate();
	LoadRParaFromJson(&relay,json);
	buf = DumpJsonFromRPara(&relay);
	ESP_LOGI(TAG, "buf:\n%s\n",buf);
	free(buf);
	free(json);
}
