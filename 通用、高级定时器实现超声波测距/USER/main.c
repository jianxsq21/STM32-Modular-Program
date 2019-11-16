#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
 
#define jishu_pinlv_psc 71

/************************************************
 printf使用串口2、波特率115200 	
 USART2_TX GPIOA.2，USART2_RX GPIOA.3
 
 超声波测距【输入捕获】
 TIM1_CH1(PA8) 
 TIM3_CH3(PB0)
************************************************/

u32 Distance;
int main(void)
{		
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
	
	LED_Init();
	TIM3_Cap_Init(0xFFFF,71);
	
	
	printf("ok\r\n");
	LED = 0;
	while(1)
	{
		Read_TIM3Distane();
		delay_ms(200);
	}
}


