#include "include.h"

#define DataPort P0	   //数据口，P00~P07

sbit    RS=P3^3; //1发数据，0发命令
sbit    RW=P3^2; //1读数据，0写数据
sbit    EN=P3^5; //使能信号


void lcdDelay(unsigned int m)            //延时函数，ms
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

void lcdBuzy()	  //工作函数
{
    DataPort=0xff;
    RW=1;		   //1读数据
    RS=0;		   //0发命令
    EN=1;		//LCD工作
    while(DataPort&0x80);
    EN=0;
}

void lcdTransferData(char data1,bit DI)  //传输函数，传送数据或者命令,当DI=1,传送数据,当DI=0,传送命令.
{
    lcdBuzy();
    RW=0;
    RS=DI;
    DataPort=data1;
    EN=1;
    EN=0;

}



void lcdDingwei(unsigned char line,unsigned char row)     //定位函数，在哪行哪列显示
{
    unsigned int i;
    switch(line)
    {
    case 1:  i=0x80+row;break;	  //第一行起始地址
    case 2:  i=0x90+row;break;
    case 3:  i=0x88+row;break;
    case 4:  i=0x98+row;break;
    default: i=0x80;break;

    }
    lcdTransferData(i,0);
    lcdDelay(1);
}

void lcdMsg(unsigned char  *addr,unsigned char line,unsigned char row)    //显示函数，传送一个字符串
{
    lcdDingwei(line,row);
    while(*addr>0)
    {
        lcdTransferData(*addr,1);
        addr++;
    }


}		

void lcdInitinal(void)           //LCD字库初始化程序
{
    lcdTransferData(0x30,0);  //8BIT设置,RE=0: basic instruction set
    lcdTransferData(0x08,0);  //Display on Control
    lcdTransferData(0x10,0);  //Cursor Display Control光标显示控制
    lcdTransferData(0x0C,0);  //Display Control,D=1,显示控制
    lcdTransferData(0x01,0);  //Display Clear  
}


 /*
 //向LCD12864中写入数据
  void LCD12864_Write_Data(unsigned char dat)
        {
            EN = 0;
            RW = 0;//选择写模式 
            RS = 1;//选择数据模式

            //延时
            lcdDelay(1);

            //将数据准备
           // LCD12864_Data = dat;

            lcdDelay(1);

            EN = 1;//表示开始写入数据,上升沿写入数据

            lcdDelay(20); //延时等待数据全部发送完成
        }



    //向12864中写入命令
   void LCD12864_Write_Com(unsigned char com)
        {
            //首先我们设置片选信号
            EN = 0;//片选信号一般设置为0表示片选
            RW = 0;//选择写模式 
            RS = 0;//选择命令模式

            //延时
            lcdDelay(1);

            //将数据准备
           // LCD12864_Data = com;

            lcdDelay(1);

            EN = 1;//表示开始写入数据,上升沿写入数据

            lcdDelay(20);
        }
 */