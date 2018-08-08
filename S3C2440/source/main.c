/**************************************************
**	文件名:main.c
**	版本号:V 1.0
**	版权说明:本源码由天嵌科技编写并维护。
**	文件说明:自定义的主函数，当汇编部分执行完成后，
**		跳转到本文件的main函数中，完成对uCOS-II的任务初始化等，
**		最后调用任务管理器，并开始任务调用。
***************************************************/

#include "config.h"
#include "stats.h"
extern void Lcd_Display(void);

void Task_LCD(void *p);

/*****************【任务相关自定义变量】*****************/
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk [Task0StkLengh];       // Define the Task0 stack 
OS_STK	Task1Stk [Task1StkLengh];       // Define the Task1 stack 
OS_STK	Task2Stk [Task2StkLengh];       // Define the Task2 stack 

OS_EVENT *Semp;                         //Creat semp

U8 err;
int rYear, rMonth,rDay,rDayOfWeek,rHour,rMinute,rSecond;
//建立一个任务用变量，实现该任务第一次运行时，就需要初始化，之后再运行该任务时则不需要进行初始化
int user_task0_firstboot = 1;
int user_task1_firstboot = 1;
int user_task2_firstboot = 1;

int Main(int argc, char **argv)
{
	//初始化目标板
	TargetInit(); 

	//初始化uCOS-II
	OSInit ();	 

	//初始化系统时基
	OSTimeSet(0);

	//创建系统初始任务
	OSTaskCreate (MainTask,(void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);

	//开始任务
	OSStart ();

	return 0;
}
//extern struct stats_ lwip_stats;
void MainTask(void *pdata) //Main Task create taks0 and task1
{
	#if OS_CRITICAL_METHOD == 3		/* Allocate storage for CPU status register */
		OS_CPU_SR  cpu_sr;
	#endif
	OS_ENTER_CRITICAL();

	Timer0Init();				//initial timer0 for ucos time tick
	ISRInit();				//initial interrupt prio or enable or disable

	OS_EXIT_CRITICAL();

	OSPrintfInit();				//use task to print massage to Uart 

	OSStatInit();
	lwip_main_task();
	printf("\nlink xmit: %d recv: %d fw: %d drop: %d ",lwip_stats.link.xmit,lwip_stats.link.recv,lwip_stats.link.fw,lwip_stats.link.drop);
	printf("chkerr: %d lenerr:%d memerr: %d rterr: %d ",lwip_stats.link.chkerr,lwip_stats.link.lenerr,lwip_stats.link.memerr,lwip_stats.link.rterr);
	printf("proterr: %d opterr: %d err: %d cachehit: %d",lwip_stats.link.proterr,lwip_stats.link.opterr,lwip_stats.link.err,lwip_stats.link.cachehit);
	//OSTaskCreate (lwip_main_task,(void *)0, &lwip_main_taskStk[lwip_main_taskStkLengh - 1], Task0Prio);	
	//OSTaskCreate (Task0,(void *)0, &Task0Stk[Task0StkLengh - 1], Task0Prio);	
	//OSTaskCreate (Task1,(void *)0, &Task1Stk[Task1StkLengh - 1], Task1Prio);	
	OSTaskCreate (Task2,(void *)0, &Task2Stk[Task2StkLengh - 1], Task2Prio);	 
	while(1)
	{
		//OSPrintf("\nEnter Main Task\n");
		OSTimeDly(OS_TICKS_PER_SEC);
	}
}

void Task0(void *pdata)				//任务0，打印CPU占用率
{
	
	while (1)
	{
		OSPrintf("\nEnter Task0\n");
		OSPrintf("CPU Usage: %d%%\n",OSCPUUsage); //打印CPU占用率，调用系统变量实现

		OSTimeDly(OS_TICKS_PER_SEC);
	}
}

void Task1(void *pdata)				//任务1，控制LED灯亮，开启蜂鸣器同时鸣叫
{
	U16 task1Cnt=0;

	if(user_task1_firstboot == 1)
	{
		// RTC初始化
		Rtc_Init();

		user_task1_firstboot = 0;
	}

	while (1)
	{
		task1Cnt++;
		OSPrintf("\nEnter Task1\n");
		OSPrintf("uC/OS Version:V%4.2f\n",(float)OSVersion()*0.01);//打印uC/OS的版本号 

		//实现流水灯
		if((task1Cnt%5) == 0)
			rGPBDAT = 0x1E0;				//全灭
		else
			rGPBDAT = rGPBDAT - (0x10<<(task1Cnt%5));	//流水灯

		//蜂鸣器鸣叫
		Beep(3000, 60);
		Beep(2500, 60);
		Beep(2000, 60);
		Beep(1500, 60);
		Beep(1000, 60);
		Beep(900, 60);
		OSTimeDly(OS_TICKS_PER_SEC*5);
	}
}

void Task2(void *pdata)
{
	unsigned int i, x, m, n, k, y;
	int tmp,key;         

	int width = 10;
	int height = 100;

	if(user_task2_firstboot == 1)
	{
		//LCD 初始化
		Lcd_Display();

		user_task2_firstboot = 0;
	}

	while(1)
	{
		i++;
		if(i>99)i=0;

		if(rBCDYEAR == 0x99)
			rYear = 1999;
		else
			rYear    = (2000 + rBCDYEAR);
			rMonth   = FROM_BCD(rBCDMON & 0x1f);
			rDay		= FROM_BCD(rBCDDAY & 0x03f);
			rDayOfWeek = rBCDDATE - 1;
			rHour    = FROM_BCD(rBCDHOUR & 0x3f);
			rMinute     = FROM_BCD(rBCDMIN & 0x7f);
			rSecond     = FROM_BCD(rBCDSEC & 0x7f);

		OSTimeDly( 5 );
		//OSPrintf("\nEnter Task2\n");	
		

		//在LCD上打印日期，星期，时间
		Lcd_printf(0,65,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),0,"时间:%4d-%02d-%02d 星期%d  %02d:%02d:%02d\n",
        	      rYear, rMonth, rDay,rDayOfWeek, rHour, rMinute, rSecond);

		Lcd_printf(84,92,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),1," uC/OS2任务演示");
		Lcd_printf(89,122,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),0,"任务 计数中:%02d" , i);

		OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}

