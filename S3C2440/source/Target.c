/**************************************************
**	文件名:Target.c
**	版本号:V 1.0
**	文件说明:初始化S3C2440，设置系统运行时钟，初始化端口，MMU，串口等
***************************************************/

#include "config.h"

void TargetInit(void)
{
	int i;
	U8 key;
	U32 mpll_val=0;
	#if ADS10   
	__rt_lib_init(0,0); //for ADS 1.0
 
	#endif
    
	i = 2 ;	//use 400M!
		
	switch ( i ) {
	case 0:	//200
		key = 12;
		mpll_val = (92<<12)|(4<<4)|(1);
		break;
	case 1:	//300
		key = 14;
		mpll_val = (67<<12)|(1<<4)|(1);
		break;
	case 2:	//400
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	case 3:	//440!!!
		key = 14;
		mpll_val = (102<<12)|(1<<4)|(1);
		break;
	default:
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	}
	
	//init FCLK=400M
	ChangeClockDivider(key, 12);    
	ChangeMPllValue((mpll_val>>12)&0xff, (mpll_val>>4)&0x3f, mpll_val&3);

	//MMU_EnableICache();
	//MMU_EnableDCache();
    
	MMU_DisableICache();
	MMU_DisableDCache();
 	Port_Init();
	MMU_Init();

	Delay(0);
	Uart_Init(0,115200);
	Uart_Select(0);
	Uart_SendString("Board init complete.\n");
}