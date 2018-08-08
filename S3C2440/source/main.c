/**************************************************
**	�ļ���:main.c
**	�汾��:V 1.0
**	��Ȩ˵��:��Դ������Ƕ�Ƽ���д��ά����
**	�ļ�˵��:�Զ����������������ಿ��ִ����ɺ�
**		��ת�����ļ���main�����У���ɶ�uCOS-II�������ʼ���ȣ�
**		���������������������ʼ������á�
***************************************************/

#include "config.h"
#include "stats.h"
extern void Lcd_Display(void);

void Task_LCD(void *p);

/*****************����������Զ��������*****************/
OS_STK  MainTaskStk[MainTaskStkLengh];
OS_STK	Task0Stk [Task0StkLengh];       // Define the Task0 stack 
OS_STK	Task1Stk [Task1StkLengh];       // Define the Task1 stack 
OS_STK	Task2Stk [Task2StkLengh];       // Define the Task2 stack 

OS_EVENT *Semp;                         //Creat semp

U8 err;
int rYear, rMonth,rDay,rDayOfWeek,rHour,rMinute,rSecond;
//����һ�������ñ�����ʵ�ָ������һ������ʱ������Ҫ��ʼ����֮�������и�����ʱ����Ҫ���г�ʼ��
int user_task0_firstboot = 1;
int user_task1_firstboot = 1;
int user_task2_firstboot = 1;

int Main(int argc, char **argv)
{
	//��ʼ��Ŀ���
	TargetInit(); 

	//��ʼ��uCOS-II
	OSInit ();	 

	//��ʼ��ϵͳʱ��
	OSTimeSet(0);

	//����ϵͳ��ʼ����
	OSTaskCreate (MainTask,(void *)0, &MainTaskStk[MainTaskStkLengh - 1], MainTaskPrio);

	//��ʼ����
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

void Task0(void *pdata)				//����0����ӡCPUռ����
{
	
	while (1)
	{
		OSPrintf("\nEnter Task0\n");
		OSPrintf("CPU Usage: %d%%\n",OSCPUUsage); //��ӡCPUռ���ʣ�����ϵͳ����ʵ��

		OSTimeDly(OS_TICKS_PER_SEC);
	}
}

void Task1(void *pdata)				//����1������LED����������������ͬʱ����
{
	U16 task1Cnt=0;

	if(user_task1_firstboot == 1)
	{
		// RTC��ʼ��
		Rtc_Init();

		user_task1_firstboot = 0;
	}

	while (1)
	{
		task1Cnt++;
		OSPrintf("\nEnter Task1\n");
		OSPrintf("uC/OS Version:V%4.2f\n",(float)OSVersion()*0.01);//��ӡuC/OS�İ汾�� 

		//ʵ����ˮ��
		if((task1Cnt%5) == 0)
			rGPBDAT = 0x1E0;				//ȫ��
		else
			rGPBDAT = rGPBDAT - (0x10<<(task1Cnt%5));	//��ˮ��

		//����������
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
		//LCD ��ʼ��
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
		

		//��LCD�ϴ�ӡ���ڣ����ڣ�ʱ��
		Lcd_printf(0,65,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),0,"ʱ��:%4d-%02d-%02d ����%d  %02d:%02d:%02d\n",
        	      rYear, rMonth, rDay,rDayOfWeek, rHour, rMinute, rSecond);

		Lcd_printf(84,92,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),1," uC/OS2������ʾ");
		Lcd_printf(89,122,RGB( 0xFF,0xFF,0xFF),RGB( 0x00,0x00,0x00),0,"���� ������:%02d" , i);

		OSTimeDly(OS_TICKS_PER_SEC/5);
	}
}
