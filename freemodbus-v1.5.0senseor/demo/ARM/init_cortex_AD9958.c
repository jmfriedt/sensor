#include <stdlib.h>
#include <string.h>

////////////// begin libsmt32 ///

#include "init_cortex_AD9958.h"
#include <libopencm3/cm3/common.h>
#include <stm32/gpio.h>
#include <stm32/usart.h>
#include <stm32/rcc.h>
#include <cmsis/stm32.h>
#include <stm32/flash.h>
#include <stm32/misc.h>
#include <stm32/adc.h>
#include <stm32/tim.h>
#include <stm32/spi.h>
#include <stm32/dac.h>
#include <cmsis/stm32.h>
#include "DDSinc.h"
#include "DDSvar.h"

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>

#define readerCS_SET    GPIO_SetBits  (GPIOA,GPIO_Pin_13)
#define readerCS_CLR    GPIO_ResetBits(GPIOA,GPIO_Pin_13)
#define readerIOUP_SET  GPIO_SetBits  (GPIOA,GPIO_Pin_14)
#define readerIOUP_CLR  GPIO_ResetBits(GPIOA,GPIO_Pin_14)
#define readerRESET_SET GPIO_SetBits  (GPIOA,GPIO_Pin_15)
#define readerRESET_CLR GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define readerF1_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_10)
#define readerF1_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_10)
#define readerF2_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define readerF2_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_9)
#define readerFD_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#define readerFD_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_11)
#define readerallSET    GPIO_SetBits  (GPIOB,GPIO_Pin_11|GPIO_Pin_9|GPIO_Pin_10)
// HMC349 : lo donne RFC=RF2 et hi donne RFC=RF1

#define alim5p0V GPIO_SetBits(GPIOB,GPIO_Pin_7)	// ON if hi, OFF if lo
#define alim1p8V GPIO_SetBits(GPIOB,GPIO_Pin_8)	// ON if hi, OFF if lo
#define alimDDS  GPIO_ResetBits(GPIOB,GPIO_Pin_6)	// ON if lo, OFF if hi (DDS PWR_DWN)

void init_DDS () { }

void init_PLL () { }

int
init_GPIO ()
{
  GPIO_InitTypeDef gspi;
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);	// PIO alternate func for PA13..15
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);

  // PB 6..11
  gspi.GPIO_Pin =
    GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
    GPIO_Pin_11;
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOB, &gspi);

  gspi.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;	// PC1, 2 LED
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOC, &gspi);

  GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, ENABLE);	// Disable JTAG/SWD so pins are availble
  gspi.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	// PA13 .. 15
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOA, &gspi);

  // SPI2 SCK, MISO, MOSI
  gspi.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	// PA5, 6, 7 = SPI1 CK, MISO, MOSI
  gspi.GPIO_Mode = GPIO_Mode_AF_PP;	// alors que SPI2 est PB15 (MOSI) et PB13 (CK)
  GPIO_Init (GPIOB, &gspi);

  SPI_InitTypeDef spi;
  spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi.SPI_Mode = SPI_Mode_Master;
  spi.SPI_DataSize = SPI_DataSize_8b;
  spi.SPI_CPOL = SPI_CPOL_High;	// Low/High;   rest state of clock
  spi.SPI_CPHA = SPI_CPHA_2Edge;	// 1Edge/2Edge 
  spi.SPI_NSS = SPI_NSS_Soft;
  spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  
  spi.SPI_FirstBit = SPI_FirstBit_MSB;
  spi.SPI_CRCPolynomial = 7;
  SPI_I2S_DeInit (SPI2);
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_SPI2, ENABLE);
  SPI_Init (SPI2, &spi);
  SPI_Cmd (SPI2, ENABLE);
  // synchronize

  return 0;
}

GPIO_InitTypeDef GPIO_InitStructure;
/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration (void);
void NVIC_Configuration (void);
void Delay (uint32_t nCount);
void RTC_GetTime (uint32_t * THH, uint32_t * TMM, uint32_t * TSS);
void RTC_Init (void);
void initADC (void);
/* Private functions ---------------------------------------------------------*/

void
init_CPU_specifique (void)
{
  NVIC_InitTypeDef NVIC_InitStruct;
  init_GPIO ();
  readerCS_SET;
  readerF1_SET;
  readerF2_CLR;
  readerFD_SET;
  alim5p0V;
  alim1p8V;
  alimDDS;
  dac1_init ();
  dac2_init ();
  readerIOUP_CLR;
  readerRESET_SET;		// reset DDS
  delay (10000);
  readerRESET_CLR;
  delay (10000);
  readerIOUP_SET;
  delay (10000);
  readerIOUP_CLR;
  delay (1000);

  NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;	// TIM3_IRQChannel;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init (&NVIC_InitStruct);  

  RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
  Timer3_Configuration ();
  TIM_ClearITPendingBit (TIM3, TIM_IT_Update);
  TIM_ITConfig (TIM3, TIM_IT_Update, ENABLE);	// Activation de l'interruption Timer3
  TIM_Cmd (TIM3, ENABLE);	                // Activation du timer3 au dixième de seconde
}

void
envoi_DDS (unsigned char *entree, int n)
{
  unsigned char i;
/*  unsigned short v;
  if (LBT == 1)
    {
      do
        {                       // on attend pour interroger que la bande soit silencieuse
          ADCCON = 0x7e3;       // 7E3: 5 kHz avec delay=50, 15 kHz avec delay=0
          while (!ADCSTA)
            {
            };                  // 17 kHz si on passe a 0E3 avec tps de charge=25 ou 15
          v = ((unsigned short) (ADCDAT >> 16));
        }
      while (v > SEUILMIN);
    }
*/
  readerCS_CLR;
  delay (attend);
  for (i = 0; i < n; i++)
    {
      spi_put2 (entree[i]);
    }
  delay (attend);

  readerCS_SET;
  delay (attend);
  readerIOUP_SET;
  delay (attend);
  readerIOUP_CLR;
  delay (attend);
}

void
puiss (unsigned char puissance)
{
}

unsigned short
interroge (unsigned char puissance)
{
  unsigned short v;
  v=10;
  return (v);
}

