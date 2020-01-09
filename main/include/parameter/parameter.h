#ifndef __PARAMETER__
#define __PARAMETER__
#include "mylist.h"

enum __TYPE_SET_PARA_
{
	SET_RELAY_FACTORY = 0,
	SET_RELAY_ADDRESS,//1
	ADD_VALVE,//2
	SET_VALVE_CHILDLOCK,//3
	SET_VALVE_MANUAL,//4
	DEL_VALVE,//5
	ADD_SCHEDULE,//6
	SET_SCHEDULE,//7
	DEL_SCHEDULE,//8
	PRINT_RELAY//9
};

//network interface
char* SetRelayFactory(cJSON *payload);
char* SetRelayAddress(cJSON *payload);
char* AddValve(cJSON *payload);
char* SetValveChildLock(cJSON *payload);
char* SetValveManual(cJSON *payload);
char* DelValve(cJSON *payload);
char* AddSchedule(cJSON *payload);
char* SetSchedule(cJSON *payload);
char* DelSchedule(cJSON *payload);
char* PrintRelay();
char* SetPara(char* json);

//Internal interface
char* DumpRelayPara();
char* GetRelayId();
void SetUserId(char* userid);
#endif
