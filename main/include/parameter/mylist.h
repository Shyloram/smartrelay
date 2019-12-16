#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cJSON.h"

typedef struct ScheduleElem
{
	char Id[17];
	char Enable;
	int Duration;
	int Time;
	int Week;
	struct ScheduleElem *next;
}SELEM;

typedef struct ScheduleList
{
	SELEM *tail;
	int maxlen;
	int cnt;
}SLIST;

typedef struct ValveElem
{
	char ValveId[17];
	char FwVer[12];
	char HwVer[12];
	char Model[10];
	char MAC[24];
	char ChildLock;
	char ManualEnable;
	int ManualDuration;
	SLIST *pslist;
	struct ValveElem *next;
}VELEM;

typedef struct ValveList
{
	VELEM *tail;
	int maxlen;
	int cnt;
}VLIST;

typedef struct RelayPara
{
	char RelayId[17];
	char UserId[17];
	char FwVer[12];
	char HwVer[12];
	char Model[10];
	char MAC[18];
	char Address[10];
	VLIST *pvlist;
}RPARA;

//ScheduleList
SLIST* InitSList();
int ReleaseSList(SLIST* pslist);
int AddElemIntoSList(SLIST* pslist,SELEM* selem);
int DelElemFromSList(SLIST* pslist,char* id);
int GetElemFromSList(SLIST* pslist,SELEM* selem);
int SetElemFromSList(SLIST* pslist,SELEM* selem);
char* DumpJsonFromSList(SLIST* pslist);
int LoadSListFromJson(SLIST* pslist,char* json);

//ValveList
VLIST* InitVList();
int ReleaseVList(VLIST* pvlist);
int AddElemIntoVList(VLIST* pvlist,VELEM* velem,char* json);
int DelElemFromVList(VLIST* pvlist,char* id);
int GetElemFromVList(VLIST* pvlist,VELEM* velem);
int SetElemFromVList(VLIST* pvlist,VELEM* velem);
int AddElemIntoVListSchedule(VLIST* pvlist,char* id,SELEM* selem);
int DelElemFromvListSchedule(VLIST* pvlist,char* id,char* sid);
int GetElemFromVListSchedule(VLIST* pvlist,char* id,SELEM* selem);
int SetElemFromVListSchedule(VLIST* pvlist,char* id,SELEM* selem);
char* DumpJsonFromVList(VLIST* pvlist);
int LoadVListFromJson(VLIST* pvlist,char* json);

//RelayPara
char* DumpJsonFromRPara(RPARA* prpara);
int LoadRParaFromJson(RPARA* prpara,char* json);
