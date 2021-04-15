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
//SSD1306 OLED ����IC��������
//������ʽ:8080����/4�ߴ���



//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
u8 OLED_GRAM[128][8];	 




//�����Դ浽OLED		 
void OLED_Refresh_Gram(void)
{
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //����ҳ��ַ��0~7��
		OLED_WR_Byte (0x00,OLED_CMD);      //������ʾλ�á��е͵�ַ
		OLED_WR_Byte (0x10,OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA); 
	}   
}
#if OLED_MODE==1	//8080���� 
//��SSD1306д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
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
//��SSD1306д��һ���ֽڡ�
//dat:Ҫд�������/����
//cmd:����/�����־ 0,��ʾ����;1,��ʾ����;
void OLED_WR_Byte(u8 dat,u8 cmd)
{	
	u8 i;			  
	OLED_RS=cmd; //д���� 
	OLED_CS=0;		  
	for(i=0;i<8;i++)
	{			  
		OLED_SCLK=0;
		if(dat&0x80)OLED_SDIN=1; //���������,
		else OLED_SDIN=0;
		OLED_SCLK=1;
		dat<<=1;   
	}				 
	OLED_CS=1;		  
	OLED_RS=1;   	  
} 
#endif
	  	  
//����OLED��ʾ    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC����
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!	  
void OLED_Clear(void)  
{  
	u8 i,n;  
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0X00;  
	OLED_Refresh_Gram();//������ʾ
}
//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���				   
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//������Χ��.
	pos=7-y/8;
	bx=y%8;
	temp=1<<(7-bx);
	if(t)OLED_GRAM[x][pos]|=temp;
	else OLED_GRAM[x][pos]&=~temp;	    
}
//x1,y1,x2,y2 �������ĶԽ�����
//ȷ��x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,���;1,���	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	u8 x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	OLED_Refresh_Gram();//������ʾ
}
//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ				 
//size:ѡ������ 12/16/24
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{      			    
	u8 temp,t,t1;
	u8 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���
	chr=chr-' ';//�õ�ƫ�ƺ��ֵ	ASCLL��ӡ�ַ��ӿո�Ҳ����ʮ����32��ʼ�����Կձ��Ϊ1	 
    for(t=0;t<csize;t++)
    {   
		if(size==12)temp=asc2_1206[chr][t]; 	 	//����1206����
		else if(size==16)temp=asc2_1608[chr][t];	//����1608����
		else if(size==24)temp=asc2_2412[chr][t];	//����2412����
		else return;								//û�е��ֿ�
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);//ȡ��һ���ֽڵ����λ
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
//m^n����
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//��ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ	0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);	 		  
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
//��ʾ�ַ���
//x,y:�������  
//size:�����С 
//*p:�ַ�����ʼ��ַ 


void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
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
            temp<<=1;  //����ÿһ���ֽڴӸ�λ��ʼȡ
            y++; //��д
            if((y-y0)==size) //������д��Χ
            {
                y=y0;
                x++;
                break;
            }
        }    
    } 	
}



//��ʼ��SSD1306					    
void OLED_Init(void)
{ 	
 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PC,D,G�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;	 //PD3,PD6�������  
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
 	GPIO_Init(GPIOD, &GPIO_InitStructure);	  //��ʼ��GPIOD3,6
 	GPIO_SetBits(GPIOD,GPIO_Pin_3|GPIO_Pin_6);	//PD3,PD6 �����

 #if OLED_MODE==1
 
 	GPIO_InitStructure.GPIO_Pin =0xFF; //PC0~7 OUT�������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,0xFF); //PC0~7�����

 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //PG13,14,15 OUT�������
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);						 //PG13,14,15 OUT  �����

 #else
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;				 //PC0,1 OUT�������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOC,GPIO_Pin_0|GPIO_Pin_1);						 //PC0,1 OUT  �����

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 //PG15 OUT�������	  RST
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_15);						 //PG15 OUT  �����


 #endif
  							  
	OLED_CS=1;
	OLED_RS=1;	 
	
	OLED_RST=0;
	delay_ms(100);
	OLED_RST=1; 
					  
	OLED_WR_Byte(0xAE,OLED_CMD); //�ر���ʾ
	OLED_WR_Byte(0xD5,OLED_CMD); //����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_WR_Byte(80,OLED_CMD);   //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_WR_Byte(0xA8,OLED_CMD); //��������·��
	OLED_WR_Byte(0X3F,OLED_CMD); //Ĭ��0X3F(1/64) 
	OLED_WR_Byte(0xD3,OLED_CMD); //������ʾƫ��
	OLED_WR_Byte(0X00,OLED_CMD); //Ĭ��Ϊ0

	OLED_WR_Byte(0x40,OLED_CMD); //������ʾ��ʼ�� [5:0],����.
													    
	OLED_WR_Byte(0x8D,OLED_CMD); //��ɱ�����
	OLED_WR_Byte(0x14,OLED_CMD); //bit2������/�ر�
	OLED_WR_Byte(0x20,OLED_CMD); //�����ڴ��ַģʽ
	OLED_WR_Byte(0x02,OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	OLED_WR_Byte(0xA1,OLED_CMD); //���ض�������,bit0:0,0->0;1,0->127;
	OLED_WR_Byte(0xC0,OLED_CMD); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
	OLED_WR_Byte(0xDA,OLED_CMD); //����COMӲ����������
	OLED_WR_Byte(0x12,OLED_CMD); //[5:4]����
		 
	OLED_WR_Byte(0x81,OLED_CMD); //�Աȶ�����
	OLED_WR_Byte(0xEF,OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_WR_Byte(0xD9,OLED_CMD); //����Ԥ�������
	OLED_WR_Byte(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB,OLED_CMD); //����VCOMH ��ѹ����
	OLED_WR_Byte(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA4,OLED_CMD); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	OLED_WR_Byte(0xA6,OLED_CMD); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ	    						   
	OLED_WR_Byte(0xAF,OLED_CMD); //������ʾ	 
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

void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
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
                  temp<<=1;  //����ÿһ���ֽڴӸ�λ��ʼȡ
                  y++; //��д
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
                  OLED_Refresh_Gram();		    //������ʾ��OLED
                  break;
            case 1:
                  OLED_Clear();
			OLED_ShowGBK(0,0,0,16,1);//ѧ
			OLED_ShowGBK(16,0,1,16,1);//��
			OLED_ShowGBK(32,0,2,16,1);//��
			OLED_ShowGBK(48,0,3,16,1);//��
			OLED_ShowString(0,17, "1:",12);
			OLED_ShowGBK(12,17,4,12,1);//��
			OLED_ShowGBK(24,17,5,12,1);//��
			OLED_ShowString(0,30, "2:",12);
			OLED_ShowGBK(12,30,6,12,1);//��
			OLED_ShowGBK(24,30,7,12,1);//��
			OLED_Refresh_Gram();		    //������ʾ��OLED
                  break;
            case 2:
                  OLED_Clear();
			OLED_ShowGBK(0,0,4,16,1);//��
			OLED_ShowGBK(16,0,5,16,1);//��
			OLED_ShowGBK(32,0,6,16,1);//��
			OLED_ShowGBK(48,0,7,16,1);//��
			OLED_ShowGBK(0,17,8,12,1);//��
			OLED_ShowGBK(12,17,9,12,1);//��
			OLED_ShowString(24,17, ": 100",12);
			OLED_ShowGBK(0,30,10,12,1);//��
			OLED_ShowGBK(12,30,11,12,1);//��
			OLED_ShowString(24,30, ": 100",12);
			OLED_Refresh_Gram();
                  break;//������ʾ��OLED
            case 3:
                  OLED_Clear();
				OLED_ShowGBK(0,0,8,16,1);//��
				OLED_ShowGBK(16,0,9,16,1);//��
				OLED_ShowGBK(32,0,6,16,1);//��
				OLED_ShowGBK(48,0,7,16,1);//��
				OLED_ShowGBK(0,17,8,12,1);//��
				OLED_ShowGBK(12,17,9,12,1);//��
				OLED_ShowString(24,17, ": 59",12);
				OLED_ShowGBK(0,30,10,12,1);//��
				OLED_ShowGBK(12,30,11,12,1);//��
				OLED_ShowString(24,30, ": 59",12);
				OLED_Refresh_Gram();		    //������ʾ��OLED
				break;
            case 4:
                        OLED_Clear();
                        OLED_DrawBMP(bmp);
                        OLED_Refresh_Gram();		    //������ʾ��OLED
				break;
      }
      
}























