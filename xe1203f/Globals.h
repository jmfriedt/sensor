/*******************************************************************
** File        : Globals.h                                        **
** Description : Definition of common typedefs, defines, macros,  **
**               etc ...                                          **
*******************************************************************/
#ifndef __GLOBALS__
#define __GLOBALS__
#endif
/*******************************************************************
** Types definitions                                              **
*******************************************************************/
#include "Types.h"

#define CORTEX
#ifdef CORTEX
#define set_bit(GPIOx, GPIO_Pin) GPIO_SetBits(GPIOx, GPIO_Pin)
#define clear_bit( GPIOx, GPIO_Pin) GPIO_ResetBits(GPIOx, GPIO_Pin)
#define check_bit(GPIOx, GPIO_Pin) GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin)
#define SrtInit()        GPIO_ResetBits(GPIOB, SO)
#define SrtSetSCK(level) (level) ? (set_bit(GPIOB, SCK)) : (clear_bit(GPIOB, SCK))
#define SrtSetSI(level)  (level) ? (set_bit(GPIOB, SI)) : (clear_bit(GPIOB, SI))
#define SrtCheckSO()     check_bit(GPIOB, SO)
#endif
/*******************************************************************
** Definition des déclarations                                    **
*******************************************************************/
#define _U8	unsigned char
#define _S8	char
#define _U16	unsigned short
#define _S16	short
#define _S32	long
#define _U32	unsigned long
#define _F32	double


