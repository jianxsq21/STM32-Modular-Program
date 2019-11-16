#ifndef __TIMERs_H
#define __TIMERs_H
#include "sys.h"
#include "stm32f10x.h"

extern u32 Distance;

//******TIM1���***********//

void TIM1_Cap_Init(u16 arr,u16 psc);
void Read_TIM1Distane(void);

//******TIM3���***********//
void TIM3_Cap_Init(u16 arr,u16 psc);
void Read_TIM3Distane(void);

#endif
