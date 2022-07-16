/*
: Hello from a STM32
Sat Nov 13 14:06:18 2010
: 0000316
Sat Nov 13 14:06:28 2010
: 0000316
Sat Nov 13 14:08:34 2010
: 
Sat Nov 13 14:08:34 2010
: Hello from a STM32
Sat Nov 13 14:08:34 2010
*/

// DOIT etre compile' avec l'option -msoft-float
// stm32flash /dev/ttyUSB0 -w main.bin 

//******************* (C) COPYRIGHT 2007 STMicroelectronics ********************
//Author             : MCD Application Team

#include "stm32f10x_lib.h"
#include "stm32f10x_adc.h"
#include "comm.h"                           // jmf RS232
#include <math.h> 

int __errno() {return(0);}

void RCC_Configuration(void);
void NVIC_Configuration(void);
void Delay(vu32 nCount);

GPIO_InitTypeDef GPIO_InitStructure;        // jmf: must be global variables ?!
USART_InitTypeDef USART_InitStructure;
ADC_InitTypeDef ADC_InitStructure;  

u16 readADC1(u8 channel)                    // jmf convert ADC1
{ADC_RegularChannelConfig(ADC1,channel,1,ADC_SampleTime_239Cycles5);
 ADC_SoftwareStartConvCmd(ADC1,ENABLE);
 while (ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET); // EOC is set @ end of cv
 return(ADC_GetConversionValue(ADC1));
}

void writeDECi(unsigned int temp)
{unsigned int tmp;
  tmp=(temp/1000000);comm_put(tmp+'0');temp-=tmp*1000000;
  tmp=(temp/100000); comm_put(tmp+'0');temp-=tmp*100000;
  tmp=(temp/10000);  comm_put(tmp+'0');temp-=tmp*10000;
  tmp=(temp/1000);   comm_put(tmp+'0');temp-=tmp*1000;
  tmp=(temp/100);    comm_put(tmp+'0');temp-=tmp*100;
  tmp=(temp/10);     comm_put(tmp+'0');temp-=tmp*10;
  comm_put(temp+'0');
}
  
void writeHEXi(unsigned int temp)
{unsigned int tmp;
  tmp=(temp&0xf000)>>12;
  if (tmp<10) comm_put(tmp+'0'); else comm_put(tmp+'A'-10);
  tmp=(temp&0x0f00)>8;
  if (tmp<10) comm_put(tmp+'0'); else comm_put(tmp+'A'-10);
  tmp=(temp&0x00f0)>>4;
  if (tmp<10) comm_put(tmp+'0'); else comm_put(tmp+'A'-10);
  tmp=(temp&0x000f);
  if (tmp<10) comm_put(tmp+'0'); else comm_put(tmp+'A'-10);
}

int main(void)
{int i=0;
 unsigned int sA,A=100000;
/* float fA0=-250.4;      // 0.1 pres
 float fA1=160000.66;   // % 0.01 pres
 float fA2=-0.17212345; // % 1e-4 pres
 float fdf=500000.;
*/
 double fA,fsA;
 //unsigned int A0=(unsigned int)(fA0*10.);
 //unsigned int A1=(unsigned int)(fA1*10000.);
 //unsigned int A2=(unsigned int)(fA2*100000.);
 //unsigned int df=(unsigned int)(fdf/10.);
 const char welcome[] = "\r\nHello from a STM32\0" ;
 const char fini[] = "The end\r\n\0" ;
#ifdef DEBUG
//  debug();
#endif

  /* Configure the system clocks */
  RCC_Configuration(); // jmf: added ADC and USART
  
  /* NVIC Configuration */
  NVIC_Configuration();

  /* Configure PC.4 as Output push-pull */
  /* Enable GPIOC clock */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure USART1 TX (PA.09) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* Configure USART1 RX (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
  // USART configured 57600, N81, no hw flow ctrl, enable Tx & Rx
  USART_InitStructure.USART_BaudRate = 57600; //115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);

  comm_puts(welcome);comm_put('\r');comm_put('\n');
//  writeDECi(sizeof(A)); comm_put(' ');
//  writeDECi(1234567);   comm_put(' ');
  for (i=0;i<1000000;i++) sA=jm_sqrt(A);
  writeDECi(sA);comm_put('\r');comm_put('\n');
  
  fA=(double)A; 
  for (i=0;i<1000000;i++) fsA=sqrt(fA);
  writeDECi((int)fsA); comm_put('\r');comm_put('\n');
  
  comm_puts(welcome);comm_put('\r');comm_put('\n');
  
  while (1) {}
// printf("sqrt(%d)=%d %d\r\n",A,sA,fsA-sA);
// printf("T=%f=%d\r\n",fA0+sqrt(fA1+fA2*fdf),A0+jm_sqrt(A1+A2*df)/10);
}

void RCC_Configuration(void)           // Configures the different system clocks
{
  ErrorStatus HSEStartUpStatus;
  RCC_DeInit();                        // RCC system reset(for debug purpose) 
  RCC_HSEConfig(RCC_HSE_ON);                  // Enable HSE 
  HSEStartUpStatus = RCC_WaitForHSEStartUp(); // Wait till HSE is ready 

  if(HSEStartUpStatus == SUCCESS)
  {
    FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);// Ena Prefetch Buffer
    FLASH_SetLatency(FLASH_Latency_2);                   // Flash 2 wait state
    RCC_HCLKConfig(RCC_SYSCLK_Div1);                     // HCLK = SYSCLK 
    RCC_PCLK2Config(RCC_HCLK_Div1);                      // PCLK2 = HCLK 
    RCC_PCLK1Config(RCC_HCLK_Div2);                      // PCLK1 = HCLK/2 
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // PLLCLK=8MHz*9=72 MHz
    RCC_PLLCmd(ENABLE);                                  // Enable PLL 
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}// Wait till PLL ready

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08) {}
 // ADC  jmf
 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC,ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 12 MHz on APB2 CK

 // RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);        // GPIO 

 // USART jmf 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC,ENABLE);
  }
}

void NVIC_Configuration(void)         // Configures Vector Table base location.
{
#ifdef  VECT_TAB_RAM  
  /* Set the Vector Table base location at 0x20000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */ 
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif
}

void Delay(vu32 nCount) {for(; nCount != 0; nCount--);}

#ifdef  DEBUG
/*******************************************************************************
* Function Name : assert_failed
* Description   : Reports the name of the source file and the source line number
*                 where the assert_param error has occurred.
* Input         : - file: pointer to the source file name
*                 - line: assert_param error line source number
* Output        : None
* Return        : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
/* User can add his own implementation to report the file name and line number,
   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1) {}
}
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
