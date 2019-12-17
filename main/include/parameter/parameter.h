#ifndef __PARAMETER__
#define __PARAMETER__
#include "mylist.h"

enum __TYPE_SET_PARA_
{
	SET_RELAY_FACTORY = 0,
	SET_RELAY_USERID,//1
	SET_RELAY_ADDRESS,//2
	ADD_VALVE,//3
	SET_VALVE_CHILDLOCK,//4
	SET_VALVE_MANUAL,//5
	DEL_VALVE,//6
	ADD_SCHEDULE,//7
	SET_SCHEDULE,//8
	DEL_SCHEDULE,//9
	PRINT_RELAY//10
};

char* SetRelayFactory(cJSON *payload);
char* SetRelayUserid(cJSON *payload);
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
#endif
