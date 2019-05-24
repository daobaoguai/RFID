#ifndef  __main_h__	   //if not define ��ֹ���ض���
#define  __main_h__

//��typedef�����µ�������
typedef	unsigned char	BOOLEAN;
typedef	unsigned char	INT8U;
typedef	signed char		INT8S;
typedef	unsigned int	INT16U;
typedef	signed int		INT16S;
typedef	unsigned long  	INT32U;
typedef	signed long    	INT32S;

#define	FOSC	18432000L //fosc��Ƭ����ʱ��/�������е�ϵͳʱ��Ƶ�ʣ�������Ϊ18.432M,L����long
#define	BAUD	9600	 //baud���أ���������Ϊ9600

#define	FALSE	0
#define	TRUE	1

#define	WR		0	//д
#define	RD		1	 //��

#define nop() _nop_()  //�ղ���

#define	BIT(n)	( 1 << n )



/******************
UartCmdLen��UartCmd + UartErrCode
UartDataLen��UartDataBuf
*******************/
typedef struct __sUartData
{
	INT8U UartCmdLen;	 //unsigned char command length
	INT8U UartDataLen;	 //unsigned char data  length
	INT16U UartCmd;		 //unsigned int command
	INT8U  UartErrCode;	 //unsigned char error code
	INT8U  UartDataBuf[1]; // data buf,one space,the type of data in it is unsigned char
} *psUartData;	  //�����µ�������*psUartData����ṹ������__sUartData

#define	LED_NONE	0
#define	LED_LONG	1
#define	LED_200MS	2
#define	LED_80MS	3

//enum	 //ö��
//{
//	LED_NONE  = 0,
//	LED_LONG  = 1,
//	LED_200MS = 2,
//	LED_80MS  = 3,
//};

extern void delay_ms( INT16U tms );	//�����ⲿ��ʱ����delay_ms
extern void delay_s( INT16U ts );

#endif 
