#include "include.h"


void delay_ms( INT16U tms )		  //������������ʱ����,��λms
{
    INT16U i;

    while( tms-- )
    {
    	for( i = 0; i < 300; i ++ )
    	{
            nop();				  //�ղ���
            nop();
            nop();
            nop();
            nop();
            nop();
    	}
    }
} 


void delay_s( INT16U ts )	 //��ʱ��������λS
{
	INT16U x,y;
	for( x = ts*1000; x > 0; x-- )
		for( y = 1100; y > 0; y-- );
}



void init_led( void )				//LED��ʼ�� 
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

void init_port( void )		//�˿ڳ�ʼ��
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

void init_wdt( void )	 //2.7S	 ���Ź���ʼ��
{
    //	WDT_CONTR = 0xC1;
    //	WDT_CONTR = 0x3E;
}

void feed_wdt( void )	//ι��
{
    //	WDT_CONTR = 0x3E;
}

void init_all(void)
{
    EA = 0;		   //EA ���жϿ��� ����ʱ�ر��ж�

    init_timer();
    init_uart();
    init_port();
    init_rc522();
    init_par();
    init_wdt();
    init_led();

    EA = 1;	  //�����ж�
}

void main(void)
{
    if((PCON&0x10)==0) //���POFλ=0,PCON��Դ���ƼĴ���
    {
       PCON=PCON|0x10;  //��POFλ��1
       IAP_CONTR=0x60;  //��λ,��ISP���������
    }
    else
    {
       PCON=PCON&0xef;  //��POFλ����
    }

    lcdInitinal();
    lcdMsg("RFID�����Ž�",1,1);
    lcdMsg("�Ƽ�",3,3);
	lcdMsg("Ϊ���˻���",4,2);
    delay_ms(5000);
    lcdInitinal();
    delay_ms(2);

	lcdMsg("���ܼ�����",1,0); 
    lcdMsg("ʶ��ͬ���͵Ŀ�",2,0);
	lcdMsg("��ʾ�����Լ�״̬",3,0); 
	lcdMsg("�����˿�����һ��",4,0);
	delay_s(5); 
	

    init_all();	 //��ʼ��������


    while(1)
    {
        feed_wdt();
        ctrl_process();		
    }
}
