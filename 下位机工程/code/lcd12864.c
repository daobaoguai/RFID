#include "include.h"

#define DataPort P0	   //���ݿڣ�P00~P07

sbit    RS=P3^3; //1�����ݣ�0������
sbit    RW=P3^2; //1�����ݣ�0д����
sbit    EN=P3^5; //ʹ���ź�


void lcdDelay(unsigned int m)            //��ʱ������ms
{
    while(m--)
    {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    }
}

void lcdBuzy()	  //��������
{
    DataPort=0xff;
    RW=1;		   //1������
    RS=0;		   //0������
    EN=1;		//LCD����
    while(DataPort&0x80);
    EN=0;
}

void lcdTransferData(char data1,bit DI)  //���亯�����������ݻ�������,��DI=1,��������,��DI=0,��������.
{
    lcdBuzy();
    RW=0;
    RS=DI;
    DataPort=data1;
    EN=1;
    EN=0;

}



void lcdDingwei(unsigned char line,unsigned char row)     //��λ������������������ʾ
{
    unsigned int i;
    switch(line)
    {
    case 1:  i=0x80+row;break;	  //��һ����ʼ��ַ
    case 2:  i=0x90+row;break;
    case 3:  i=0x88+row;break;
    case 4:  i=0x98+row;break;
    default: i=0x80;break;

    }
    lcdTransferData(i,0);
    lcdDelay(1);
}

void lcdMsg(unsigned char  *addr,unsigned char line,unsigned char row)    //��ʾ����������һ���ַ���
{
    lcdDingwei(line,row);
    while(*addr>0)
    {
        lcdTransferData(*addr,1);
        addr++;
    }


}		

void lcdInitinal(void)           //LCD�ֿ��ʼ������
{
    lcdTransferData(0x30,0);  //8BIT����,RE=0: basic instruction set
    lcdTransferData(0x08,0);  //Display on Control
    lcdTransferData(0x10,0);  //Cursor Display Control�����ʾ����
    lcdTransferData(0x0C,0);  //Display Control,D=1,��ʾ����
    lcdTransferData(0x01,0);  //Display Clear  
}


 /*
 //��LCD12864��д������
  void LCD12864_Write_Data(unsigned char dat)
        {
            EN = 0;
            RW = 0;//ѡ��дģʽ 
            RS = 1;//ѡ������ģʽ

            //��ʱ
            lcdDelay(1);

            //������׼��
           // LCD12864_Data = dat;

            lcdDelay(1);

            EN = 1;//��ʾ��ʼд������,������д������

            lcdDelay(20); //��ʱ�ȴ�����ȫ���������
        }



    //��12864��д������
   void LCD12864_Write_Com(unsigned char com)
        {
            //������������Ƭѡ�ź�
            EN = 0;//Ƭѡ�ź�һ������Ϊ0��ʾƬѡ
            RW = 0;//ѡ��дģʽ 
            RS = 0;//ѡ������ģʽ

            //��ʱ
            lcdDelay(1);

            //������׼��
           // LCD12864_Data = com;

            lcdDelay(1);

            EN = 1;//��ʾ��ʼд������,������д������

            lcdDelay(20);
        }
 */