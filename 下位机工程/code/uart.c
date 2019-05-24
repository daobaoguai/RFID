#include "include.h"

INT8U  UartBuf[UART_MAX_LEN] = { 0 };
INT8U UartCount = 0, UartLen = 0, UartStart = FALSE, UartComp = FALSE;

/**********************************************************************
functionName:putChar(BYTE sentData)
description:通过串口发送数据sentData
**********************************************************************/
void send_byte( INT8U SendData )
{
  	ES = 0;		  //关闭串行中断
  	TI = 0;		 //TI发送完成中断标志
  	SBUF = SendData; //将数据写入到串口缓冲
  	while( TI == 0 );
  	TI = 0;	   
  	ES = 1;	  //串行中断允许
}

/*
void send_bytes( INT8U *Buf, INT8U Len )
{
	while( Len-- )
	{
		send_byte( *Buf++ );	
	}
}
*/

/**********************************************************************
串口发送格式：fe  cmdlen  datalen cxx cxx dxx dxx dxx ... check
**********************************************************************/
void uart_interrupt_receive(void) interrupt 4	  //串口发送中断
{
	INT8U ReByte;
	if( RI == 1 )  //收到数据
	{
		RI = 0;	    //清中断请求
		UartWaitTime = 0;
		ReByte = SBUF;	 //把串口缓冲的数据赋给ReByte

		//send_byte( ReByte );
		
		if( ( UartStart == FALSE ) && ( ReByte == 0xFE ) )
		{
			UartStart = TRUE;
			UartCount = 0;
			UartLen = 255;
		}
		else if( UartStart == TRUE )
		{
			UartBuf[ UartCount++ ] = ReByte;
			if( UartCount == 2 )
			{
				UartLen = UartBuf[0] + UartBuf[1] + 3;
			}
			else if( UartCount >= UartLen )
			{
				UartComp = TRUE;
			}
			if( UartCount >= 45 )
			{
				UartCount = 45;
			}
		}
	}
}

//UART0 initialize
// desired baud rate: 9600
// actual: baud rate:9600 (0.0%)
// char size: 8 bit
// parity: Disabled
void init_uart(void)
{
  	SCON = 0x50;  //串口工作在模式1
  	ES=1;		  //串行中断允许
}
