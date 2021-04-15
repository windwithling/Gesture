#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 
#include "delay.h"
#include "rtc.h"
#include "key.h"
#include "string.h"
#include "paj7620u2.h"
#include "paj7620u2_iic.h"
#include "tpad.h"
//SSD1306 OLED 驱动IC驱动代码
//驱动方式:8080并口/4线串口



//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
u8 OLED_GRAM[128][8];	 




//更新显存到OLED		 
void OLED_Refresh_Gram(void)
{
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA); 
	}   
}
#if OLED_MODE==1	//8080并口 
//向SSD1306写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(u8 dat,u8 cmd)
{
	DATAOUT(dat);	    
 	OLED_RS=cmd;
	OLED_CS=0;	   
	OLED_WR=0;	 
	OLED_WR=1;
	OLED_CS=1;	  
	OLED_RS=1;	 
} 	    	    
#else
//向SSD1306写入一个字节。
//dat:要写入的数据/命令
//cmd:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(u8 dat,u8 cmd)
{	
	u8 i;			  
	OLED_RS=cmd; //写命令 
	OLED_CS=0;		  
	for(i=0;i<8;i++)
	{			  
		OLED_SCLK=0;
		if(dat&0x80)OLED_SDIN=1; //如果是数据,
		else OLED_SDIN=0;
		OLED_SCLK=1;
		dat<<=1;   
	}				 
	OLED_CS=1;		  
	OLED_RS=1;   	  
} 
#endif
	  	  
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;  
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0X00;  
	OLED_Refresh_Gram();//更新显示
}
//画点 
//x:0~127
//y:0~63
//t:1 填充 0,清空				   
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	    
}
//x1,y1,x2,y2 填充区域的对角坐标
//确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,清空;1,填充	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	u8 x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	OLED_Refresh_Gram();//更新显示
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 12/16/24
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{      			    
	u8 temp,t,t1;
	u8 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数
	chr=chr-' ';//得到偏移后的值	ASCLL打印字符从空格也就是十进制32开始，所以空标记为1	 
    for(t=0;t<csize;t++)
    {   
		if(size==12)temp=asc2_1206[chr][t]; 	 	//调用1206字体
		else if(size==16)temp=asc2_1608[chr][t];	//调用1608字体
		else if(size==24)temp=asc2_2412[chr][t];	//调用2412字体
		else return;								//没有的字库
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);//取出一个字节的最高位
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}
//m^n函数
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
	}
} 
//显示字符串
//x,y:起点坐标  
//size:字体大小 
//*p:字符串起始地址 


void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size)
{	
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>(128-(size/2))){x=0;y+=size;}
        if(y>(64-size)){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,size,1);	 
        x+=size/2;
        p++;
    }  
	
}

void OLED_ShowGBK(u8 x, u8 y,  u8 num, u8 size,u8 mode)
{
    u8 temp,t,t1;
    u8 y0=y;
	//u8 size = 16;
    u8 csize=(size/8 + ((size%8)?1:0)) * size;     
              
    for(t=0;t<csize;t++)
    {  
		  
        if(size==12)   
					temp = asc12_chinese[num][t];       
        else if(size==16)
					temp = asc16_chinese[num][t];
				else return;                              
		for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)OLED_DrawPoint(x,y,mode);
            else OLED_DrawPoint(x,y,!mode);
            temp<<=1;  //对于每一个字节从高位开始取
            y++; //竖写
            if((y-y0)==size) //超出竖写范围
            {
                y=y0;
                x++;
                break;
            }
        }    
    } 	
}



//初始化SSD1306					    
void OLED_Init(void)
{ 	
 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE);	 //使能PC,D,G端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;	 //PD3,PD6推挽输出  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOD, &GPIO_InitStructure);	  //初始化GPIOD3,6
 	GPIO_SetBits(GPIOD,GPIO_Pin_3|GPIO_Pin_6);	//PD3,PD6 输出高

 #if OLED_MODE==1
 
 	GPIO_InitStructure.GPIO_Pin =0xFF; //PC0~7 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,0xFF); //PC0~7输出高

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //PG13,14,15 OUT推挽输出
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PG13,14,15 OUT  输出高

 #else
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;				 //PC0,1 OUT推挽输出
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1);						 //PC0,1 OUT  输出高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 //PG15 OUT推挽输出	  RST
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_15);						 //PG15 OUT  输出高


 #endif
  							  
	OLED_CS=1;
	OLED_RS=1;	 
	
	OLED_RST=0;
	delay_ms(100);
	OLED_RST=1; 
					  
	OLED_WR_Byte(0xAE,OLED_CMD); //关闭显示
	OLED_WR_Byte(0xD5,OLED_CMD); //设置时钟分频因子,震荡频率
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],分频因子;[7:4],震荡频率
	OLED_WR_Byte(0xA8,OLED_CMD); //设置驱动路数
	OLED_WR_Byte(0X3F,OLED_CMD); //默认0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //设置显示偏移
	OLED_WR_Byte(0X00,OLED_CMD); //默认为0

	OLED_WR_Byte(0x40,OLED_CMD); //设置显示开始行 [5:0],行数.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //电荷泵设置
	OLED_WR_Byte(0x14,OLED_CMD); //bit2，开启/关闭
	OLED_WR_Byte(0x20,OLED_CMD); //设置内存地址模式
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	OLED_WR_Byte(0xA1,OLED_CMD); //段重定义设置,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0,OLED_CMD); //设置COM扫描方向;bit3:0,普通模式;1,重定义模式 COM[N-1]->COM0;N:驱动路数
	OLED_WR_Byte(0xDA,OLED_CMD); //设置COM硬件引脚配置
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]配置
		 
	OLED_WR_Byte(0x81,OLED_CMD); //对比度设置
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9,OLED_CMD); //设置预充电周期
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //设置VCOMH 电压倍率
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	OLED_WR_Byte(0xA6,OLED_CMD); //设置显示方式;bit0:1,反相显示;0,正常显示	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //开启显示	 
	OLED_Clear();
}  





void OLED_ShowTime(void)
{
      u8 t=0;
           
      while(1)
	{								    
		          
            if(t!=calendar.sec)
		{
			t=calendar.sec;			
                  OLED_ShowNum(0,0,calendar.w_year,4,16);
                  OLED_ShowChar(32,0,'-',16,1);
			if(calendar.w_month<=9)
                  {
                        OLED_ShowNum(40,0,calendar.w_month,2,16);	
                        OLED_ShowChar(40,0,'0',16,1);
                  }
                  else
                  {
                          OLED_ShowNum(40,0,calendar.w_month,2,16);
                  }      
                  OLED_ShowChar(56,0,'-',16,1);
			if(calendar.w_date<=9)
                  {
                        OLED_ShowNum(64,0,calendar.w_date,2,16);	 
                        OLED_ShowChar(64,0,'0',16,1);
                  }
                  else
                  {
                        OLED_ShowNum(64,0,calendar.w_date,2,16);
                  }
                         
                  switch(calendar.week)
			{
				case 0:
					OLED_ShowString(0,20,"Sunday",16);
					break;
				case 1:
					OLED_ShowString(0,20,"Monday",16);
					break;
				case 2:
					OLED_ShowString(0,20,"Tuesday",16);
					break;
				case 3:
					OLED_ShowString(0,20,"Wednesday",16);
					break;
				case 4:
					OLED_ShowString(0,20,"Thursday",16);
					break;
				case 5:
					OLED_ShowString(0,20,"Friday",16);
					break;
				case 6:
					OLED_ShowString(0,20,"Saturday",16);
					break;  
			}
			
                  if(calendar.hour<=9)
                  {
                        OLED_ShowNum(0,40,calendar.hour,2,16);	
                        OLED_ShowChar(0,40,'0',16,1);
                  }
                  else
                  {
                        OLED_ShowNum(0,40,calendar.hour,2,16);
                  }
                  
                  OLED_ShowChar(16,40,':',16,1);
			 
                  if(calendar.min<=9)
                  {
                        OLED_ShowNum(24,40,calendar.min,2,16);	
                        OLED_ShowChar(24,40,'0',16,1);
                  }
                  else
                  {
                        OLED_ShowNum(24,40,calendar.min,2,16);
                  }
                  
                  OLED_ShowChar(40,40,':',16,1);
			
                  if(calendar.sec<=9)
                  {
                        OLED_ShowNum(48,40,calendar.sec,2,16);
                        OLED_ShowChar(48,40,'0',16,1);
                  }
                  else
                  {
                        OLED_ShowNum(48,40,calendar.sec,2,16);
                  }
                  			
		}	
		OLED_Refresh_Gram();	
            delay_ms(10);								             
            if(TPAD_Scan(0))
            {
                  break;          
            }
      }
         
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	OLED_WR_Byte(0xb0+y,0);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,0);
	OLED_WR_Byte((x&0x0f)|0x01,0);
}


//extern const unsigned char bmp[1024];
void OLED_DrawBMP(const unsigned char BMP[])
{
	unsigned int i=0,j=0;
	unsigned char x=0,y=0;
      unsigned char temp;
/*if(y1%8==0)
		y = y1/8;
  else
		
      y = y1/8 + 1;*/
      for(i=0;i<1024;i++)
      {
            temp = BMP[i];
            for(j=0;j<8;j++)
            {
                  if(temp&0x80)OLED_DrawPoint(x,y,1);
                  else OLED_DrawPoint(x,y,0);
                  temp<<=1;  //对于每一个字节从高位开始取
                  y++; //竖写
            }
            if((i+1)%8==0)
            {
                   y=0;
                   x++;                 
            }
      }          
            
}


extern const unsigned char bmp[1024];
void OLED_ShowSurface(u8 x)
{
      switch(x)
      {
            case 0:
                  OLED_Clear();
                  OLED_ShowString(16,20, "WELCOME",24);
                  OLED_ShowString(40,48,"L & Z",12);
                  OLED_Refresh_Gram();		    //更新显示到OLED
                  break;
            case 1:
                  OLED_Clear();
			OLED_ShowGBK(0,0,0,16,1);//学
			OLED_ShowGBK(16,0,1,16,1);//生
			OLED_ShowGBK(32,0,2,16,1);//名
			OLED_ShowGBK(48,0,3,16,1);//单
			OLED_ShowString(0,17, "1:",12);
			OLED_ShowGBK(12,17,4,12,1);//张
			OLED_ShowGBK(24,17,5,12,1);//三
			OLED_ShowString(0,30, "2:",12);
			OLED_ShowGBK(12,30,6,12,1);//李
			OLED_ShowGBK(24,30,7,12,1);//四
			OLED_Refresh_Gram();		    //更新显示到OLED
                  break;
            case 2:
                  OLED_Clear();
			OLED_ShowGBK(0,0,4,16,1);//张
			OLED_ShowGBK(16,0,5,16,1);//三
			OLED_ShowGBK(32,0,6,16,1);//成
			OLED_ShowGBK(48,0,7,16,1);//绩
			OLED_ShowGBK(0,17,8,12,1);//高
			OLED_ShowGBK(12,17,9,12,1);//数
			OLED_ShowString(24,17, ": 100",12);
			OLED_ShowGBK(0,30,10,12,1);//线
			OLED_ShowGBK(12,30,11,12,1);//代
			OLED_ShowString(24,30, ": 100",12);
			OLED_Refresh_Gram();
                  break;//更新显示到OLED
            case 3:
                  OLED_Clear();
				OLED_ShowGBK(0,0,8,16,1);//李
				OLED_ShowGBK(16,0,9,16,1);//四
				OLED_ShowGBK(32,0,6,16,1);//成
				OLED_ShowGBK(48,0,7,16,1);//绩
				OLED_ShowGBK(0,17,8,12,1);//高
				OLED_ShowGBK(12,17,9,12,1);//数
				OLED_ShowString(24,17, ": 59",12);
				OLED_ShowGBK(0,30,10,12,1);//线
				OLED_ShowGBK(12,30,11,12,1);//代
				OLED_ShowString(24,30, ": 59",12);
				OLED_Refresh_Gram();		    //更新显示到OLED
				break;
            case 4:
                        OLED_Clear();
                        OLED_DrawBMP(bmp);
                        OLED_Refresh_Gram();		    //更新显示到OLED
				break;
      }
      
}























