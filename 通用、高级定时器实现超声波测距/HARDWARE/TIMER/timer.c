#include "timer.h"
#include "delay.h"
#include "usart.h"


//**************����TIM1ʵ�ֳ��������****************//
u16 TIM1CH1_CAPTURE_STA,TIM1CH1_CAPTURE_VAL;
u32 TIM1_CH1_Time;	//CH1��������1�ĸߵ�ƽʱ�䣬���ڼ���ռ�ձ�

void TIM1_Cap_Init(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ��


	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //PB1��� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;     //2M
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);//����ȱʡֵ,��һ����ü���
	TIM_TimeBaseStructure.TIM_Period = arr; //�Զ���װ�ؼĴ������ڵ�ֵ�����ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //ʱ��Ƶ��Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:���벶��ģʽ�����˲�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;//�����ظ�������������Ƕ��ٴ����������жϣ�һ��Ϊ0��ֻ�и߼���ʱ��������
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 


	TIM_ICStructInit(&TIM_ICInitStructure);//����ȱʡֵ,��һ����ü���
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	  //���������Ƶ,����Ƶ 
	TIM_ICInitStructure.TIM_ICFilter = 0x00;	  //IC1F=0000 ���������˲��� 0���˲�
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1; //IC1ӳ�䵽TI1�ϣ����ľ䲻�ܺϲ�
	TIM_ICInit(TIM1, &TIM_ICInitStructure);

	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);   

	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC1,ENABLE);   //��������жϣ�����CC1IE,CC2IE,CC3IE,CC4IE�����ж�	
	TIM_CtrlPWMOutputs(TIM1,ENABLE);	//�����ʹ��
	TIM_Cmd(TIM1, ENABLE); 		//ʹ�ܶ�ʱ��1
}

void Read_TIM1Distane(void)
{   
	PBout(1)=1;
	delay_us(15);  
	PBout(1)=0;	
	
	if(TIM1CH1_CAPTURE_STA&0X80)
	{
		Distance=TIM1CH1_CAPTURE_STA&0X3F;
		Distance*=65536;					        //���ʱ���ܺ�
		Distance+=TIM1CH1_CAPTURE_VAL;		//�õ��ܵĸߵ�ƽʱ��
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
			  //ͨ��1					
				if(TIM1CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
				{
					if((TIM1CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
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

		//ͨ��1
		if (TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET) 		//����1���������¼�
		{	
			if (TIM1CH1_CAPTURE_STA & 0X40)		//����һ���½���
			{
				TIM1CH1_CAPTURE_STA|=0X80;	
				TIM1CH1_CAPTURE_VAL = TIM_GetCapture1(TIM1);//��¼�´�ʱ�Ķ�ʱ������ֵ
				TIM_OC1PolarityConfig(TIM1, TIM_ICPolarity_Rising); //����Ϊ�����ز���					
			}
			else //��������ʱ�䵫�����½��أ���һ�β��������أ���¼��ʱ�Ķ�ʱ������ֵ
			{
				TIM1CH1_CAPTURE_STA = 0;
				TIM1CH1_CAPTURE_VAL = 0;
				TIM1CH1_CAPTURE_STA |= 0X40;		//����Ѳ���������
				TIM_SetCounter(TIM1, 0);		//��ռ�����
				TIM_OC1PolarityConfig(TIM1, TIM_ICPolarity_Falling);//����Ϊ�½��ز���
				
			}
		}
	}
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC1|TIM_IT_Update); 		//����жϱ�־λ		
}









//**************����TIM3ʵ�ֳ��������****************//
u16 TIM3CH3_CAPTURE_STA,TIM3CH3_CAPTURE_VAL;

void TIM3_Cap_Init(u16 arr,u16 psc)	
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef  TIM3_ICInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ��TIM3ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PB0 ����  
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     //PB1��� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;     //2M
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//��ʼ����ʱ��3 TIM3	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	//��ʼ��TIM3���벶�����
	TIM3_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=03 	ѡ������� IC3ӳ�䵽TI1��
	TIM3_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
	TIM3_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM3_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
	TIM3_ICInitStructure.TIM_ICFilter = 0x00;//���������˲��� ���˲�
	TIM_ICInit(TIM3, &TIM3_ICInitStructure);

	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 	
	TIM_ITConfig(TIM3,TIM_IT_Update|TIM_IT_CC3,ENABLE);//��������ж� ,����CC3IE�����ж�	
	TIM_Cmd(TIM3,ENABLE ); 	//ʹ�ܶ�ʱ��3
}

void Read_TIM3Distane(void)
{   
	PBout(1)=1;
	delay_us(15);  
	PBout(1)=0;	
	
	if(TIM3CH3_CAPTURE_STA&0X80)//�ɹ�������һ�θߵ�ƽ
	{
		Distance=TIM3CH3_CAPTURE_STA&0X3F;
		Distance*=65536;					        //���ʱ���ܺ�
		Distance+=TIM3CH3_CAPTURE_VAL;		//�õ��ܵĸߵ�ƽʱ��
		Distance=Distance*170/1000;
		printf("%d \r\n",Distance);
		TIM3CH3_CAPTURE_STA=0;			//������һ�β���
	}				
}

void TIM3_IRQHandler(void)
{ 		    		  			    
	if((TIM3CH3_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)//���
		{	    
			if(TIM3CH3_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM3CH3_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM3CH3_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM3CH3_CAPTURE_VAL=0XFFFF;
				}
				else 
					TIM3CH3_CAPTURE_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)//����3���������¼�
		{	
			if(TIM3CH3_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM3CH3_CAPTURE_STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
				TIM3CH3_CAPTURE_VAL=TIM_GetCapture3(TIM3);	//��ȡ��ǰ�Ĳ���ֵ.
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Rising); //����Ϊ�����ز���
			}
			else  								//��δ��ʼ,��һ�β���������
			{
				TIM3CH3_CAPTURE_STA=0;			//���
				TIM3CH3_CAPTURE_VAL=0;
				TIM3CH3_CAPTURE_STA|=0X40;		//��ǲ�����������
				TIM_SetCounter(TIM3, 0);		//��ռ�����
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Falling);//����Ϊ�½��ز���
			}		    
		}			     	    					   
	}
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update|TIM_IT_CC3); 		//����жϱ�־λ
}
