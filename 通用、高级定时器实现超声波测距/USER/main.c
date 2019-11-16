#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
 
#define jishu_pinlv_psc 71

/************************************************
 printfʹ�ô���2��������115200 	
 USART2_TX GPIOA.2��USART2_RX GPIOA.3
 
 ��������ࡾ���벶��
 TIM1_CH1(PA8) 
 TIM3_CH3(PB0)
************************************************/

u32 Distance;
int main(void)
{		
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	
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


