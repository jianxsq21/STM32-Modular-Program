#ifndef __TIMERs_H
#define __TIMERs_H
#include "sys.h"
#include "stm32f10x.h"

extern u32 Distance;

//******TIM1相关***********//

void TIM1_Cap_Init(u16 arr,u16 psc);
void Read_TIM1Distane(void);

//******TIM3相关***********//
void TIM3_Cap_Init(u16 arr,u16 psc);
void Read_TIM3Distane(void);

#endif
