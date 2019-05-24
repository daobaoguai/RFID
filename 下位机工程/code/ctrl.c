#include "include.h"

const INT8U DefaultKeyABuf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//Ĭ����Կ���黺����

INT8U CardRevBuf[16] = { 0 }; //������ʾ��������16space��0
INT8U const CardKeyABlockBuf[16] = {  //������Կ ����黺����
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0xff,0x07,0x80,0x69,
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
								};

INT8U CtrlMode = CTRL_BOARD_MODE;  //����������ģʽ
INT8U OptMode  = OPT_INC_MODE;	//ѡ�� ģʽ
INT8U bPass = 1; bWarn = 0;
INT8U LedOnType = LED_LONG;	//LED���ͣ�����
INT16U i;
//INT16U flag;
//INT16U count=0;

int CARDREDUCE = 1;	 //ʱЧ��Ȩ�޼�һ
int CARDADD = 2;	//ʱЧ��Ȩ�޼Ӷ�


void pass( void )  //ͨ��ʱ��LED�ͷ�������״̬
{	
	lcdMsg("ͨ",3,3);
	P15_ON;
	//LED_ON;
	BEEP_ON;
	delay_ms(2000);
	P15_OFF;
	BEEP_OFF;
	//LED_OFF;
		
}

void warn(void)	 //��ͨʱ��LED�ͷ�������״̬
{
	INT8U i;
	lcdMsg("��ͨ",3,3);
	//lcdMsg("����",1,3);	
	LED_OFF;
	for( i = 0; i < 3; i++ )
	{
		LED_ON;
		BEEP_ON;
		delay_ms(200);
		BEEP_OFF;
		LED_OFF;
		delay_ms(200);
	}	
}

void alarm(void)	 //����ʱ��LED�ͷ�������״̬
{
	//lcdInitinal();
	//delay_ms(2);
	INT8U i;
	lcdMsg("����",3,3);		
	LED_OFF;
	for( i = 0; i < 5; i++ )
	{
		LED_ON;
		BEEP_ON;
		delay_ms(150);
		BEEP_OFF;
		LED_OFF;
		delay_ms(150);
	}
	
}

void cal_keyA( INT8U *DestBuf )	 //������������
{
	const INT8U KeyABuf[] = { 0x20, 0x12, 0x10, 0x01, 0x00, 0x00 };	//��Կ
	memcpy( DestBuf, KeyABuf, 6 ); //��KeyABuf�����鿽����DestBuf
}

void send_rebck( psUartData psUartDataRevSend )	  //���� psUartData�ṹ�����psUartDataRevSend
{
	INT8U * pTmp;
	INT8U Len,Check;
	INT8U i;

	Len = psUartDataRevSend->UartCmdLen	+ psUartDataRevSend->UartDataLen + 2;
	pTmp = ( INT8U * )psUartDataRevSend;

	send_byte( 0xFC );
	Check = 0xFC;
	for( i = 0; i < Len; i++ )
	{
		send_byte( *pTmp );
		Check ^= *pTmp++;
	}
	send_byte( Check );	
}

void uart_over( void )	  //�������
{
	UartCount = 0;
	UartStart = FALSE;
	UartComp = FALSE;
}

INT8U check_com( psUartData psUartDataRevSend )	  //�������
{
	psUartDataRevSend->UartDataBuf[0] = 0xAA;

	return TRUE;
}

INT8U req_card_sn( psUartData psUartDataRevSend )	//Ѱ��
{
	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( psUartDataRevSend->UartDataBuf, 0x00, psUartDataRevSend->UartDataLen );
			bWarn = 1;
			return FALSE;
		}	
	}
	
	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //����ײ�����ؿ������к� 4�ֽ� 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( psUartDataRevSend->UartDataBuf, 0x00, psUartDataRevSend->UartDataLen );
		bWarn = 1;
		return FALSE;	
	}

	memcpy( psUartDataRevSend->UartDataBuf, &CardRevBuf[2], 4 );

	bPass = 1;

	return TRUE;
}

INT8U updata_key( psUartData psUartDataRevSend )  //�޸�����
{
	INT8U CardKeyABuf[6];
	INT8U KeyBlockAddr;

	psUartDataRevSend->UartDataLen = 7;
	KeyBlockAddr = psUartDataRevSend->UartDataBuf[0];

	if( KeyBlockAddr % 4 != 3 )
	{
		psUartDataRevSend->UartErrCode = ERROR_BLOCK_ADDR;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //����ײ�����ؿ������к� 4�ֽ� 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//ѡ��
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;	
	}

	memcpy( CardKeyABlockBuf, &psUartDataRevSend->UartDataBuf[7], 6 );

	if( PcdWrite( KeyBlockAddr, CardKeyABlockBuf ) != MI_OK )// д��
	{
		psUartDataRevSend->UartErrCode = ERROR_WRITE_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;
	}

   	memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
	bPass = 1;
	return TRUE;
}

INT8U block_write( psUartData psUartDataRevSend )	//�飬д
{
	INT8U CardWriteBuf[16];
	INT8U CardKeyABuf[6];
	INT8U KeyBlockAddr;

	psUartDataRevSend->UartDataLen = 0x11;
	KeyBlockAddr = psUartDataRevSend->UartDataBuf[0];

	if( KeyBlockAddr % 4 == 3 )
	{
		psUartDataRevSend->UartErrCode = ERROR_BLOCK_ADDR;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //����ײ�����ؿ������к� 4�ֽ� 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//ѡ��
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	memcpy( CardWriteBuf, &psUartDataRevSend->UartDataBuf[7], 16 );
	if( PcdWrite( KeyBlockAddr, CardWriteBuf ) != MI_OK )//д��
	{
		psUartDataRevSend->UartErrCode = ERROR_WRITE_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}
	bPass = 1;
	
	return TRUE;
}

INT8U block_read( psUartData psUartDataRevSend )   //�飬��
{
	INT8U CardReadBuf[16];
	INT8U CardKeyABuf[6];
	INT8U KeyBlockAddr;

	psUartDataRevSend->UartDataLen = 0x11;
	KeyBlockAddr = psUartDataRevSend->UartDataBuf[0];

	if( KeyBlockAddr % 4 == 3 )
	{
		psUartDataRevSend->UartErrCode = ERROR_BLOCK_ADDR;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //����ײ�����ؿ������к� 4�ֽ� 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//ѡ��
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdRead( KeyBlockAddr, CardReadBuf ) != MI_OK )// ����
	{
		psUartDataRevSend->UartErrCode = ERROR_READ_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}
	memcpy( &psUartDataRevSend->UartDataBuf[1], CardReadBuf, 16 );
	bPass = 1;
			
	return TRUE;
}

void display_error( void )	//���򾯸�ʱ����ʾ״̬
{		
	
		lcdInitinal();
		delay_ms(2);
        	
        lcdMsg("�Ƿ���",1,3);
		lcdMsg("���棬����",2,2);
		delay_ms(2);
		warn();
		
}

void display_no_id( void )	//δ�ſ�״̬
{
    char DisplayBuf[10];	  //������ʾ���������飬10���ռ䣬ÿ���ռ���һ��char������
    memset(DisplayBuf,0,10);	 //��ʼ����ʾ������

	switch( OptMode )	  //ѡ��ģʽ
        {
		case OPT_INC_MODE:	   //�ſ�������Ĭ��
						
			lcdInitinal();
			delay_ms(2);	
            lcdMsg("��ſ�",1,2);
			lcdMsg("���ţ�",2,0);
            lcdMsg("״̬��",3,0); 

			break;

		case OPT_ADD_MODE:	   //��Ȩ

			lcdInitinal();
            lcdMsg("����Ȩ��+", 1, 0 );
			lcdMsg("���ţ�",2,0);
            lcdMsg("״̬��",3,0);						
                        
			break;

		case OPT_READ_MODE:	   //����
						
			lcdInitinal();
            lcdMsg("����ģʽ",1,3);
			lcdMsg("����:",2,0);
            lcdMsg("����:----",3,0);
						
			break;

		case OPT_REG_MODE:	 //ע��
						
			lcdInitinal();
            lcdMsg("ע��ģʽ", 1, 3 );
			lcdMsg("���ţ�",2,0);
            lcdMsg("������----",3,0);

			break;

		default:
			break;
	}
}


void display_money( INT32U CardMoney )	//��ʾmoney..
{
	char DisplayBuf[10];
	char *pDisplayChar;
	INT32U Tmp;

	pDisplayChar = DisplayBuf;

	if( CardMoney > 9999 )	//
	{
		CardMoney = 9999;
	}

	Tmp = CardMoney;

	*pDisplayChar++ = Tmp / 1000 + '0';
	Tmp %= 1000;
	*pDisplayChar++ = Tmp / 100 + '0';
	Tmp %= 100;
	*pDisplayChar++ = Tmp / 10 + '0';
	Tmp %= 10;
	*pDisplayChar++ = Tmp + '0';
	*pDisplayChar++ = '\0';     	  

 }	


void ruce( void )	   //
{	
	INT16U z;	
	delay_ms(2000);

	for(z=5;z>0;z--)
	{		
		P16=0;
		delay_ms(2000);
		P16=1;		
		delay_ms(2000);
	}

		if(OptMode = OPT_INC_MODE)
		{
			lcdInitinal();
			delay_ms(2);	
			lcdMsg("ע��",1,3);
			lcdMsg("����������",2,0);
			lcdMsg("����������״��",3,0);
			delay_ms(3000);
			lcdInitinal();
			alarm();
		}  
		else
		{
			pass();
			lcdInitinal();
			delay_ms(2);
			lcdMsg("���뿪",3,3);
		}
}	

void display_id( INT8U * SrcBuf )  //�ſ�ʱ״̬
{
	char DisplayBuf[10];
	char *pDisplayChar;
	INT8U Tmp, i;
	//�Ѵ�������ת�����ַ���С��10���ֱ�ʾ������10Ӣ�ı�ʾ
	pDisplayChar = DisplayBuf;

	for( i = 0; i < 4; i++ )
	{
		Tmp = ( ( *SrcBuf ) >> 4 ) & 0x0F;	
		if( ( Tmp >=0 ) && ( Tmp <= 9 )	)
		{
			*pDisplayChar ++ = '0' + Tmp;	
		}
		else
		{
			*pDisplayChar ++ = 'A' + Tmp - 10;
		}

		Tmp = ( *SrcBuf ) & 0x0F;	
		if( ( Tmp >=0 ) && ( Tmp <= 9 )	)
		{
			*pDisplayChar ++ = '0' + Tmp;	
		}
		else
		{
			*pDisplayChar ++ = 'A' + Tmp - 10;
		}

		SrcBuf ++;
	}
	*pDisplayChar ++ = '\0';
//	char myBuf=DisplayBuf;
 	//ʶ��ͬ���͵Ŀ�
	if  (strcmp(DisplayBuf,"6E34F771")==0)
	{	
		switch(OptMode)	
		{
		case OPT_INC_MODE :
			
			lcdInitinal();
			delay_ms(20);
			lcdMsg("���˿�",1,3);
			lcdMsg("���ţ�",2,0);
	   		lcdMsg(DisplayBuf, 2, 3 );			 
    		lcdMsg("״̬��",3,0);
			pass();
		//	ff();
		//	fflush();//�������
		//	strcpy(DisplayBuf,"00000000");
		//	flag=1;
			lcdInitinal();
			delay_ms(2);			
			lcdMsg("�����",2,2);
			lcdMsg("���Ժ���ˢ��",3,1);
			ruce();		
		
			i=get_key(); 
			if(i==2)		
			 delay_ms(2000);
			 BEEP_ON;
			 delay_ms(1000);
			 BEEP_OFF;
			
/*		else
		lcdMsg("������",3,3);
		 delay_ms(2000);   */
		
		break;

		default:
			lcdInitinal();
			delay_ms(20);
			lcdMsg("���˿�",1,3);
			break;
			}									
	}

	else if(strcmp(DisplayBuf,"BB3F58DF")==0)
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("��ͨ��",1,3);
						
			}
	else if(strcmp(DisplayBuf,"D23EC3B9")==0)
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("ʱЧ��",1,3);	
			
			}
	else
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("δ����",1,3);
				
			
			}             
	 	lcdMsg("���ţ�",2,0);
	   	lcdMsg(DisplayBuf, 2, 3 );			 
    	lcdMsg("״̬��",3,0);	

}




INT8U ctrl_by_pc( psUartData psUartDataRevSend )	//PC���ƣ���λ�����ƣ�
{
	psUartDataRevSend = psUartDataRevSend;

	CtrlMode = CTRL_PC_MODE;	
	
	lcdMsg("��λ������",1,1);
	lcdMsg("һѡ�����ģʽ",2,0);
	lcdMsg("����д������Ϣ",3,0);
	delay_ms(5000);
	lcdInitinal();
	lcdMsg("��λ������",1,1);
	lcdMsg("״̬��",3,0);	
     
	return TRUE;	
}

INT8U ctrl_by_board( psUartData psUartDataRevSend )	 //���������ƣ���λ�����ƣ�
{
	psUartDataRevSend = psUartDataRevSend;

	display_no_id();
	
	return TRUE;	
}

void pc_process( void )	   //PC����
{
	psUartData psUartDataRevSend;
	INT8U i;
	INT8U Check = 0xFE;
	INT8U Len;

	if( UartComp != TRUE )
	{
		return;
	}

	psUartDataRevSend = (psUartData)UartBuf;
	Len = psUartDataRevSend->UartCmdLen + psUartDataRevSend->UartDataLen + 3;

	for( i = 0; i < ( Len - 1 ); i ++ )
	{
		Check ^= UartBuf[i];
	}
	/*
	if( Check != UartBuf[Len - 1] )
	{
		psUartDataRevSend->UartErrCode = ERROR_CHECK;
		send_rebck( psUartDataRevSend );
		uart_over();
		return;
	}
	*/
	
	switch( psUartDataRevSend->UartCmd ) //����ѡ��
	{	
		case 0x0002:  //��⴮��״̬ -> fe 03 01 c1 c0 er 00 ck,  <- fc 03 01 c1 c0 er aa ck
			check_com( psUartDataRevSend );
			break;
		case 0x0003: //��ѯ����  ->	fe 03 04 c1 c0 er 00 00 00 00 ck, <- fc 03 04 c1 c0 er sn3 sn2 sn1 sn0 ck
			req_card_sn( psUartDataRevSend );
			break;

		case 0x0110: //�޸����뷽ʽ0 fe 03 0d c1 c0 er kn o5 o4.. o0 n5 n4.. n0 ck, <-fe 03 07 c1 c0 er kn n5 n4.. n0 ck	
			updata_key( psUartDataRevSend ); //�޸����� kn%4 == 3	
			break;

		case 0x0120: //�����ݿ鷽ʽ0  -> fe 03 07 c1 c0 er kn ky5 ... ky0 ck, <- fc 03 11 c1 c0 er kn d15...d0 ck 
			block_read( psUartDataRevSend ); //�����ݿ� kn%4 != 3	
			break;

		case 0x0130: //д���ݿ鷽ʽ0  -> fe 03 07 c1 c0 er kn ky5 ... ky0 ck, <- fc 03 11 c1 c0 er kn d15...d0 ck 
			block_write( psUartDataRevSend ); //�����ݿ� kn%4 != 3	
			break;

		case 0x0140: //���ӿ���
			ctrl_by_board( psUartDataRevSend ); //�����ݿ� kn%4 != 3	
			break;	
			
		default:
			psUartDataRevSend->UartErrCode = ERROR_NOCMD;	
			break;
	}
	send_rebck( psUartDataRevSend );
	uart_over();
}

INT8U board_uart_event( void )	 //UART����
{
	psUartData psUartDataRevSend;
	INT8U i;
	INT8U Check = 0xFE;
	INT8U Len;
	INT8U Result = FALSE;

	if( UartComp != TRUE )
	{
		return Result;
	}

	psUartDataRevSend = (psUartData)UartBuf;
	Len = psUartDataRevSend->UartCmdLen + psUartDataRevSend->UartDataLen + 3;

	for( i = 0; i < ( Len - 1 ); i ++ )
	{
		Check ^= UartBuf[i];
	}
	/*
	if( Check != UartBuf[Len - 1] )
	{
		psUartDataRevSend->UartErrCode = ERROR_CHECK;
		send_rebck( psUartDataRevSend );
		uart_over();
		return;
	}
	*/
	
	switch( psUartDataRevSend->UartCmd )
	{	
		case 0x0141:  //PC����
			ctrl_by_pc( psUartDataRevSend );
			Result = TRUE;
			break;
			
		default:
			psUartDataRevSend->UartErrCode = ERROR_NOCMD;	
			break;
	}
	send_rebck( psUartDataRevSend );
	uart_over();

	return Result;
}

void board_process( void ) //����������
{
	INT8U CardReadBuf[16];
	INT8U CardWriteBuf[16];
	INT8U CardKeyABuf[6];
	INT32U CardMoney;
        INT8U KeyVal;


	if( board_uart_event() == TRUE )   //����������
	{
		return;
	}

	//���尴������
	KeyVal = get_key();	
	switch( KeyVal )
	{
		case KEY_1:		//�ſ�����Ĭ��

			lcdInitinal();		
			OptMode = OPT_INC_MODE;		
            lcdMsg("��ſ�", 1, 0 );
			lcdMsg("���ţ�",2,0);
			lcdMsg("״̬��",3,0);			
			break;

		case KEY_2:	   //��Ȩ��

			lcdInitinal();		
			OptMode = OPT_ADD_MODE;			
            lcdMsg("����Ȩ��+",1,0);
			lcdMsg("���ţ�",2,0);
			lcdMsg("״̬��",3,0);		
			break;

		case KEY_3:		//������

			lcdInitinal();			
			OptMode = OPT_READ_MODE;			
            lcdMsg("����ģʽ", 1, 3 );	
			lcdMsg("���ţ�",2,0);
			lcdMsg("״̬��",3,0);
			break;

		case KEY_4:		 //ע���

			lcdInitinal();		
			OptMode = OPT_REG_MODE;			
            lcdMsg("ע��ģʽ", 1, 3 );
			lcdMsg("���ţ�",2,0);
			lcdMsg("״̬��",3,0);
			break;

		default:
			break;
	}


	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
        {
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//Ѱ��������δ��������״̬�Ŀ������ؿ�Ƭ���� 2�ֽ�	
                {
			display_no_id();
			return;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //����ײ�����ؿ������к� 4�ֽ� 
	{
		display_no_id();
		return;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//ѡ��
	{
		display_no_id();
		return;
        }
	display_id( &CardRevBuf[2] );

	cal_keyA( CardKeyABuf );		//��������
  //Ϊѡ��ģʽ����ÿ���������庯��
	switch( OptMode )
	{
		case OPT_REG_MODE:			//�¿�ע��
			if( PcdAuthState( PICC_AUTHENT1A, 4, DefaultKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
			{
				alarm();	
				return;	
			}
			memset( CardWriteBuf, 0, 16 );
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// д��
			{
				alarm();
				return;	
			}
			memcpy( CardWriteBuf, CardKeyABlockBuf, 16 );
			memcpy( CardWriteBuf, CardKeyABuf, 6 );
			if( PcdWrite( 7, CardWriteBuf ) != MI_OK )// �޸�����
			{
				alarm();
				return;	
			}
                        lcdMsg("ע��ɹ�", 1, 3);
			bPass = 1;
			break;
				
		case OPT_ADD_MODE:		// ��Ȩ
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
			{
				alarm();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// ������
			{
				alarm();
				return;	
			}
			memcpy( ( INT8U * )&CardMoney, CardReadBuf, 4 ); 
                        CardMoney += CARDADD;			//
			memset( CardWriteBuf, 0, 16 );
			memcpy( CardWriteBuf, ( INT8U * )&CardMoney, 4 ); 
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// д����
			{
				alarm();
				return;	
			}
			display_money( CardMoney );
			bPass = 1;
			break;
			
		case OPT_INC_MODE:		//�ſ�
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
			{
				display_error();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// ������
			{
				display_error();
				return;	
			}
			memcpy( ( INT8U * )&CardMoney, CardReadBuf, 4 ); 
			if( CardMoney == 0 )		 //
			{
				display_error();
				bWarn = 1;
				return;
			}
            CardMoney -= CARDREDUCE;	// 
			memset( CardWriteBuf, 0, 16 );
			memcpy( CardWriteBuf, ( INT8U * )&CardMoney, 4 ); 
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// д����
			{
				display_error();
				return;	
			}
			display_money( CardMoney );
			bPass = 1;
			break;  
			
		case OPT_READ_MODE:		//����
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// ��֤����
			{
				display_error();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// ������
			{
				alarm();
				return;	
			}
			memcpy( ( INT8U * )&CardMoney, CardReadBuf, 4 ); 
			display_money( CardMoney );
			bPass = 1;
			break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
			
		default:
			break;
	}
	PcdHalt();	
}

void ctrl_process( void )  //ѡ�����ģʽ��PC���ƻ���������ƣ���λ������λ����
{

	if( CtrlMode == CTRL_PC_MODE )
        {
		pc_process();
	}
	else if( CtrlMode == CTRL_BOARD_MODE )
        {
		board_process();
	}

	if( bPass )
        {
		bPass = 0;
		pass();	

	}
	if( bWarn )
	{
		bWarn = 0;
		warn();
	}
}

