#include "include.h"

const INT8U DefaultKeyABuf[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认密钥数组缓冲区

INT8U CardRevBuf[16] = { 0 }; //数据显示缓冲区，16space，0
INT8U const CardKeyABlockBuf[16] = {  //数据密钥 数组块缓冲区
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0xff,0x07,0x80,0x69,
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
								};

INT8U CtrlMode = CTRL_BOARD_MODE;  //读卡器控制模式
INT8U OptMode  = OPT_INC_MODE;	//选择 模式
INT8U bPass = 1; bWarn = 0;
INT8U LedOnType = LED_LONG;	//LED类型：长亮
INT16U i;
//INT16U flag;
//INT16U count=0;

int CARDREDUCE = 1;	 //时效卡权限减一
int CARDADD = 2;	//时效卡权限加二


void pass( void )  //通过时的LED和蜂鸣器的状态
{	
	lcdMsg("通",3,3);
	P15_ON;
	//LED_ON;
	BEEP_ON;
	delay_ms(2000);
	P15_OFF;
	BEEP_OFF;
	//LED_OFF;
		
}

void warn(void)	 //不通时的LED和蜂鸣器的状态
{
	INT8U i;
	lcdMsg("不通",3,3);
	//lcdMsg("警告",1,3);	
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

void alarm(void)	 //警告时的LED和蜂鸣器的状态
{
	//lcdInitinal();
	//delay_ms(2);
	INT8U i;
	lcdMsg("警告",3,3);		
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

void cal_keyA( INT8U *DestBuf )	 //计算密码数组
{
	const INT8U KeyABuf[] = { 0x20, 0x12, 0x10, 0x01, 0x00, 0x00 };	//密钥
	memcpy( DestBuf, KeyABuf, 6 ); //把KeyABuf的数组拷贝给DestBuf
}

void send_rebck( psUartData psUartDataRevSend )	  //定义 psUartData结构体变量psUartDataRevSend
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

void uart_over( void )	  //接收完成
{
	UartCount = 0;
	UartStart = FALSE;
	UartComp = FALSE;
}

INT8U check_com( psUartData psUartDataRevSend )	  //检查命令
{
	psUartDataRevSend->UartDataBuf[0] = 0xAA;

	return TRUE;
}

INT8U req_card_sn( psUartData psUartDataRevSend )	//寻卡
{
	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( psUartDataRevSend->UartDataBuf, 0x00, psUartDataRevSend->UartDataLen );
			bWarn = 1;
			return FALSE;
		}	
	}
	
	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //防冲撞，返回卡的序列号 4字节 
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

INT8U updata_key( psUartData psUartDataRevSend )  //修改密码
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

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //防冲撞，返回卡的序列号 4字节 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//选卡
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 6 );
		bWarn = 1;
		return FALSE;	
	}

	memcpy( CardKeyABlockBuf, &psUartDataRevSend->UartDataBuf[7], 6 );

	if( PcdWrite( KeyBlockAddr, CardKeyABlockBuf ) != MI_OK )// 写卡
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

INT8U block_write( psUartData psUartDataRevSend )	//块，写
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

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //防冲撞，返回卡的序列号 4字节 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//选卡
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	memcpy( CardWriteBuf, &psUartDataRevSend->UartDataBuf[7], 16 );
	if( PcdWrite( KeyBlockAddr, CardWriteBuf ) != MI_OK )//写卡
	{
		psUartDataRevSend->UartErrCode = ERROR_WRITE_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}
	bPass = 1;
	
	return TRUE;
}

INT8U block_read( psUartData psUartDataRevSend )   //块，读
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

	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
	{
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
		{
			psUartDataRevSend->UartErrCode = ERROR_NOCARD;
			memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
			bWarn = 1;
			return FALSE;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //防冲撞，返回卡的序列号 4字节 
	{
		psUartDataRevSend->UartErrCode = ERROR_ATCLL;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//选卡
	{
		psUartDataRevSend->UartErrCode = ERROR_SLCT;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;
	}

	memcpy( CardKeyABuf, &psUartDataRevSend->UartDataBuf[1], 6 );

	if( PcdAuthState( PICC_AUTHENT1A, KeyBlockAddr, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
	{
		psUartDataRevSend->UartErrCode = ERROR_KEYA_IC;
		memset( &psUartDataRevSend->UartDataBuf[1], 0x00, 16 );
		bWarn = 1;
		return FALSE;	
	}

	if( PcdRead( KeyBlockAddr, CardReadBuf ) != MI_OK )// 读卡
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

void display_error( void )	//程序警告时的显示状态
{		
	
		lcdInitinal();
		delay_ms(2);
        	
        lcdMsg("非法卡",1,3);
		lcdMsg("警告，警告",2,2);
		delay_ms(2);
		warn();
		
}

void display_no_id( void )	//未放卡状态
{
    char DisplayBuf[10];	  //定义显示缓冲区数组，10个空间，每个空间存放一个char型数据
    memset(DisplayBuf,0,10);	 //初始化显示缓冲区

	switch( OptMode )	  //选择模式
        {
		case OPT_INC_MODE:	   //放卡，开机默认
						
			lcdInitinal();
			delay_ms(2);	
            lcdMsg("请放卡",1,2);
			lcdMsg("卡号：",2,0);
            lcdMsg("状态：",3,0); 

			break;

		case OPT_ADD_MODE:	   //加权

			lcdInitinal();
            lcdMsg("增加权限+", 1, 0 );
			lcdMsg("卡号：",2,0);
            lcdMsg("状态：",3,0);						
                        
			break;

		case OPT_READ_MODE:	   //读卡
						
			lcdInitinal();
            lcdMsg("读卡模式",1,3);
			lcdMsg("卡号:",2,0);
            lcdMsg("读卡:----",3,0);
						
			break;

		case OPT_REG_MODE:	 //注册
						
			lcdInitinal();
            lcdMsg("注册模式", 1, 3 );
			lcdMsg("卡号：",2,0);
            lcdMsg("开卡：----",3,0);

			break;

		default:
			break;
	}
}


void display_money( INT32U CardMoney )	//显示money..
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
			lcdMsg("注意",1,3);
			lcdMsg("已如厕五分钟",2,0);
			lcdMsg("请留意老人状况",3,0);
			delay_ms(3000);
			lcdInitinal();
			alarm();
		}  
		else
		{
			pass();
			lcdInitinal();
			delay_ms(2);
			lcdMsg("已离开",3,3);
		}
}	

void display_id( INT8U * SrcBuf )  //放卡时状态
{
	char DisplayBuf[10];
	char *pDisplayChar;
	INT8U Tmp, i;
	//把串口数据转换成字符，小于10数字表示，大于10英文表示
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
 	//识别不同类型的卡
	if  (strcmp(DisplayBuf,"6E34F771")==0)
	{	
		switch(OptMode)	
		{
		case OPT_INC_MODE :
			
			lcdInitinal();
			delay_ms(20);
			lcdMsg("老人卡",1,3);
			lcdMsg("卡号：",2,0);
	   		lcdMsg(DisplayBuf, 2, 3 );			 
    		lcdMsg("状态：",3,0);
			pass();
		//	ff();
		//	fflush();//清除缓存
		//	strcpy(DisplayBuf,"00000000");
		//	flag=1;
			lcdInitinal();
			delay_ms(2);			
			lcdMsg("如厕中",2,2);
			lcdMsg("请稍后再刷卡",3,1);
			ruce();		
		
			i=get_key(); 
			if(i==2)		
			 delay_ms(2000);
			 BEEP_ON;
			 delay_ms(1000);
			 BEEP_OFF;
			
/*		else
		lcdMsg("出来了",3,3);
		 delay_ms(2000);   */
		
		break;

		default:
			lcdInitinal();
			delay_ms(20);
			lcdMsg("老人卡",1,3);
			break;
			}									
	}

	else if(strcmp(DisplayBuf,"BB3F58DF")==0)
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("普通卡",1,3);
						
			}
	else if(strcmp(DisplayBuf,"D23EC3B9")==0)
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("时效卡",1,3);	
			
			}
	else
		{
			lcdInitinal();
			delay_ms(20);
			lcdMsg("未分类",1,3);
				
			
			}             
	 	lcdMsg("卡号：",2,0);
	   	lcdMsg(DisplayBuf, 2, 3 );			 
    	lcdMsg("状态：",3,0);	

}




INT8U ctrl_by_pc( psUartData psUartDataRevSend )	//PC控制（上位机控制）
{
	psUartDataRevSend = psUartDataRevSend;

	CtrlMode = CTRL_PC_MODE;	
	
	lcdMsg("上位机控制",1,1);
	lcdMsg("一选择控制模式",2,0);
	lcdMsg("二读写卡内信息",3,0);
	delay_ms(5000);
	lcdInitinal();
	lcdMsg("上位机控制",1,1);
	lcdMsg("状态：",3,0);	
     
	return TRUE;	
}

INT8U ctrl_by_board( psUartData psUartDataRevSend )	 //读卡器控制（下位机控制）
{
	psUartDataRevSend = psUartDataRevSend;

	display_no_id();
	
	return TRUE;	
}

void pc_process( void )	   //PC程序
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
	
	switch( psUartDataRevSend->UartCmd ) //命令选择
	{	
		case 0x0002:  //检测串口状态 -> fe 03 01 c1 c0 er 00 ck,  <- fc 03 01 c1 c0 er aa ck
			check_com( psUartDataRevSend );
			break;
		case 0x0003: //查询卡号  ->	fe 03 04 c1 c0 er 00 00 00 00 ck, <- fc 03 04 c1 c0 er sn3 sn2 sn1 sn0 ck
			req_card_sn( psUartDataRevSend );
			break;

		case 0x0110: //修改密码方式0 fe 03 0d c1 c0 er kn o5 o4.. o0 n5 n4.. n0 ck, <-fe 03 07 c1 c0 er kn n5 n4.. n0 ck	
			updata_key( psUartDataRevSend ); //修改密码 kn%4 == 3	
			break;

		case 0x0120: //读数据块方式0  -> fe 03 07 c1 c0 er kn ky5 ... ky0 ck, <- fc 03 11 c1 c0 er kn d15...d0 ck 
			block_read( psUartDataRevSend ); //读数据块 kn%4 != 3	
			break;

		case 0x0130: //写数据块方式0  -> fe 03 07 c1 c0 er kn ky5 ... ky0 ck, <- fc 03 11 c1 c0 er kn d15...d0 ck 
			block_write( psUartDataRevSend ); //读数据块 kn%4 != 3	
			break;

		case 0x0140: //板子控制
			ctrl_by_board( psUartDataRevSend ); //读数据块 kn%4 != 3	
			break;	
			
		default:
			psUartDataRevSend->UartErrCode = ERROR_NOCMD;	
			break;
	}
	send_rebck( psUartDataRevSend );
	uart_over();
}

INT8U board_uart_event( void )	 //UART接收
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
		case 0x0141:  //PC控制
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

void board_process( void ) //读卡器程序
{
	INT8U CardReadBuf[16];
	INT8U CardWriteBuf[16];
	INT8U CardKeyABuf[6];
	INT32U CardMoney;
        INT8U KeyVal;


	if( board_uart_event() == TRUE )   //读卡器控制
	{
		return;
	}

	//定义按键功能
	KeyVal = get_key();	
	switch( KeyVal )
	{
		case KEY_1:		//放卡键，默认

			lcdInitinal();		
			OptMode = OPT_INC_MODE;		
            lcdMsg("请放卡", 1, 0 );
			lcdMsg("卡号：",2,0);
			lcdMsg("状态：",3,0);			
			break;

		case KEY_2:	   //加权键

			lcdInitinal();		
			OptMode = OPT_ADD_MODE;			
            lcdMsg("增加权限+",1,0);
			lcdMsg("卡号：",2,0);
			lcdMsg("状态：",3,0);		
			break;

		case KEY_3:		//读卡键

			lcdInitinal();			
			OptMode = OPT_READ_MODE;			
            lcdMsg("读卡模式", 1, 3 );	
			lcdMsg("卡号：",2,0);
			lcdMsg("状态：",3,0);
			break;

		case KEY_4:		 //注册键

			lcdInitinal();		
			OptMode = OPT_REG_MODE;			
            lcdMsg("注册模式", 1, 3 );
			lcdMsg("卡号：",2,0);
			lcdMsg("状态：",3,0);
			break;

		default:
			break;
	}


	if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
        {
		if( PcdRequest( PICC_REQIDL, &CardRevBuf[0] ) != MI_OK )//寻天线区内未进入休眠状态的卡，返回卡片类型 2字节	
                {
			display_no_id();
			return;
		}	
	}

	if( PcdAnticoll( &CardRevBuf[2] ) != MI_OK ) //防冲撞，返回卡的序列号 4字节 
	{
		display_no_id();
		return;	
	}

	if( PcdSelect( &CardRevBuf[2] ) != MI_OK )//选卡
	{
		display_no_id();
		return;
        }
	display_id( &CardRevBuf[2] );

	cal_keyA( CardKeyABuf );		//计算密码
  //为选择模式，即每个按键定义函数
	switch( OptMode )
	{
		case OPT_REG_MODE:			//新卡注册
			if( PcdAuthState( PICC_AUTHENT1A, 4, DefaultKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
			{
				alarm();	
				return;	
			}
			memset( CardWriteBuf, 0, 16 );
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// 写卡
			{
				alarm();
				return;	
			}
			memcpy( CardWriteBuf, CardKeyABlockBuf, 16 );
			memcpy( CardWriteBuf, CardKeyABuf, 6 );
			if( PcdWrite( 7, CardWriteBuf ) != MI_OK )// 修改密码
			{
				alarm();
				return;	
			}
                        lcdMsg("注册成功", 1, 3);
			bPass = 1;
			break;
				
		case OPT_ADD_MODE:		// 加权
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
			{
				alarm();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// 读数据
			{
				alarm();
				return;	
			}
			memcpy( ( INT8U * )&CardMoney, CardReadBuf, 4 ); 
                        CardMoney += CARDADD;			//
			memset( CardWriteBuf, 0, 16 );
			memcpy( CardWriteBuf, ( INT8U * )&CardMoney, 4 ); 
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// 写数据
			{
				alarm();
				return;	
			}
			display_money( CardMoney );
			bPass = 1;
			break;
			
		case OPT_INC_MODE:		//放卡
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
			{
				display_error();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// 读数据
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
			if( PcdWrite( 4, CardWriteBuf ) != MI_OK )// 写数据
			{
				display_error();
				return;	
			}
			display_money( CardMoney );
			bPass = 1;
			break;  
			
		case OPT_READ_MODE:		//读卡
			if( PcdAuthState( PICC_AUTHENT1A, 4, CardKeyABuf, &CardRevBuf[2] ) != MI_OK )// 验证密码
			{
				display_error();
				return;	
			}
			if( PcdRead( 4, CardReadBuf ) != MI_OK )// 读数据
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

void ctrl_process( void )  //选择控制模式，PC控制或读卡器控制（上位机或下位机）
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

