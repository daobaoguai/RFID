#include "include.h"

INT8U  UartBuf[UART_MAX_LEN] = { 0 };
INT8U UartCount = 0, UartLen = 0, UartStart = FALSE, UartComp = FALSE;

/**********************************************************************
functionName:putChar(BYTE sentData)
description:ͨ�����ڷ�������sentData
**********************************************************************/
void send_byte( INT8U SendData )
{
  	ES = 0;		  //�رմ����ж�
  	TI = 0;		 //TI��������жϱ�־
  	SBUF = SendData; //������д�뵽���ڻ���
  	while( TI == 0 );
  	TI = 0;	   
  	ES = 1;	  //�����ж�����
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
���ڷ��͸�ʽ��fe  cmdlen  datalen cxx cxx dxx dxx dxx ... check
**********************************************************************/
void uart_interrupt_receive(void) interrupt 4	  //���ڷ����ж�
{
	INT8U ReByte;
	if( RI == 1 )  //�յ�����
	{
		RI = 0;	    //���ж�����
		UartWaitTime = 0;
		ReByte = SBUF;	 //�Ѵ��ڻ�������ݸ���ReByte

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
  	SCON = 0x50;  //���ڹ�����ģʽ1
  	ES=1;		  //�����ж�����
}
