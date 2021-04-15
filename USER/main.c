#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "oled.h"
#include "tpad.h"
#include "usart.h" 
#include "rtc.h"
#include "lcd.h"
#include "paj7620u2.h"


extern void gesture_Init(void);
 
 int main(void)
 {	
	int8_t i=0;
      u8 status;
      u8 data[2]={0x00};
	u16 gesture_data;
	u8 flag=0;
      
      Stm32_Clock_Init(9);
      uart_init(115200);//串口初始化为115200
      delay_init();	    	 //延时函数初始化	  
	KEY_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
 	LED_Init();			     //LED端口初始化
      OLED_Init();			//初始化OLED      	   
	OLED_Refresh_Gram();//更新显示到OLED 		
      RTC_Init();
      TPAD_Init(6);	//初始化触摸按键
      paj7620u2_init();
      LCD_ShowString(30,140,200,16,16,"PAJ7620U2 OK");
      gesture_Init();
      while(1)
	{		
            status = GS_Read_nByte(PAJ_GET_INT_FLAG1,2,&data[0]);//读取手势状态			
		if(!status)
		{   
			gesture_data =(u16)data[1]<<8 | data[0];
			if(gesture_data) 
			{
				
                        switch(gesture_data)
				{
					case GES_UP:                                                              
                                    flag=1;      
                                    break; 
					case GES_DOWM:               
                                    flag=1;      
                                    break; 
					case GES_LEFT:                   
                                    flag=1;      
                                    i = i + 1;
                                    break; 
					case GES_RIGHT:                    
                                    flag=1;      
                                    i = i - 1;                                     
                                    break; 
					case GES_FORWARD:               
                                    LED0=0;
                                    flag=1;      
                                    break; 
					case GES_BACKWARD:             
                                    flag=1; 
                                    LED0=1;
                                    break; 
					case GES_CLOCKWISE:            
                                    flag=1;      
                                    break; //顺时针
					case GES_COUNT_CLOCKWISE:  
                                    flag=1;      
                                    break; //逆时针
					case GES_WAVE:                
                                    flag=1;      
                                    break; //挥动
					default:  flag=0; break;
					
				}
                   if(flag)
                   {   					
				delay_ms(200);
				flag=0;
                        if(i>5) i = 0;                      
                        if(i<0) i = 5;                             
                        if(i == 5)
                        {
                              OLED_Clear();
                              OLED_ShowTime();
                              delay_ms(10);
                        }
                      else
                        {
                            OLED_ShowSurface(i);
                        }
                  }	
                  
                }
            }
                       
      
      }
}
