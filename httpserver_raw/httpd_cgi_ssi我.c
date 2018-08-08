#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "config.h"
//#include "led.h"
//#include "pcf8574.h"
//#include "adc.h"
//#include "rtc.h"
//#include "lcd.h"
#include <string.h>
#include <stdlib.h>
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F4&F7开发板
//http 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/8/5
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//改为需要的TQ2440的版本
////////////////////////////////////////////////////////////////////////////////// 	   
 

#define NUM_CONFIG_CGI_URIS	2  //CGI的URI数量
#define NUM_CONFIG_SSI_TAGS	4  //SSI的TAG数量
//控制LED和BEEP的CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[]);
int flag1=0;
int flag2=0;
static const char *ppcTAGs[]=  //SSI的Tag
{
	"h", //时间
	"y", //日期
	"w",//电阻值
	"c"//Cpu占用率
	//"L"//lwip print
	
};

static const tCGI ppcURLs[]= //cgi程序
{
	{"/leds.cgi",LEDS_CGI_Handler},
	{"/beep.cgi",BEEP_CGI_Handler},
};


//找到led的索引号
//当web客户端请求浏览器的时候,使用此函数被CGI handler调用
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //返回iLOOP
		}
	}
	return (-1);
}
//SSIHandler中需要用到的处理变阻器cpu占用率读数值的函数
void CPU_Handler(char *pcInsert)
{
	if(OSCPUUsage<100)
		*pcInsert = (char)(OSCPUUsage/100 +0x30);
	else
		*pcInsert = (char)(' ' +0x30);
	*(pcInsert+1) = (char)(OSCPUUsage/10 +0x30);
	*(pcInsert+2) = (char)(OSCPUUsage%10 +0x30);
}
//SSIHandler中需要用到的处理变阻器读数值的函数
void Temperate_Handler(char *pcInsert)
{
    
    char Digit1=0, Digit2=0, Digit3=0, Digit4=0; 
    U32 ADCVal = 0;        

    //获取ADC的值
	ADCVal =ReadAdc(2); //获取ADC1_CH2的电压值
	printf("ADCVal:%d",ADCVal);		
    //转换为电压 ADCVval * 0.8mv
    ADCVal = (U32)(ADCVal * 0.8);  
    Digit1= ADCVal/1000;
    Digit2= (ADCVal-(Digit1*1000))/100 ;
    Digit3= (ADCVal-((Digit1*1000)+(Digit2*100)))/10;
    Digit4= ADCVal -((Digit1*1000)+(Digit2*100)+ (Digit3*10));
        
    //准备添加到html中的数据
    *pcInsert       = (char)(Digit1+0x30);
    *(pcInsert + 1) = (char)(Digit2+0x30);
    *(pcInsert + 2) = (char)(Digit3+0x30);
    *(pcInsert + 3) = (char)(Digit4+0x30);
}
//SSIHandler中需要用到的处理RTC时间的函数
void RTCTime_Handler(char *pcInsert)
{
	int rHour, rMinute, rSecond;
	rHour = FROM_BCD(rBCDHOUR & 0x3f);
	rMinute = FROM_BCD(rBCDMIN & 0x7f);
	rSecond = FROM_BCD(rBCDSEC & 0x7f);
	*pcInsert = 	(char)((rHour /10) + 0x30);
	*(pcInsert+1) = (char)((rHour%10) + 0x30);
	*(pcInsert+2) = ':';
	*(pcInsert+3) = (char)((rMinute /10) + 0x30);
	*(pcInsert+4) = (char)((rMinute %10) + 0x30);
	*(pcInsert+5) = ':';
	*(pcInsert+6) = (char)((rSecond /10) + 0x30);
	*(pcInsert+7) = (char)((rSecond %10) + 0x30);
}

//SSIHandler中需要用到的处理RTC日期的函数
void RTCdate_Handler(char *pcInsert)
{
	int rYear, rMonth, rDay, rDayOfWeek ;
	if (rBCDYEAR == 0x99)
		rYear = 1999;
	else
		rDayOfWeek = rBCDDATE - 1;
		rYear = (2000 + rBCDYEAR);
	rMonth = FROM_BCD(rBCDMON & 0x1f);
	rDay = FROM_BCD(rBCDDAY & 0x03f);
	*pcInsert = '2';
	*(pcInsert+1) = '0';
	*(pcInsert+2) = (char)((rYear /10)%10 + 0x30);
	*(pcInsert+3) = (char)((rYear %10) + 0x30);
	*(pcInsert+4) = '-';
	*(pcInsert+5) = (char)((rMonth /10)%10 + 0x30);
	*(pcInsert+6) = (char)((rMonth %10) + 0x30);
	*(pcInsert+7) = '-';
	*(pcInsert+8) = (char)((rDay /10)%10 + 0x30);
	*(pcInsert+9) = (char)((rDay %10) + 0x30);
	*(pcInsert+10) = ' ';
	*(pcInsert+11) = 'w';
	*(pcInsert+12) = 'e';
	*(pcInsert+13) = 'e';
	*(pcInsert+14) = 'k';
	*(pcInsert+15) = ':';
	*(pcInsert+16) = (char)(rDayOfWeek%10 + 0x30);
	
}
//SSI的Handler句柄
static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
	switch(iIndex)
	{
		case 0: 
			RTCTime_Handler(pcInsert);
			//Temperate_Handler(pcInsert);
				break;
		case 1:
			RTCdate_Handler(pcInsert);
			//RTCTime_Handler(pcInsert);
				break;
		case 2:
			Temperate_Handler(pcInsert);
			//RTCdate_Handler(pcInsert);
				break;
		case 3:
		    CPU_Handler(pcInsert);
				break;
	}
	return strlen(pcInsert);
}

//CGI LED控制句柄
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	int i=0;  //注意根据自己的GET的参数的多少来选择i值范围
	iIndex = FindCGIParameter("LED1",pcParam,iNumParams);  //找到led的索引号
	//只有一个CGI句柄 iIndex=0
	if (iIndex != -1)
	{
		rGPBDAT = 0x1E0;  //关闭LED灯
		for (i=0; i<iNumParams; i++) //检查CGI参数
		{
		  if (strcmp(pcParam[i] , "LED1")==0)  //检查参数"led" 属于控制LED1灯的
		  {
			  if (strcmp(pcValue[i], "LED1ON") == 0)  //改变LED1状态
				  {LED_Display(1);
				  flag1=1;
				  }
				  
			else if(strcmp(pcValue[i],"LED1OFF") == 0)
				{rGPBDAT = 0x1E0; //关闭LED
				flag1=0;
				}
		  }
		}
	 }
	if(flag1==0&&flag2==0)      return "/TQ2440_LED_ON_BEEP_ON.shtml";    //LED1开,BEEP关
	else if(flag1==0&&flag2==1) return "/TQ2440_LED_ON_BEEP_OFF.shtml";	  //LED1开,BEEP开
	else if(flag1==1&&flag2==1) return "/TQ2440_LED_OFF_BEEP_OFF.shtml";  //LED1关,BEEP开
	else return "/TQ2440_LED_OFF_BEEP_ON.shtml";   						  //LED1关,BEEP关					
}

//BEEP的CGI控制句柄
const char *BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	int i=0;
	iIndex = FindCGIParameter("BEEP",pcParam,iNumParams);  //找到BEEP的索引号
	printf("--------------in----------------");
	if(iIndex != -1) //找到BEEP索引号
	{
		for(i = 0;i < iNumParams;i++)
		{
			if(strcmp(pcParam[i],"BEEP") == 0)  //查找CGI参数
			{
				if(strcmp(pcValue[i], "BEEPON") == 0) //打开BEEP
				{
				    printf("--------------in----------------");
					Beep(256, 2000);
					Beep(288, 2000);
					Beep(320, 2000);
					Beep(256, 2000);
					flag2=1;
				}
				else if(strcmp(pcValue[i],"BEEPOFF") == 0) //关闭BEEP
			        flag2=0;
			}
		}
	}
	if(flag1==0&&flag2==0)      return "/TQ2440_LED_ON_BEEP_ON.shtml";    //LED1开,BEEP关
	else if(flag1==0&&flag2==1) return "/TQ2440_LED_ON_BEEP_OFF.shtml";	  //LED1开,BEEP开
	else if(flag1==1&&flag2==1) return "/TQ2440_LED_OFF_BEEP_OFF.shtml";  //LED1关,BEEP开
	else return "/TQ2440_LED_OFF_BEEP_ON.shtml";   							//LED1关,BEEP关		
}

//SSI句柄初始化
void httpd_ssi_init(void)
{  
	//配置SSI句柄
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

//CGI句柄初始化
void httpd_cgi_init(void)
{
  //配置CGI句柄
  http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);

}


