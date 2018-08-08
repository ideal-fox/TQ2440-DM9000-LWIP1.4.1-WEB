/**************************************************
**	文件名:lcd.h
**	版本号:V 1.0
**	文件说明:配置LCD相关参数
**		使用不同的屏请修改第13行的预定义中
***************************************************/

#define S35		1
#define W35		2
#define T35		3
#define W43		4
#define VGA		5
#define TFT70		6

#define LCD_TYPE	W43		//显示类型选择

#define MVAL		(13)
#define MVAL_USED 	(0)		//0=each frame   1=rate by MVAL
#define INVVDEN		(1)		//0=normal       1=inverted
#define BSWP		(0)		//Byte swap control
#define HWSWP		(1)		//Half word swap control

#define M5D(n) ((n) & 0x1fffff)	// To get lower 21bits

#if(LCD_TYPE == S35)
//TFT 320240
#define LCD_XSIZE_TFT 	(320)	
#define LCD_YSIZE_TFT 	(240)

#define SCR_XSIZE_TFT 	(320)
#define SCR_YSIZE_TFT 	(240)

//Timing parameter for LCD LTV350QV-F0E
#define VBPD                      (3)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (15)		//垂直同步信号的脉宽

#define HBPD                      (5)		//水平同步信号的后肩
#define HFPD                      (15)		//水平同步信号的前肩
#define HSPW                      (8)		//水平同步信号的脉宽

#define CLKVAL_TFT           (7) 	
//FCLK=180MHz,HCLK=90MHz,VCLK=6.5MHz

#elif(LCD_TYPE == W35)
//TFT 320240
#define LCD_XSIZE_TFT 	(320)	
#define LCD_YSIZE_TFT 	(240)

#define SCR_XSIZE_TFT 	(320)
#define SCR_YSIZE_TFT 	(240)

//Timing parameter for LCD LTV350QV-F0E
#define VBPD                      (12)		//垂直同步信号的后肩
#define VFPD                      (4)		//垂直同步信号的前肩
#define VSPW                      (5)		//垂直同步信号的脉宽

#define HBPD                      (22)		//水平同步信号的后肩
#define HFPD                      (33)		//水平同步信号的前肩
#define HSPW                      (44)		//水平同步信号的脉宽

#define CLKVAL_TFT           (6) 	
//FCLK=180MHz,HCLK=90MHz,VCLK=6.5MHz

#elif(LCD_TYPE == T35)
//TFT 240320
#define LCD_XSIZE_TFT 	(240)	
#define LCD_YSIZE_TFT 	(320)

#define SCR_XSIZE_TFT 	(240)
#define SCR_YSIZE_TFT 	(320)

//Timing parameter for LCD TFT NEC
#define VBPD                      (1)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (1)		//垂直同步信号的脉宽

#define HBPD                      (2)		//水平同步信号的后肩
#define HFPD                      (2)		//水平同步信号的前肩
#define HSPW                      (1)		//水平同步信号的脉宽

#define CLKVAL_TFT           (4) 	
//FCLK=180MHz,HCLK=90MHz,VCLK=6.5MHz

#elif(LCD_TYPE == W43)
//TFT 320240
#define LCD_XSIZE_TFT 	(480)	
#define LCD_YSIZE_TFT 	(272)

#define SCR_XSIZE_TFT 	(480)
#define SCR_YSIZE_TFT 	(272)

//TFT_SIZE
#define LCD_XSIZE_TFT 	(480)	
#define LCD_YSIZE_TFT 	(272)

#define SCR_XSIZE_TFT 	(480)
#define SCR_YSIZE_TFT 	(272)

//Timing parameter for 4.3' LCD
#define VBPD 		(2)			//垂直同步信号的后肩
#define VFPD 		(4)			//垂直同步信号的前肩
#define VSPW 		(8)			//垂直同步信号的脉宽

#define HBPD 		(10)			//水平同步信号的后肩
#define HFPD 		(19)			//水平同步信号的前肩
#define HSPW 		(30)			//水平同步信号的脉宽

#define CLKVAL_TFT 	(4) 	

#elif(LCD_TYPE == VGA)				// VGA
//TFT 640480
#define LCD_XSIZE_TFT 	(640)	
#define LCD_YSIZE_TFT 	(480)

#define SCR_XSIZE_TFT 	(640)
#define SCR_YSIZE_TFT 	(480)

//Timing parameter for VGA
#define VBPD                      (29)		//垂直同步信号的后肩
#define VFPD                      (5)		//垂直同步信号的前肩
#define VSPW                      (4)		//垂直同步信号的脉宽

#define HBPD                      (71)		//水平同步信号的后肩
#define HFPD                      (40)		//水平同步信号的前肩
#define HSPW                      (31)		//水平同步信号的脉宽

#define CLKVAL_TFT			(1) 	

#elif(LCD_TYPE == TFT70)
//TFT 800480
#define LCD_XSIZE_TFT 	(800)	
#define LCD_YSIZE_TFT 	(480)

#define SCR_XSIZE_TFT 	(800)
#define SCR_YSIZE_TFT 	(480)

//Timing parameter for LCD LTP700WV-F01
#define VBPD                      (4)		//垂直同步信号的后肩
#define VFPD                      (2)		//垂直同步信号的前肩
#define VSPW                      (2)		//垂直同步信号的脉宽

#define HBPD                      (27)		//水平同步信号的后肩
#define HFPD                      (14)		//水平同步信号的前肩
#define HSPW                      (8)		//水平同步信号的脉宽

#define CLKVAL_TFT           (2) 	
//FCLK=180MHz,HCLK=90MHz,VCLK=6.5MHz

#endif

#define HOZVAL_TFT          (LCD_XSIZE_TFT -1)
#define LINEVAL_TFT         (LCD_YSIZE_TFT -1)


#if(LCD_TYPE == T35)
	extern unsigned char TQ_LOGO_240320[];
#elif((LCD_TYPE == W35) || (LCD_TYPE == S35))
	extern unsigned char phone[];
#elif(LCD_TYPE == W43)
	extern unsigned char TQ_LOGO_480272[];
#elif(LCD_TYPE == VGA)
	extern unsigned char VGA_ucos[];
#elif(LCD_TYPE == TFT70)
	extern unsigned char TFT70_ucos[];
#endif

