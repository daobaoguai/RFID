#ifndef _LCD12864_H_
#define _LCD12864_H_

extern void lcdInitinal(void) ;
extern void lcdMsg(unsigned char  *addr,unsigned char line,unsigned char row);    //传送一个字符串
//extern int lcdS(unsigned int counts ,unsigned char line,unsigned char row)    //数字递减 
#endif
