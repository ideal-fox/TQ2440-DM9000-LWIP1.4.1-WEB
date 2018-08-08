/**************************************************
**	文件名:2440lib.c
**	版本号:V 1.0
**	文件说明:实现S3C2440的PLL，串口，以及各个IO口的初始化
**		同时，可以将一些常用函数放到该文件中，以便于其他函数调用。
***************************************************/

#include "config.h"

extern char Image$$RW$$Limit[];
void *mallocPt=Image$$RW$$Limit;

//***************************[ SYSTEM ]***************************************************
static int delayLoopCount;
 #define LOOP 10000
 volatile U32 preScaler=49;
void Delay(int time)
{
      	// time=0: adjust the Delay function by WatchDog timer.
      	// time>0: the number of loop time
      	// resolution of time is 100us.
 	int i, adjust=0;

    	if(time==0)
    	{
       	time   = 200;
        	adjust = 1;
        	delayLoopCount = 800;

            	//PCLK/1M,Watch-dog disable,1/64,interrupt disable,reset disable
        	rWTCON = ((PCLK/1000000-1)<<8)|(2<<3); 
        	rWTDAT = 0xffff;                              //for first update
        	rWTCNT = 0xffff;                              //resolution=64us @any PCLK 
		rWTCON = ((PCLK/1000000-1)<<8)|(2<<3)|(1<<5); //Watch-dog timer start
    	}

    	for(;time>0;time--)
       	for(i=0;i<delayLoopCount;i++);

    	if(adjust==1)
    	{
        	rWTCON = ((PCLK/1000000-1)<<8)|(2<<3); //Watch-dog timer stop
        	i = 0xffff - rWTCNT;                     //1count->64us, 200*800 cycle runtime = 64*i us

		//Uart_Printf("\nrWTCNT=%x ", rWTCNT);
		//Uart_Printf("\ni (0xffff -rWTCNT)=%d", i);
		
        	delayLoopCount = 16000000/(i*64);         //200*800:64*i=1*x:100 -> x=160000*100/(64*i)   
        	//Uart_Printf("\ndelayLoopCount=%d", delayLoopCount);
    	}
    		
}


//***************************[ PORTS ]****************************************************
void Port_Init(void)
{
    //CAUTION:Follow the configuration order for setting the ports. 
    // 1) setting value(GPnDAT) 
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)  

    //32bit data bus configuration  
    //*** PORT A GROUP
    //Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12  
    //Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1 
    //Binary :  1     1      1  , 1   1   1    1   ,  1     1     1     1
    //Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0 
    //Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1         
    rGPACON = 0x7fffff; 

    //**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
    //Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
    //Setting: OUTPUT  OUTPUT   OUTPUT  OUTPUT   OUTPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT 
    //Binary :   01  ,  01       01  ,   01      01   ,  01       01  ,   01     01   ,  01        01  
    rGPBCON = 0x155555;
    rGPBUP  = 0x7ff;     // The pull up function is disabled GPB[10:0]

    //*** PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND  
    //Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
    rGPCCON = 0xaaaaaaaa;       
    rGPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0] 

    //*** PORT D GROUP
    //Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
    //Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
    //Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
    rGPDCON = 0xaaaaaaaa;       
    rGPDUP  = 0xffff;     // The pull up function is disabled GPD[15:0]

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4  
    //Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SSDO 
    //Binary :  10     10  ,  10      10  ,  10      10   ,  10      10   ,   10    10  , 10     10  ,     
    //-------------------------------------------------------------------------------------------------------
    //Ports  :  GPE3   GPE2  GPE1    GPE0    
    //Signal : I2SSDI CDCLK I2SSCLK I2SLRCK     
    //Binary :  10     10  ,  10      10 
    rGPECON = 0xaaaaaaaa;       
    rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]

	
    //*** PORT F GROUP
    //Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
    //Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
    //Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
    //Binary :  01      01 ,  01     01  ,     10       10  , 10     10
    rGPFCON = 0x55aa;
    rGPFUP  = 0xff;     // The pull up function is disabled GPF[7:0]

    //*** PORT G GROUP
    //Ports  : GPG15 GPG14 GPG13 GPG12 GPG11    GPG10    GPG9     GPG8     GPG7      GPG6    
    //Signal : nYPON  YMON nXPON XMON  EINT19 DMAMODE1 DMAMODE0 DMASTART KBDSPICLK KBDSPIMOSI
    //Setting: nYPON  YMON nXPON XMON  EINT19  Output   Output   Output   SPICLK1    SPIMOSI1
    //Binary :   11    11 , 11    11  , 10      01    ,   01       01   ,    11         11
    //-----------------------------------------------------------------------------------------
    //Ports  :    GPG5       GPG4    GPG3    GPG2    GPG1    GPG0    
    //Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
    //Setting:  SPIMISO1  LCD_PWRDN EINT11   nSS0   EINT9    EINT8
    //Binary :     11         11   ,  10      11  ,  10        10
    rGPGCON = 0xff95ffba;
    rGPGUP  = 0xffff;    // The pull up function is disabled GPG[15:0]

    //*** PORT H GROUP
    //Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0 
    //Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
    //Binary :   10   ,  10     10 , 11    11  , 10   10 , 10   10 , 10    10
    rGPHCON = 0x2afaaa;
    rGPHUP  = 0x7ff;    // The pull up function is disabled GPH[10:0]

    // Added for S3C2440, DonGo
    //PORT J GROUP
    //Ports	:  GPJ12    GPJ11     GPJ10	GPJ9  GPJ8      GPJ7	GPJ6  GPJ5	GPJ4  GPJ3  GPJ2  GPJ1  GPJ0
    //Signal : CAMRESET CAMCLKOUT CAMHREF CAMVS CAMPCLKIN CAMD7 CAMD6 CAMD5 CAMD4 CAMD3 CAMD2 CAMD1 CAMD0
    //Setting: Out      Out      CAMHREF CAMVS CAMPCLKIN CAMD7 CAMD6 CAMD5 CAMD4 CAMD3 CAMD2 CAMD1 CAMD0
    //Binary : 01	  01        10      10    10        10    10    10    10    10    10    10    10
    //PU_OFF : 1	  0 		1	    1     1         1     1     1		1	1     1     1     1
    //---------------------------------------------------------------------------------------
    rGPJDAT = (1<<12)|(0<<11);
    rGPJCON = 0x016aaaa;
    rGPJUP	= ~((0<<12)|(1<<11));

    rGPJDAT = (0<<12)|(0<<11);
    rGPJCON = 0x016aaaa;
    rGPJUP	= 0x1fff;//~((1<<12)|(1<<11));

	
    //rGPJCON = 0x02aaaaaa;
    //rGPJUP  = 0x1fff;    // The pull up function is disabled GPH[10:0]

    //External interrupt will be falling edge triggered. 
    rEXTINT0 = 0x22222222;    // EINT[7:0]
    rEXTINT1 = 0x22222222;    // EINT[15:8]
    rEXTINT2 = 0x22222222;    // EINT[23:16]
}


//***************************[ UART ]******************************
static int whichUart=0;

void Uart_Init(int pclk,int baud)
{
    int i;
    if(pclk == 0)
    pclk    = PCLK;
    rUFCON0 = 0x0;   //UART channel 0 FIFO control register, FIFO disable
    rUFCON1 = 0x0;   //UART channel 1 FIFO control register, FIFO disable
    rUFCON2 = 0x0;   //UART channel 2 FIFO control register, FIFO disable
    rUMCON0 = 0x0;   //UART chaneel 0 MODEM control register, AFC disable
    rUMCON1 = 0x0;   //UART chaneel 1 MODEM control register, AFC disable
//UART0
    rULCON0 = 0x3;   //Line control register : Normal,No parity,1 stop,8 bits
     //    [10]       [9]     [8]        [7]        [6]      [5]         [4]           [3:2]        [1:0]
     // Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
     //     0          1       0    ,     0          1        0           0     ,       01          01
     //   PCLK       Level    Pulse    Disable    Generate  Normal      Normal        Interrupt or Polling
    rUCON0  = 0x245;   // Control register
    rUBRDIV0=( (int)(pclk/16./baud+0.5) -1 );   //Baud rate divisior register 0
//UART1
    rULCON1 = 0x3;
    rUCON1  = 0x245;
    rUBRDIV1=( (int)(pclk/16./baud+0.5) -1 );
//UART2
    rULCON2 = 0x3;
    rUCON2  = 0x245;
    rUBRDIV2=( (int)(pclk/16./baud+0.5) -1 );    

	Uart_TxEmpty(whichUart);
    //for(i=0;i<100;i++);
}

//===================================================================
void Uart_Select(int ch)
{
    whichUart = ch;
}

//===================================================================
void Uart_TxEmpty(int ch)
{
    if(ch==0)
        while(!(rUTRSTAT0 & 0x4)); //Wait until tx shifter is empty.
          
    else if(ch==1)
        while(!(rUTRSTAT1 & 0x4)); //Wait until tx shifter is empty.
        
    else if(ch==2)
        while(!(rUTRSTAT2 & 0x4)); //Wait until tx shifter is empty.
}

//=====================================================================
char Uart_Getch(void)
{
    if(whichUart==0)
    {       
        while(!(rUTRSTAT0 & 0x1)); //Receive data ready
        return RdURXH0();
    }
    else if(whichUart==1)
    {       
        while(!(rUTRSTAT1 & 0x1)); //Receive data ready
        return RdURXH1();
    }
    else if(whichUart==2)
    {
        while(!(rUTRSTAT2 & 0x1)); //Receive data ready
        return RdURXH2();
    }
}

//====================================================================
char Uart_GetKey(void)
{
    if(whichUart==0)
    {       
        if(rUTRSTAT0 & 0x1)    //Receive data ready
            return RdURXH0();
        else
            return 0;
    }
    else if(whichUart==1)
    {
        if(rUTRSTAT1 & 0x1)    //Receive data ready
            return RdURXH1();
        else
            return 0;
    }
    else if(whichUart==2)
    {       
        if(rUTRSTAT2 & 0x1)    //Receive data ready
            return RdURXH2();
        else
            return 0;
    }    
}

//====================================================================
void Uart_GetString(char *string)
{
    char *string2 = string;
    char c;
    while((c = Uart_Getch())!='\r')
    {
        if(c=='\b')
        {
            if( (int)string2 < (int)string )
            {
                Uart_Printf("\b \b");
                string--;
            }
        }
        else 
        {
            *string++ = c;
            Uart_SendByte(c);
        }
    }
    *string='\0';
    Uart_SendByte('\n');
}

//=====================================================================
int Uart_GetIntNum(void)
{
    char str[30];
    char *string = str;
    int base     = 10;
    int minus    = 0;
    int result   = 0;
    int lastIndex;    
    int i;
    
    Uart_GetString(string);
    
    if(string[0]=='-')
    {
        minus = 1;
        string++;
    }
    
    if(string[0]=='0' && (string[1]=='x' || string[1]=='X'))
    {
        base    = 16;
        string += 2;
    }
    
    lastIndex = strlen(string) - 1;
    
    if(lastIndex<0)
        return -1;
    
    if(string[lastIndex]=='h' || string[lastIndex]=='H' )
    {
        base = 16;
        string[lastIndex] = 0;
        lastIndex--;
    }

    if(base==10)
    {
        result = atoi(string);
        result = minus ? (-1*result):result;
    }
    else
    {
        for(i=0;i<=lastIndex;i++)
        {
            if(isalpha(string[i]))
            {
                if(isupper(string[i]))
                    result = (result<<4) + string[i] - 'A' + 10;
                else
                    result = (result<<4) + string[i] - 'a' + 10;
            }
            else
                result = (result<<4) + string[i] - '0';
        }
        result = minus ? (-1*result):result;
    }
    return result;
}

//=====================================================================
void Uart_SendByte(int data)
{
    if(whichUart==0)
    {
        if(data=='\n')
        {
            while(!(rUTRSTAT0 & 0x2));
            Delay(10);                 //because the slow response of hyper_terminal 
            WrUTXH0('\r');
        }
        while(!(rUTRSTAT0 & 0x2));   //Wait until THR is empty.
        Delay(10);
        WrUTXH0(data);
    }
    else if(whichUart==1)
    {
        if(data=='\n')
        {
            while(!(rUTRSTAT1 & 0x2));
            Delay(10);                 //because the slow response of hyper_terminal 
            rUTXH1 = '\r';
        }
        while(!(rUTRSTAT1 & 0x2));   //Wait until THR is empty.
        Delay(10);
        rUTXH1 = data;
    }   
    else if(whichUart==2)
    {
        if(data=='\n')
        {
            while(!(rUTRSTAT2 & 0x2));
            Delay(10);                 //because the slow response of hyper_terminal 
            rUTXH2 = '\r';
        }
        while(!(rUTRSTAT2 & 0x2));   //Wait until THR is empty.
        Delay(10);
        rUTXH2 = data;
    }       
}               

//====================================================================
void Uart_SendString(char *pt)
{
    while(*pt)
        Uart_SendByte(*pt++);
}

#if !USE_MAIN
//If you don't use vsprintf(), the code size is reduced very much.
void Uart_Printf(const char *fmt,...)
{
    va_list ap;
    char string[50];

    va_start(ap,fmt);
    vsprintf(string,fmt,ap);
    va_end(ap);
    Uart_SendString(string);
}
#endif

//*************************[ Timer ]********************************
void Timer_Start(int divider)  //0:16us,1:32us 2:64us 3:128us
{
    rWTCON = ((PCLK/1000000-1)<<8)|(divider<<3);  //Watch-dog timer control register
    rWTDAT = 0xffff;  //Watch-dog timer data register
    rWTCNT = 0xffff;  //Watch-dog count register

      // Watch-dog timer enable & interrupt  disable
    rWTCON = (rWTCON & ~(1<<5) & ~(1<<2)) |(1<<5);
}

//=================================================================
int Timer_Stop(void)
{
    rWTCON = ((PCLK/1000000-1)<<8);
    return (0xffff - rWTCNT);
}


//*************************[ MPLL ]*******************************
void ChangeMPllValue(int mdiv,int pdiv,int sdiv)
{
    rMPLLCON = (mdiv<<12) | (pdiv<<4) | sdiv;
    __asm
    {
    	NOP;
    	NOP;
    	NOP;
    	NOP;
    	NOP;
    	NOP;
    	NOP;
    	NOP;
    }
}


//************************[ HCLK, PCLK ]***************************
void ChangeClockDivider(int hdivn_val,int pdivn_val)
{
	int hdivn=2, pdivn=0;
	
     // hdivn_val (FCLK:HCLK)ratio hdivn
     // 11           1:1       (0)
     // 12           1:2       (1)
     // 13           1:3       (3) 
     // 14           1:4       (2)
     // pdivn_val (HCLK:PCLK)ratio pdivn
     // 11           1:1       (0)
     // 12           1:2       (1)
	switch(hdivn_val) {
		case 11: hdivn=0; break;
		case 12: hdivn=1; break;
		case 13: hdivn=3; break;
		case 16: hdivn=3; break;
		case 14: hdivn=2; break;
		case 18: hdivn=2; break;
	}
	
	switch(pdivn_val) {
		case 11: pdivn=0; break;
		case 12: pdivn=1; break;
	}
	
	//Uart_Printf("Clock division change [hdiv:%x, pdiv:%x]\n", hdivn, pdivn);
	rCLKDIVN = (hdivn<<1) | pdivn;
	//Uart_Printf("rCLKDIVN:%x]\n", rCLKDIVN);

	switch(hdivn_val) {
		case 16:		// when 1, HCLK=FCLK/6.
			rCAMDIVN = (rCAMDIVN & ~(3<<8)) | (1<<8); 
		break; 
		case 18: 	// when 1, HCLK=FCLK/8.
			rCAMDIVN = (rCAMDIVN & ~(3<<8)) | (1<<9); 
		break;
	}
	//Uart_Printf("rCAMDIVN:%x]\n", rCAMDIVN);
	
    if(hdivn!=0)
        MMU_SetAsyncBusMode();
    else 
        MMU_SetFastBusMode();
}



//**************************[ UPLL ]*******************************
void ChangeUPllValue(int mdiv,int pdiv,int sdiv)
{
    rUPLLCON = (mdiv<<12) | (pdiv<<4) | sdiv;
}

/************************【实时时钟】*********************************/
void Rtc_Init(void)
{
	int wYear, wMonth,wDay,wDayOfWeek,wHour,wMinute,wSecond;

	wYear = 2009;
	wMonth = 3;
	wDay = 5;
	wDayOfWeek = 4;
	wHour = 12;
	wMinute = 00;
	wSecond = 00;

	rRTCCON = 1 ;		//RTC read and write enable

	rBCDYEAR = (unsigned char)TO_BCD(wYear%100);		//年
	rBCDMON  = (unsigned char)TO_BCD(wMonth);		//月
	rBCDDAY	 = (unsigned char)TO_BCD(wDay);			//日	
	rBCDDATE = wDayOfWeek+1;				//星期
	rBCDHOUR = (unsigned char)TO_BCD(wHour);		//小时
	rBCDMIN  = (unsigned char)TO_BCD(wMinute);		//分
	rBCDSEC  = (unsigned char)TO_BCD(wSecond);		//秒

	rRTCCON &= ~1 ;		//RTC read and write disable
}

/*****************************【蜂鸣器】************************************/
void Buzzer_Freq_Set( U32 freq )
{
	rGPBCON &= ~3;			//set GPB0 as tout0, pwm output
	rGPBCON |= 2;

	rTCFG0 &= ~0xff;
	rTCFG0 |= 15;			//prescaler = 15+1
	rTCFG1 &= ~0xf;
	rTCFG1 |= 2;			//mux = 1/8
	rTCNTB0 = (PCLK>>7)/freq;
	rTCMPB0 = rTCNTB0>>1;		// 50%
	rTCON &= ~0x1f;
	rTCON |= 0xb;			//disable deadzone, auto-reload, inv-off, update TCNTB0&TCMPB0, start timer 0
	rTCON &= ~2;			//clear manual update bit
}

void Buzzer_Stop( void )
{
	rGPBCON &= ~3;			//set GPB0 as output
	rGPBCON |= 1;
	rGPBDAT &= ~1;
}

void Beep(U32 freq, U32 ms)
{
	Buzzer_Freq_Set( freq ) ;
	Delay( ms ) ;
	Buzzer_Stop() ;
}
void LED_Display(int data)
{
	rGPBDAT = (rGPBDAT & ~(0x5<<5)) | ((~data & 0x5)<<5);  
}
int ReadAdc(int ch)
{
	int i;
	static int prevCh=-1;

	rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);		//setup channel

	if(prevCh!=ch)
	{
		rADCCON = (1<<14)|(preScaler<<6)|(ch<<3);	//setup channel
		for(i=0;i<LOOP;i++);							//delay to set up the next channel
		prevCh=2;
	}
	rADCCON|=0x1;
	Uart_Printf("CON:%x %d",rADCCON,rADCCON & 0x1);									//start ADC
    while(rADCCON & 0x1);							//check if Enable_start is low
	Uart_Printf("rADCCON 0X1");
	while(!(rADCCON & 0x8000));						//check if EC(End of Conversion) flag is high
	//Uart_Printf("rADCDAT0 0x3ff");
	return ( (int)rADCDAT0 & 0x3ff );
}

#define aISR_START	(_ISR_STARTADDRESS+0x20)
#define aISR_END	(_ISR_STARTADDRESS+0xA0)
#define aISR_EINT0	(_ISR_STARTADDRESS+0x20)

#define MAX_INT_NUM ((aISR_END-aISR_START)>>2)

typedef int (*ISR_FUNCPTR)(void*);
ISR_FUNCPTR intIsrFuncs[MAX_INT_NUM];
unsigned int intIsrArgs[MAX_INT_NUM];

extern volatile unsigned int * HandleEINT0;
//OOOOOOOOOOOOOO
void BSP_IRQHandler(void)
{
	(intIsrFuncs[rINTOFFSET])(intIsrArgs[rINTOFFSET]);
}
void BSP_Int_Connect(unsigned int tt, void * func,unsigned int option)
{
	volatile unsigned int * handler = (volatile unsigned int *)0x33FFFF20;
	if(tt == MAX_INT_NUM)
	  return;
	handler[tt] = (unsigned int) BSP_IRQHandler;
	intIsrFuncs[tt] = (ISR_FUNCPTR)func;
	intIsrArgs[tt] = option;
}

#define INT_NUM_LEVELS	32
unsigned int IntLv1Enabled;

int BSP_Int_Enable( int level /*level to be enabled*/)
{
	int key;
	
	if (level < 0 ||level >= INT_NUM_LEVELS)
		return (-1);
	/*set bit in enable mask*/
	IntLv1Enabled |= (1<<level);
	rINTMSK &= ((~IntLv1Enabled) & BIT_ALLMSK);
	
	return 0;
}






