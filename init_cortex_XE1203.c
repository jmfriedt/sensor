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
#include <stdlib.h>
#include <string.h>
#include "DDSinc.h"
#include "DDSvar.h"

#include <xe1203f/XE1203Driver.h>

#define readerCS_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_12)
#define readerCS_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_12)
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
// HMC349 : lo donne RFC=RF2 et hi donne RFC=RF1

#define alim5p0V GPIO_SetBits(GPIOB,GPIO_Pin_7)	// ON if hi, OFF if lo
#define alim1p8V GPIO_SetBits(GPIOB,GPIO_Pin_8)	// ON if hi, OFF if lo
#define alimDDS  GPIO_ResetBits(GPIOB,GPIO_Pin_6)	// ON if lo, OFF if hi (DDS PWR_DWN)

uint8_t
spi_put (uint8_t data)
{
  SPI2->DR = data;

  //comm_puts("data :");
  //writeHEXi(data); comm_puts("  ");
  //writeHEXi(SPI2->DR); comm_puts(" \r\n SR :");

  while ((SPI2->SR & (1 << 1)) == 0);	// Si TXE =1 le buff est libre
  // Mais cela ne veut pas dire que la liaison est fini
  return 0;
}

void
jmf_putchar (int a,char *chaine, int* chaine_index,int USARTx)
{
  if (chaine != NULL)
    {chaine[*chaine_index]=a;
     *chaine_index=*chaine_index+1;
    }
  else
    {if (USARTx==1)
       {while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) { ; }
        USART_SendData (USART1, a);
       }
     else
       {while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) { ; }
        USART_SendData (USART2, a);
       }
    }
}

void
init_PLL ()
{				// inutile dans le cas du XE1203F
}

void
envoi_DDS (unsigned char *entree, int n)
{
  unsigned char i;
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
init_DDS ()
{				// INITIALISATION XE1203F
  InitRFChip ();
}


int
init_GPIO ()
{
  GPIO_InitTypeDef gspi;
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);	// PIO alternate func for PA13..15
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);

  // PB 6..11
  gspi.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_12;	//RX, SWICH, EN
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOB, &gspi);

  gspi.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	//PATTERN,DCK
  gspi.GPIO_Mode = GPIO_Mode_IPD;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOB, &gspi);

  // SYNC, 2 LED, 5 AT
  gspi.GPIO_Pin =
    GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 |
    GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 |
    GPIO_Pin_12;
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOC, &gspi);

  GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, ENABLE);	// Disable JTAG/SWD so pins are availble
  gspi.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_8;	// PA13 .. 15 + composant SN6517BD
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
  spi.SPI_CPOL = SPI_CPOL_Low;	// CPOL=0
  spi.SPI_CPHA = SPI_CPHA_2Edge;	// CPHA=1
  spi.SPI_NSS = SPI_NSS_Soft;
  spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;	//=> 1us  SPI_BaudRatePrescaler_2; //=> 0.1 us
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
uint16_t readADC (void);
void initADC (void);
/* Private functions ---------------------------------------------------------*/

void
AlimentationAttenuateurSwitch (void)
{
  set_bit (GPIOC, GPIO_Pin_10);
  set_bit (GPIOC, GPIO_Pin_11);
  set_bit (GPIOC, GPIO_Pin_12);
}


void
init_CPU_specifique (void)
{
  init_GPIO ();

  AlimentationAttenuateurSwitch ();

  dac1_init ();
  dac2_init ();
}

void puiss (unsigned short val,__attribute__((unused)) unsigned char chan)
{
  if (val > 31)
    val = 31;

  if (val & 0x10) set_bit (GPIOC, GPIO_Pin_8);
    else clear_bit (GPIOC, GPIO_Pin_8);
  if (val & 0x08) set_bit (GPIOC, GPIO_Pin_7);
    else clear_bit (GPIOC, GPIO_Pin_7);
  if (val & 0x04) set_bit (GPIOC, GPIO_Pin_6);
    else clear_bit (GPIOC, GPIO_Pin_6);
  if (val & 0x02) set_bit (GPIOC, GPIO_Pin_5);
    else clear_bit (GPIOC, GPIO_Pin_5);
  if (val & 0x01) set_bit (GPIOC, GPIO_Pin_4);
    else clear_bit (GPIOC, GPIO_Pin_4);

}

unsigned short
interroge (unsigned short puissance, unsigned int FREQ,__attribute__((unused))unsigned int offset,__attribute__((unused))unsigned char chan)
{
  unsigned short res;
  unsigned short v;
  unsigned int v12;
  int result;			// HYPOTHESE : on suppose que les bandes de frequences sont calculees
  int f0;			// pour un DDS AD9954/8 cadence' a 200 MHz
  //FREQ=0x2bc6a7ef;//434.2 =>marche
  //puissance=(puissance>>5);

  result = (FREQ - 730144440);	//0x2b851eb8
  f0 = (short) (result / 10737);
  envoie_porteuse ((short) f0);

  set_bit (ANT_SWITCH0, RX);	//Emission atténuateur
  set_bit (GPIOB, SWITCH);	//Emission broche semtech

//  TIM_ITConfig (TIM3, TIM_IT_Update, DISABLE);
  if (NY < puissance)
    {
      for (res = NY; res < puissance; res++)
	{
	  puiss (pulse_shape[res],3);
	}
      delay (EMET);		// tps de charge  PLUS C'EST LONG, PLUS LE PIC EST FIN
      for (res = puissance - 1; res >= NY; res--)
	{
	  puiss (pulse_shape[res],3);
	}
      puiss (0,3);
    }
  else
    {
      puiss (puissance,3);
      delay (EMET * 4);		// tps de charge
    }
//while(1) {};       // test emission fixe
  puiss (0,3);
  clear_bit (ANT_SWITCH0, RX);	// recoit
  clear_bit (GPIOB, SWITCH);	//reception

  delay (HATTENTE*25);           // modif alstom ornans
//while(1) {};       // test reception fixe

  //v = readADC1 (ADC_Channel_8);
  v12 = readADC12 ();
  // v=(unsigned short)(v12&0xffff);
  v=(unsigned short)((((v12&0xffff0000)>>16)+(v12&0xffff))/2);

  // if ((mode_balayage_continu==1) &&  -> affiche meme en mode 3 points
//  if (convertit_temperature == 0)
    {
      if (deux_points == 0)
	{
	  set_DAC0 ((v/2) << 16);
	}
    }
  delay (ATTENTE);

//  TIM_ITConfig (TIM3, TIM_IT_Update, ENABLE);

  return (v);
}

void dephase(__attribute__((unused)) unsigned short phase,__attribute__((unused)) unsigned char chan) {}
