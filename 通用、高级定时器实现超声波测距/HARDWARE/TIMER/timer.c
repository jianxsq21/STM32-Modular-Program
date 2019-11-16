#include "timer.h"
#include "delay.h"
#include "usart.h"


//**************以下TIM1实现超声波测距****************//
u16 TIM1CH1_CAPTURE_STA,TIM1CH1_CAPTURE_VAL;
u32 TIM1_CH1_Time;	//CH1测量方波1的高电平时间，用于计算占空比

void TIM1_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟


	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //PB1输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;     //2M
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);//设置缺省值,这一步最好加上
	TIM_TimeBaseStructure.TIM_Period = arr; //自动重装载寄存器周期的值，溢出值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //时钟频率预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:输入捕获模式用来滤波
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;//设置重复溢出次数，就是多少次溢出后进入中断，一般为0，只有高级定时器才有用
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 


	TIM_ICStructInit(&TIM_ICInitStructure);//设置缺省值,这一步最好加上
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	  //配置输入分频,不分频 
	TIM_ICInitStructure.TIM_ICFilter = 0x00;	  //IC1F=0000 配置输入滤波器 0不滤波
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1; //IC1映射到TI1上，这四句不能合并
	TIM_ICInit(TIM1, &TIM_ICInitStructure);

	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);   

	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC1,ENABLE);   //允许更新中断，允许CC1IE,CC2IE,CC3IE,CC4IE捕获中断	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//主输出使能
	TIM_Cmd(TIM1, ENABLE); 		//使能定时器1
}

void Read_TIM1Distane(void)
{   
	PBout(1)=1;
	delay_us(15);  
	PBout(1)=0;	
	
	if(TIM1CH1_CAPTURE_STA&0X80)
	{
		Distance=TIM1CH1_CAPTURE_STA&0X3F;
		Distance*=65536;					        //溢出时间总和
		Distance+=TIM1CH1_CAPTURE_VAL;		//得到总的高电平时间
		Distance = Distance*170/1000;
		printf("%d \r\n",Distance);
		TIM1CH1_CAPTURE_STA = 0;
	}
}

void TIM1_CC_IRQHandler(void)
{
	if((TIM1CH1_CAPTURE_STA&0X80) == 0)
	{
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
		{	
			  //通道1					
				if(TIM1CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
				{
					if((TIM1CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
					{
						TIM1CH1_CAPTURE_STA|=0X80;
						TIM1CH1_CAPTURE_VAL = 0xFFFF;					
					}
					else
					{
						TIM1CH1_CAPTURE_STA++;
					}
				}
		}

		//通道1
		if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET) 		//捕获1发生捕获事件
		{	
			if (TIM1CH1_CAPTURE_STA & 0X40)		//捕获到一个下降沿
			{
				TIM1CH1_CAPTURE_STA|=0X80;	
				TIM1CH1_CAPTURE_VAL = TIM_GetCapture1(TIM1);//记录下此时的定时器计数值
				TIM_OC1PolarityConfig(TIM1, TIM_ICPolarity_Rising); //设置为上升沿捕获					
			}
			else //发生捕获时间但不是下降沿，第一次捕获到上升沿，记录此时的定时器计数值
			{
				TIM1CH1_CAPTURE_STA = 0;
				TIM1CH1_CAPTURE_VAL = 0;
				TIM1CH1_CAPTURE_STA |= 0X40;		//标记已捕获到上升沿
				TIM_SetCounter(TIM1, 0);		//清空计数器
				TIM_OC1PolarityConfig(TIM1, TIM_ICPolarity_Falling);//设置为下降沿捕获
				
			}
		}
	}
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC1|TIM_IT_Update); 		//清除中断标志位		
}









//**************以下TIM3实现超声波测距****************//
u16 TIM3CH3_CAPTURE_STA,TIM3CH3_CAPTURE_VAL;

void TIM3_Cap_Init(u16 arr,u16 psc)	
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM3_ICInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能TIM3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PB0 输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //PB1输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;     //2M
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//初始化定时器3 TIM3	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	//初始化TIM3输入捕获参数
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=03 	选择输入端 IC3映射到TI1上
	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//配置输入滤波器 不滤波
	TIM_ICInit(TIM3, &TIM3_ICInitStructure);

	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 	
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC3,ENABLE);//允许更新中断 ,允许CC3IE捕获中断	
	TIM_Cmd(TIM3,ENABLE ); 	//使能定时器3
}

void Read_TIM3Distane(void)
{   
	PBout(1)=1;
	delay_us(15);  
	PBout(1)=0;	
	
	if(TIM3CH3_CAPTURE_STA&0X80)//成功捕获到了一次高电平
	{
		Distance=TIM3CH3_CAPTURE_STA&0X3F;
		Distance*=65536;					        //溢出时间总和
		Distance+=TIM3CH3_CAPTURE_VAL;		//得到总的高电平时间
		Distance=Distance*170/1000;
		printf("%d \r\n",Distance);
		TIM3CH3_CAPTURE_STA=0;			//开启下一次捕获
	}				
}

void TIM3_IRQHandler(void)
{ 		    		  			    
	if((TIM3CH3_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)//溢出
		{	    
			if(TIM3CH3_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM3CH3_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM3CH3_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM3CH3_CAPTURE_VAL=0XFFFF;
				}
				else 
					TIM3CH3_CAPTURE_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)//捕获3发生捕获事件
		{	
			if(TIM3CH3_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM3CH3_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM3CH3_CAPTURE_VAL=TIM_GetCapture3(TIM3);	//获取当前的捕获值.
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Rising); //设置为上升沿捕获
			}
			else  								//还未开始,第一次捕获上升沿
			{
				TIM3CH3_CAPTURE_STA=0;			//清空
				TIM3CH3_CAPTURE_VAL=0;
				TIM3CH3_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
				TIM_SetCounter(TIM3, 0);		//清空计数器
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Falling);//设置为下降沿捕获
			}		    
		}			     	    					   
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update|TIM_IT_CC3); 		//清除中断标志位
}
