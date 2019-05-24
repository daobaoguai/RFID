#ifndef  __main_h__	   //if not define 防止多重定义
#define  __main_h__

//用typedef声明新的类型名
typedef	unsigned char	BOOLEAN;
typedef	unsigned char	INT8U;
typedef	signed char		INT8S;
typedef	unsigned int	INT16U;
typedef	signed int		INT16S;
typedef	unsigned long  	INT32U;
typedef	signed long    	INT32S;

#define	FOSC	18432000L //fosc单片机定时器/计数器中的系统时钟频率，即晶振为18.432M,L代表long
#define	BAUD	9600	 //baud波特，即波特率为9600

#define	FALSE	0
#define	TRUE	1

#define	WR		0	//写
#define	RD		1	 //读

#define nop() _nop_()  //空操作

#define	BIT(n)	( 1 << n )



/******************
UartCmdLen：UartCmd + UartErrCode
UartDataLen：UartDataBuf
*******************/
typedef struct __sUartData
{
	INT8U UartCmdLen;	 //unsigned char command length
	INT8U UartDataLen;	 //unsigned char data  length
	INT16U UartCmd;		 //unsigned int command
	INT8U  UartErrCode;	 //unsigned char error code
	INT8U  UartDataBuf[1]; // data buf,one space,the type of data in it is unsigned char
} *psUartData;	  //声明新的类型名*psUartData代表结构体类型__sUartData

#define	LED_NONE	0
#define	LED_LONG	1
#define	LED_200MS	2
#define	LED_80MS	3

//enum	 //枚举
//{
//	LED_NONE  = 0,
//	LED_LONG  = 1,
//	LED_200MS = 2,
//	LED_80MS  = 3,
//};

extern void delay_ms( INT16U tms );	//引用外部延时函数delay_ms
extern void delay_s( INT16U ts );

#endif 
