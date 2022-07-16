//#define puiss_reception puiss    // cas Schneider : attenuation sur reception
#define puiss_emission puiss     // cas gilloux : attenuation sur emission

//#include <libopencm3/cm3/common.h> // BEGIN_DECL,          added 150116
#include <stdlib.h>
#include <string.h>

////////////// begin libstm32 ///

#include "init_cortex.h"
#include "DDSinc.h"
#include "DDSvar.h"
#include "config_modbus.h"

#include "init_cortex_AD9958.h"

#ifndef AD9958
#error *** il serait judicieux d''activer AD9958 et f400 ***
#endif

#ifndef f400
#error le DDS genere son signal par 340+94 MHz, bande de frequence activee par define f400
#endif

void go_usb(void);

extern unsigned char affiche_rs232;
extern int phase_mag;
unsigned char CSR[2] = { 0x00, 0xF0 };

//unsigned char FR1[4]={0x01,0xD0,0x00,0x00}; // x20,  en dernier bits 7..0  D0=400 MHz, C8=360 MHz
unsigned char FR1[4] = { 0x01, 0xC4, 0x00, 0x00 }; // x17,  en dernier bits 7..0  D0=400 MHz, C8=360 MHz, C4=340 MHz
unsigned char FR2[3] = { 0x02, 0x00, 0x00 };
unsigned char CFR[4] = { 0x03, 0x00, 0x03, 0x00 }; 
unsigned char ACR[4] = { 0x06, 0x00, 0x13, 0xFF };
unsigned char FTW0[5] = { 0x04, 0x31, 0x00, 0x70, 0xA3 };	// FTW0 2A 00 70 A3 = 32.814 MHz 2A FF 70 A3 = 33.592 // Frequency tuning World
unsigned char CPOW0[3] = { 0x05, 0x00, 0x00};	

#define readerPWDN_SET  GPIO_SetBits  (GPIOB,GPIO_Pin_6)
#define readerPWDN_CLR  GPIO_ResetBits(GPIOB,GPIO_Pin_6)
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
#define rs485TX_SET	GPIO_SetBits  (GPIOA,GPIO_Pin_8)
#define rs485TX_CLR	GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define P2_SET		GPIO_SetBits  (GPIOA,GPIO_Pin_1)
#define P2_CLR		GPIO_ResetBits(GPIOA,GPIO_Pin_1)
#define P3_SET		GPIO_SetBits  (GPIOA,GPIO_Pin_0)
#define P3_CLR		GPIO_ResetBits(GPIOA,GPIO_Pin_0)
// HMC349 : lo donne RFC=RF2 et hi donne RFC=RF1

#define alim5p0V GPIO_SetBits(GPIOB,GPIO_Pin_7)	// ON if hi, OFF if lo
#define alim1p8V GPIO_SetBits(GPIOB,GPIO_Pin_8)	// ON if hi, OFF if lo

void jmf_putchar (int a,char *chaine, int* chaine_index,__attribute__((unused))int USARTx)
{
  if (chaine != NULL)
    {chaine[*chaine_index]=a;
     *chaine_index=*chaine_index+1;
    }
  else
    {
#ifdef MODBUS
     if (USARTx==modbus_port)
       {delay(1000);
        rs485TX_SET;
        delay(1000);
        if (modbus_port==1)
           {USART_SendData (USART1, a);
            while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { ; }
           }
        else 
           {USART_SendData (USART2, a);
            while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) { ; }
           }
// while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) { ; } // pas le bon bit
        delay(1000);
        rs485TX_CLR;
        delay(1000);
       }
     else
#endif
        if (affiche_rs232==1) {
          if (usart_port==1) 
             {USART_SendData (USART1, a);
              while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) { ; }
             }
          else
             {USART_SendData (USART2, a);
              while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) { ; }
             }
       }
    }
}

void init_PLL() {} // no PLL on this board

void init_DDS ()
{
  int SEC;
  for (SEC = 0; SEC < 10; SEC++)
    { // init DDS
      envoi_DDS (CSR, 2);
      envoi_DDS (FR1, 4);
      envoi_DDS (FR2, 3);
      envoi_DDS (CFR, 4);
      envoi_DDS (FTW0, 5);
      envoi_DDS (ACR, 4);
      delay (50);
    }
 //init_rampe_AM();
}

int
init_GPIO ()
{
  GPIO_InitTypeDef gspi;
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);	// PIO alternate func for PA13..15
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);

  // PB 6..11
  gspi.GPIO_Pin =
    GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11; // vire GPIO_Pin_1 140528
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOB, &gspi);

  gspi.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 | GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_6|GPIO_Pin_7 | GPIO_Pin_8;	// PC1, 2 LED
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOC, &gspi);   // PC4..8 = attenuateur en reception

  GPIO_SetBits  (GPIOC,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8); // init attenuateur

  GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, ENABLE);	// Disable JTAG/SWD so pins are availble
  gspi.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_8 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	// PA13 .. 15 et PA0..PA2 pour ramp-AM du DDS
  gspi.GPIO_Mode = GPIO_Mode_Out_PP;
  gspi.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init (GPIOA, &gspi);
  GPIO_ResetBits  (GPIOA,GPIO_Pin_0| GPIO_Pin_1 | GPIO_Pin_2); // init DDS AM

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
  spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  // jmfxx SPI_BaudRatePrescaler_2;
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
  rs485TX_CLR;
  readerCS_SET;
  readerF1_SET;
  readerF2_CLR;
  readerFD_SET;
//  readerPWDN_SET;               // low power active
  alim5p0V;
  alim1p8V;
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

#ifdef stm32_usb
  go_usb();
#endif
}

void init_rampe_AM() // suppose d'avoir PA0,1,2 connecte' a SDIO1, 2, 3 -> pull down sur SDIO3 pour IO_SYNC=lo
{
 CSR[1]= (3<<6); envoi_DDS (CSR, 2);
 FR1[2] = 0x04; envoi_DDS (FR1, 4); // 11/10=1: RU/RD ena
 ACR[3]=0xFF;ACR[2] = 0xDB; ACR[1] = 0x01; envoi_DDS (ACR, 4); // 11/12=1: RU/RD 
/*
CFR[4] = { 0x03, 0x00, 0x03, 0x00 }; 
      envoi_DDS (CFR, 4);
*/
}

void
envoi_DDS (unsigned char *entree, int n)
{
  unsigned char i;
/*  unsigned short v;

  if (LBT == 1)
    {do {                       // on attend pour interroger que la bande soit silencieuse
          ADCCON = 0x7e3;       // 7E3: 5 kHz avec delay=50, 15 kHz avec delay=0
          while (!ADCSTA) {};   // 17 kHz si on passe a 0E3 avec tps de charge=25 ou 15
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

void puiss_reception(unsigned short puissance, __attribute__((unused))unsigned char chan) 
{// MODIF 140121 : #define pour choisir attenuation en emission ou reception
 // HMC470LP3: 1=passant, 0=attenue ; V5=1 dB, V1=16 dB
 GPIO_SetBits  (GPIOC,(puissance&0x1F)<<3);      // AT1 sur PC8, AT5 sur PC4
 GPIO_ResetBits(GPIOC,(31-(puissance&0x1F))<<3); // AT1 sur PC8, AT5 sur PC4
}

void
puiss_emission(unsigned short puissance,unsigned char chan)
{
  if (puissance<=PUISS_MAX) puissance=puiss9958[PUISS_MAX-puissance]; else puissance=0;
// freq= (0x46060600); // SEAS10 pour 340 MHz = 93 MHz  <- bon
// [1023,895 ,767 ,639 ,511, 383,255,128,64  ,32,   16,   8]
// [15.3,14.4,13.9,13.0,11.7,9.5,6.1,0.3,-5.7,-11.6,-17.6,-23.3] sur schneider

// [12.8 12  11  10  09  08  07  06  05  04  03  02  01  00  -1  -2  -3 -4 -5 -6 -7 -8 -9 -10 -11 -12 -13 -14 -15 -16 -17 -18]
// [1023 880 715 580 480 400 350 307 269 238 208 184 162 142 127 112 99 89 79 71 63 57 51  46  41  36  33  29  25  23  20  18]

  CSR[1]= (chan<<6); envoi_DDS (CSR, 2);
  ACR[2] = (ACR[2] & 0xFC)+((puissance >> 8)&0x03);
  ACR[3] = (puissance &0xff);
  envoi_DDS (ACR, 4);
}

unsigned short
interroge (unsigned short puissance, unsigned int freq,unsigned int offset,unsigned char chan)
{
#ifdef gilloux
  static int retard;
  static int cpt;
#endif
  unsigned short res;
  unsigned short v;
  unsigned int v12;

  CSR[1]= (chan<<6); envoi_DDS (CSR, 2);
  FTW0[1] = (freq & 0xFF000000) >> 24;
  FTW0[2] = (freq & 0xFF0000) >> 16;
  FTW0[3] = (freq & 0xFF00) >> 8;
  FTW0[4] = (freq & 0xFF);

  set_DAC2(offset<<16);
  // readerPWDN_CLR;               // low power inactive
  envoi_DDS (FTW0, 5);

  // puiss (0,3); // ajout lionel 140121 
  readerF1_CLR;			// emet
  readerF2_CLR;			// emet
  readerFD_CLR;			// emet
//  P2_SET;                       // ramp up
//  P3_SET;
//  TIM_ITConfig (TIM3, TIM_IT_Update, DISABLE);

#ifdef gilloux
  cpt++;
  if (cpt>=NBMOY) {cpt=0;retard++;}
  if (retard>=15) retard=0;
#endif

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
    }
  else
    {
      puiss (puissance,3);
      delay (EMET * 2 );		// tps de charge
    }

  puiss (puissance,3);
  delay (EMET);         // tps de charge
//  P2_CLR;               // ramp down
//  P3_CLR;
  // while(1) {kick_watchdog();}  // test reception fixe
  puiss (0,3);
  //readerPWDN_SET;               // low power active
  readerF1_SET;			// stop emet 
  readerFD_SET;			// recoit
  readerF2_SET;			// recoit

//  GPIO_SetBits(GPIOB,GPIO_Pin_1); //readerallSET;		// recoit
  // delay (8*HATTENTE);           // modif alstom ornans NECESSAIRE SINON NE MARCHE PLUS 20/09/2013\\

#ifndef gilloux
  delay (5*HATTENTE);           // modif alstom ornans NECESSAIRE SINON NE MARCHE PLUS 20/09/2013
#else
      GPIO_SetBits (GPIOC, GPIO_Pin_2);
  delay (1*HATTENTE);   
      GPIO_ResetBits (GPIOC, GPIO_Pin_2);
#endif
  // 131210: 8* au lieu de 5* pour reduire baseline level
//  GPIO_ResetBits  (GPIOB,GPIO_Pin_1); //readerallSET;			// recoit

  // v = readADC1 (ADC_Channel_8);
  v12 = readADC12 ();
  v=(unsigned short)(v12&0xffff); // magnitude
  readerF2_CLR;			  // coupe reception

  // if ((mode_balayage_continu==1) &&  -> affiche meme en mode 3 points
//  if (convertit_temperature == 0)
    {
      if (deux_points == 0)
	{
#ifdef gilloux
         if (phase_mag==0)                 // appuyer sur 'P'
  	    set_DAC0 ((v/2) << 16);        // affiche magnitude: 1/2 pour avoir le trig
         else
	    set_DAC0 (v12 & 0xffff0000 );  // affiche phase
#endif
	}
    }
  delay (ATTENTE);
//  TIM_ITConfig (TIM3, TIM_IT_Update, ENABLE);
  return (v);
}


void dephase(unsigned short phase,__attribute__((unused))unsigned char chan)
{ // decommenter si on commande dephaseur analogique (et regarder set_DAC0 dans init_cortex)
#ifdef analog_phase
  DAC_SetChannel2Data (DAC_Align_12b_R, (phase>>2));
  DAC_SoftwareTriggerCmd (DAC_Channel_2, ENABLE);
#else
  // commenter si on commande dephaseur analogique
  CSR[1]= (1<<6); envoi_DDS (CSR, 2); // on ne veut changer QUE chan1 wrt chan2
  phase=phase<<2;         // phase suppose'e etre sur 12 bits -> 14 bits
  CPOW0[1]=((phase)>>8);  // on veut une valeur sur 14 bits
  CPOW0[2]=(phase&0x00ff);envoi_DDS (CPOW0, 3);
#endif
}
