#ifndef __UART_915__
#define __UART_915__

typedef struct
{
	char strHead[2];
	char sourceId[17];
	char destineId[17];
	char type;
	char cmd;
	char contentLen;
	char content[8];
}MSG915;

typedef enum
{
	TYPE_VALVE = 0X00,
	TYPE_MAX
}SUBTYPE915;

typedef enum
{
	CMD_SET_CHILDLOCK = 0X00,
	CMD_MAX
}SUBCMD915;

int uart_set_valve_childlock(char* valveid,char enable);
#endif
