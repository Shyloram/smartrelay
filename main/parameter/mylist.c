#include "mylist.h"

//ScheduleList
SLIST* InitSList()
{
	SLIST* pslist = (SLIST *)malloc(sizeof(SLIST));
	if(!pslist)
	{
		printf("malloc error!\n");
		return NULL;
	}
	pslist->tail = NULL;
	pslist->maxlen = 5;
	pslist->cnt = 0;
	return pslist;
}

int ReleaseSList(SLIST* pslist)
{
	if(!pslist)
	{
		printf("pslist is NULL\n");
		return -1;
	}
	SELEM* pselem = pslist->tail;
	SELEM* pstmp;
	while(pselem)
	{
		pstmp = pselem->next;
		pselem->next = NULL;
		free(pselem);
		pselem = pstmp;
	}
	pslist->tail = NULL;
	pslist->cnt = 0;
	free(pslist);
	pslist = NULL;
	return 0;
}

int AddElemIntoSList(SLIST* pslist,SELEM* selem)
{
	if(!pslist || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}

	SELEM* pselem = pslist->tail;
	while(pselem)
	{
		if(!strcmp(selem->Id,pselem->Id))
		{
			break;
		}
		pselem = pselem->next;
	}
	if(!pselem)
	{
		if(pslist->cnt >= pslist->maxlen)
		{
			printf("list is FULL\n");
			return -2;
		}
		SELEM* pselem = (SELEM *)malloc(sizeof(SELEM));
		if(!pselem)
		{
			printf("malloc pselem error!\n");
			return -1;
		}
		strcpy(pselem->Id,selem->Id);
		pselem->Enable = selem->Enable;
		pselem->Duration = selem->Duration;
		pselem->Time = selem->Time;
		pselem->Week = selem->Week;
		pselem->next = pslist->tail;
		pslist->tail = pselem;
		pslist->cnt++;
	}
	else
	{
		printf("This id:%s is already exist in list.\n",selem->Id);
		return -1;
	}
	return 0;
}

int DelElemFromSList(SLIST* pslist,char* id)
{
	if(!pslist || !id)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	if(pslist->cnt == 0)
	{
		printf("list is Empty\n");
		return -2;
	}
	SELEM* pselem = pslist->tail;
	SELEM* prev = NULL; 
	while(pselem)
	{
		if(!strcmp(id,pselem->Id))
		{
			break;
		}
		prev = pselem;
		pselem = pselem->next;
	}
	if(pselem)
	{
		if(pselem == pslist->tail)
		{
			pslist->tail = pselem->next;
		}
		else
		{
			prev->next = pselem->next;
		}
		pselem->next = NULL;
		free(pselem);
		pselem = NULL;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	pslist->cnt--;
	return 0;
}

int GetElemFromSList(SLIST* pslist,SELEM* selem)
{
	if(!pslist || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	SELEM* pselem = pslist->tail;
	while(pselem)
	{
		if(!strcmp(selem->Id,pselem->Id))
		{
			break;
		}
		pselem = pselem->next;
	}
	if(pselem)
	{
		selem->Enable = pselem->Enable;
		selem->Duration = pselem->Duration;
		selem->Time = pselem->Time;
		selem->Week = pselem->Week;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",selem->Id);
		return -1;
	}
	return 0;
}

int SetElemFromSList(SLIST* pslist,SELEM* selem)
{
	if(!pslist || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	SELEM* pselem = pslist->tail;
	while(pselem)
	{
		if(!strcmp(selem->Id,pselem->Id))
		{
			break;
		}
		pselem = pselem->next;
	}
	if(pselem)
	{
		pselem->Enable = selem->Enable;
		pselem->Duration = selem->Duration;
		pselem->Time = selem->Time;
		pselem->Week = selem->Week;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",selem->Id);
		return -1;
	}
	return 0;
}

cJSON*  DumpJsonFromSList(SLIST* pslist)
{
	if(!pslist)
	{
		printf("pslist is NULL\n");
		return NULL;
	}
	cJSON *array = NULL;
	cJSON *fld = NULL;
	SELEM* pselem = pslist->tail;
	array = cJSON_CreateArray();
	while(pselem)
	{
		cJSON_AddItemToArray(array, fld = cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "ID", pselem->Id);
		cJSON_AddNumberToObject(fld, "EN", pselem->Enable);
		cJSON_AddNumberToObject(fld, "DU", pselem->Duration);
		cJSON_AddNumberToObject(fld, "TM", pselem->Time);
		cJSON_AddNumberToObject(fld, "WK", pselem->Week);
		pselem = pselem->next;
	}
	return array;
}

int LoadSListFromJson(SLIST* pslist,cJSON* json)
{
	if(!pslist || !json)
	{
		printf("pslist or json is NULL\n");
		return -1;
	}
	if(pslist->tail != NULL)
	{
		printf("slist is not empty! Can not load Json!\n");
		return -1;
	}
	cJSON *item = NULL;
	cJSON *fld = NULL;
	SELEM selem = {0};
	int i = 0;

	for(i = 0;i < cJSON_GetArraySize(json);i++)
	{
		item = cJSON_GetArrayItem(json,i);

		fld = cJSON_GetObjectItemCaseSensitive(item,"ID");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(selem.Id,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"EN");
		if(cJSON_IsNumber(fld))
		{
			selem.Enable = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"DU");
		if(cJSON_IsNumber(fld))
		{
			selem.Duration = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"TM");
		if(cJSON_IsNumber(fld))
		{
			selem.Time = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"WK");
		if(cJSON_IsNumber(fld))
		{
			selem.Week = fld->valueint;
		}

		AddElemIntoSList(pslist,&selem);
		memset(&selem,0,sizeof(SELEM));
	}
	return 0;
}

//ValveList
VLIST* InitVList()
{
	VLIST* pvlist = (VLIST *)malloc(sizeof(VLIST));
	if(!pvlist)
	{
		printf("malloc error!\n");
		return NULL;
	}
	pvlist->tail = NULL;
	pvlist->maxlen = 16;
	pvlist->cnt = 0;
	return pvlist;
}

int ReleaseVList(VLIST* pvlist)
{
	if(!pvlist)
	{
		printf("[%s]pvlist is NULL\n",__func__);
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	VELEM* pvtmp;
	while(pvelem)
	{
		pvtmp = pvelem->next;
		if(pvelem->pslist != NULL)
		{
			ReleaseSList(pvelem->pslist);
			pvelem->pslist = NULL;
		}
		pvelem->next = NULL;
		free(pvelem);
		pvelem = pvtmp;
	}
	pvlist->tail = NULL;
	pvlist->cnt = 0;
	free(pvlist);
	pvlist = NULL;
	return 0;
}

int AddElemIntoVList(VLIST* pvlist,VELEM* velem,cJSON* json)
{
	if(!pvlist || !velem)
	{
		printf("input arg is NULL\n");
		return -1;
	}

	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(velem->ValveId,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(!pvelem)
	{
		if(pvlist->cnt >= pvlist->maxlen)
		{
			printf("list is FULL\n");
			return -2;
		}
		VELEM* pvelem = (VELEM *)malloc(sizeof(VELEM));
		if(!pvelem)
		{
			printf("malloc pvelem error!\n");
			return -1;
		}
		strcpy(pvelem->ValveId,velem->ValveId);
		strcpy(pvelem->FwVer,velem->FwVer);
		strcpy(pvelem->HwVer,velem->HwVer);
		strcpy(pvelem->Model,velem->Model);
		strcpy(pvelem->MAC,velem->MAC);
		pvelem->ChildLock = 0;
		pvelem->ManualEnable = 0;
		pvelem->ManualDuration = 30;
		pvelem->pslist = InitSList();
		if(json)
		{
			LoadSListFromJson(pvelem->pslist,json);
		}
		pvelem->next = pvlist->tail;
		pvlist->tail = pvelem;
		pvlist->cnt++;
	}
	else
	{
	{
		printf("This id:%s is already exist in list.\n",velem->ValveId);
		return -1;
	}
	}
	return 0;
}

int AddElemIntoVListSchedule(VLIST* pvlist,char* id,SELEM* selem)
{
	if(!pvlist || !id || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(id,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		AddElemIntoSList(pvelem->pslist,selem);
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	return 0;
}

int DelElemFromVList(VLIST* pvlist,char* id)
{
	if(!pvlist || !id)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	if(pvlist->cnt == 0)
	{
		printf("list is Empty\n");
		return -2;
	}
	VELEM* pvelem = pvlist->tail;
	VELEM* prev = NULL; 
	while(pvelem)
	{
		if(!strcmp(id,pvelem->ValveId))
		{
			break;
		}
		prev = pvelem;
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		if(pvelem == pvlist->tail)
		{
			pvlist->tail = pvelem->next;
		}
		else
		{
			prev->next = pvelem->next;
		}
		if(pvelem->pslist != NULL)
		{
			ReleaseSList(pvelem->pslist);
			pvelem->pslist = NULL;
		}
		pvelem->next = NULL;
		free(pvelem);
		pvelem = NULL;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	pvlist->cnt--;
	return 0;
}

int DelElemFromvListSchedule(VLIST* pvlist,char* id,char* sid)
{
	if(!pvlist || !id || !sid)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(id,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		DelElemFromSList(pvelem->pslist,sid);
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	return 0;
}

int GetElemFromVList(VLIST* pvlist,VELEM* velem)
{
	if(!pvlist || !velem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(velem->ValveId,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		strcpy(velem->FwVer,pvelem->FwVer);
		strcpy(velem->HwVer,pvelem->HwVer);
		strcpy(velem->Model,pvelem->Model);
		strcpy(velem->MAC,pvelem->MAC);
		velem->ChildLock = pvelem->ChildLock;
		velem->ManualEnable = pvelem->ManualEnable;
		velem->ManualDuration = pvelem->ManualDuration;
		velem->pslist = pvelem->pslist;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",velem->ValveId);
		return -1;
	}
	return 0;
}

int GetElemFromVListSchedule(VLIST* pvlist,char* id,SELEM* selem)
{
	if(!pvlist || !id || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(id,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		GetElemFromSList(pvelem->pslist,selem);
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	return 0;
}

int SetElemFromVList(VLIST* pvlist,VELEM* velem)
{
	if(!pvlist || !velem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(velem->ValveId,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		pvelem->ChildLock = velem->ChildLock;
		pvelem->ManualEnable = velem->ManualEnable;
		pvelem->ManualDuration = velem->ManualDuration;
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",velem->ValveId);
		return -1;
	}
	return 0;
}

int SetElemFromVListSchedule(VLIST* pvlist,char* id,SELEM* selem)
{
	if(!pvlist || !id || !selem)
	{
		printf("input arg is NULL\n");
		return -1;
	}
	VELEM* pvelem = pvlist->tail;
	while(pvelem)
	{
		if(!strcmp(id,pvelem->ValveId))
		{
			break;
		}
		pvelem = pvelem->next;
	}
	if(pvelem)
	{
		SetElemFromSList(pvelem->pslist,selem);
	}
	else
	{
		printf("Can not found id:%s elem from list.\n",id);
		return -1;
	}
	return 0;
}

cJSON* DumpJsonFromVList(VLIST* pvlist)
{
	if(!pvlist)
	{
		printf("[%s]pvlist is NULL\n",__func__);
		return NULL;
	}
	cJSON *array = NULL;
	cJSON *fld = NULL;
	cJSON *sch = NULL;
	VELEM* pvelem = pvlist->tail;
	array = cJSON_CreateArray();
	while(pvelem)
	{
		cJSON_AddItemToArray(array, fld = cJSON_CreateObject());
		cJSON_AddStringToObject(fld, "VID", pvelem->ValveId);
		cJSON_AddStringToObject(fld, "FV", pvelem->FwVer);
		cJSON_AddStringToObject(fld, "HV", pvelem->HwVer);
		cJSON_AddStringToObject(fld, "MOD", pvelem->Model);
		cJSON_AddStringToObject(fld, "MAC", pvelem->MAC);
		cJSON_AddNumberToObject(fld, "CLK", pvelem->ChildLock);
		cJSON_AddNumberToObject(fld, "MEN", pvelem->ManualEnable);
		cJSON_AddNumberToObject(fld, "MDU", pvelem->ManualDuration);
		sch = DumpJsonFromSList(pvelem->pslist);
		cJSON_AddItemToObject(fld, "SCH", sch);
		pvelem = pvelem->next;
	}
	return array;
}

int LoadVListFromJson(VLIST* pvlist,cJSON* json)
{
	if(!pvlist || !json)
	{
		printf("[%s]pvlist or json is NULL\n",__func__);
		return -1;
	}
	if(pvlist->tail != NULL)
	{
		printf("slist is not empty! Can not load Json!\n");
		return -1;
	}
	cJSON *item = NULL;
	cJSON *fld = NULL;
	VELEM velem = {0};
	int i = 0;

	for(i = 0;i < cJSON_GetArraySize(json);i++)
	{
		item = cJSON_GetArrayItem(json,i);

		fld = cJSON_GetObjectItemCaseSensitive(item,"VID");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(velem.ValveId,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"FV");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(velem.FwVer,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"HV");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(velem.HwVer,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"MOD");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(velem.Model,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"MAC");
		if(cJSON_IsString(fld) && (fld->valuestring != NULL))
		{
			strcpy(velem.MAC,fld->valuestring);
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"CLK");
		if(cJSON_IsNumber(fld))
		{
			velem.ChildLock = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"MEN");
		if(cJSON_IsNumber(fld))
		{
			velem.ManualEnable = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"MDU");
		if(cJSON_IsNumber(fld))
		{
			velem.ManualDuration = fld->valueint;
		}

		fld = cJSON_GetObjectItemCaseSensitive(item,"SCH");
		if(cJSON_IsArray(fld))
		{
			AddElemIntoVList(pvlist,&velem,fld);
		}
		memset(&velem,0,sizeof(VELEM));
	}
	return 0;
}

//RelayPara
char* DumpJsonFromOnlyRPara(RPARA* prpara)
{
	if(!prpara)
	{
		printf("prpara is NULL\n");
		return NULL;
	}
	cJSON *root = NULL;
	char  *json = NULL;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "RID", prpara->RelayId);
	cJSON_AddStringToObject(root, "UID", prpara->UserId);
	cJSON_AddStringToObject(root, "FV", prpara->FwVer);
	cJSON_AddStringToObject(root, "HV", prpara->HwVer);
	cJSON_AddStringToObject(root, "MOD", prpara->Model);
	cJSON_AddStringToObject(root, "MAC", prpara->MAC);
	cJSON_AddStringToObject(root, "ADD", prpara->Address);
	json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return json;
}

char* DumpJsonFromRPara(RPARA* prpara)
{
	if(!prpara)
	{
		printf("prpara is NULL\n");
		return NULL;
	}
	cJSON *root = NULL;
	cJSON *fld = NULL;
	char  *json = NULL;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "RID", prpara->RelayId);
	cJSON_AddStringToObject(root, "UID", prpara->UserId);
	cJSON_AddStringToObject(root, "FV", prpara->FwVer);
	cJSON_AddStringToObject(root, "HV", prpara->HwVer);
	cJSON_AddStringToObject(root, "MOD", prpara->Model);
	cJSON_AddStringToObject(root, "MAC", prpara->MAC);
	cJSON_AddStringToObject(root, "ADD", prpara->Address);
	fld = DumpJsonFromVList(prpara->pvlist);
	cJSON_AddItemToObject(root, "VL", fld);
	json = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return json;
}

int LoadRParaFromJson(RPARA* prpara,char* json)
{
	if(!prpara || !json)
	{
		printf("prpara or json is NULL\n");
		return -1;
	}
	cJSON *root = NULL;
	cJSON *fld = NULL;

	root = cJSON_Parse(json);
	if(root == NULL)
	{
		printf("json parse error!\n");
		return -1;
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"RID");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->RelayId,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"UID");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->UserId,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"FV");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->FwVer,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"HV");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->HwVer,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"MOD");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->Model,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"MAC");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->MAC,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"ADD");
	if(cJSON_IsString(fld) && (fld->valuestring != NULL))
	{
		strcpy(prpara->Address,fld->valuestring);
	}

	fld = cJSON_GetObjectItemCaseSensitive(root,"VL");
	if(cJSON_IsArray(fld))
	{
		if(prpara->pvlist == NULL)
		{
			prpara->pvlist = InitVList();
		}
		LoadVListFromJson(prpara->pvlist,fld);
	}
	cJSON_Delete(root);
	return 0;
}
