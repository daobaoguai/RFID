#include "include.h"


void delay_ms( INT16U tms )		  //声明并定义延时函数,单位ms
{
    INT16U i;

    while( tms-- )
    {
    	for( i = 0; i < 300; i ++ )
    	{
            nop();				  //空操作
            nop();
            nop();
            nop();
            nop();
            nop();
    	}
    }
} 


void delay_s( INT16U ts )	 //延时函数，单位S
{
	INT16U x,y;
	for( x = ts*1000; x > 0; x-- )
		for( y = 1100; y > 0; y-- );
}



void init_led( void )				//LED初始化 
{	INT8U i;
	for( i=0;i<5;i++)
	{
		P16=1;
		P17=0;
		delay_ms(500);
		P16=0;
		P17=1;
		delay_ms(200);
    	P1 = 0xff;
	}
}

void init_port( void )		//端口初始化
{
//    P0M1 = 0x00;
//    P0M0 = 0xff;

    P3M1 &= ~0x10; //P3M1=P3M1 && ~0x10
    P3M0 |= 0x10;  //P3M0=P3M0 || 0x10
    //
    //	P1M1=0x00;
    //	P1M0=0x00;
    //
    //	P2M1=0x00;
    //	P2M0=0xff;
    //
    //	P30=1;
    //	P3M1=0x41;
    //	P3M0=0xa2;
    //
    //	P4M1=0x02;
    //	P4M0=0x00;
    BEEP_OFF;
    LED_OFF;
}

void init_par( void )
{
    BEEP_OFF;
    LED_OFF;
}

void init_wdt( void )	 //2.7S	 看门狗初始化
{
    //	WDT_CONTR = 0xC1;
    //	WDT_CONTR = 0x3E;
}

void feed_wdt( void )	//喂狗
{
    //	WDT_CONTR = 0x3E;
}

void init_all(void)
{
    EA = 0;		   //EA 总中断开关 ，暂时关闭中断

    init_timer();
    init_uart();
    init_port();
    init_rc522();
    init_par();
    init_wdt();
    init_led();

    EA = 1;	  //允许中断
}

void main(void)
{
    if((PCON&0x10)==0) //如果POF位=0,PCON电源控制寄存器
    {
       PCON=PCON|0x10;  //将POF位置1
       IAP_CONTR=0x60;  //软复位,从ISP监控区启动
    }
    else
    {
       PCON=PCON&0xef;  //将POF位清零
    }

    lcdInitinal();
    lcdMsg("RFID智能门禁",1,1);
    lcdMsg("科技",3,3);
	lcdMsg("为老人护航",4,2);
    delay_ms(5000);
    lcdInitinal();
    delay_ms(2);

	lcdMsg("功能简述：",1,0); 
    lcdMsg("识别不同类型的卡",2,0);
	lcdMsg("显示卡号以及状态",3,0); 
	lcdMsg("对老人卡关照一下",4,0);
	delay_s(5); 
	

    init_all();	 //初始化开发板


    while(1)
    {
        feed_wdt();
        ctrl_process();		
    }
}
