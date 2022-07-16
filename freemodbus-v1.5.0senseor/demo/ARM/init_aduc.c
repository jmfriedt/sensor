#include<ADuC7026.h>
#include<DDSinc.h>
#include<DDSvar.h>
#include "port.h"
#include "mbport.h"
#include "mb.h"

#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

#include "config_modbus.h"

USHORT   usRegInputStart;
USHORT   usRegInputBuf[REG_INPUT_NREGS];
USHORT   usRegHoldingStart;
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];


#define SEUIL_MIN       400
extern unsigned char global_tab[NB_CHARS];
volatile int global_index , tim0 , heure , minute , seconde, tim1;
extern volatile int montim0;
extern volatile unsigned int mont1;
extern volatile unsigned int mont2; 
extern volatile unsigned int GLOBAL_TIMER1;
extern unsigned char FTW0[5];

#ifdef periodique
extern volatile int reveil;
#endif

void
delay (int length)
{				// 45=28.43 us teste', inline asm => compiler en -O1
//       while (length >=0) length--;
  asm ("PUSH        {R0}			");
  asm ("       B           jmfL_1  	");
  asm ("   jmfL_3:			");
  asm ("       ADD         R0,R13,#0x0	");
  asm ("       LDR         R1,[R0,#0x0] 	");
  asm ("       SUB         R1,#0x1	");
  asm ("       STR         R1,[R0,#0x0] 	");
  asm ("   jmfL_1:			");
  asm ("       ADD         R0,R13,#0x0	");
  asm ("       LDR         R0,[R0,#0x0] 	");
  asm ("       CMP         R0,#0x0	");
  asm ("       BGE         jmfL_3  	");
  asm ("	   ADD         R13,#0x4		");
}

void
jmf_putchar (int ch)
{				// Write character to Serial Port  
#ifdef affiche_rs232
  do
    {
    }
  while ((COMSTA0 & 0x020) != 0x20);
  COMTX = ch;
#endif
}

int
my_getchar (void)
{				/* Read character from Serial Port */
  // while 
  if (!(0x01 == (COMSTA0 & 0x01)))
    {
    };
  return (COMRX);
}

unsigned short
temperature ()
{
  unsigned short moy = 0;
  unsigned char k;
  ADCCP = 16;
  for (k = 0; k < 16; k++)
    {
      ADCCON = 0x7E3;
      delay (10);
      while (!ADCSTA)
	{
	}
      moy += (unsigned short) ((ADCDAT >> 16) & 0xfff);
    }
  ADCCP = 0;			// channel number
  return (moy);
}

void
ADCpoweron (int time)
{
  ADCCON = 0x20;
  while (time >= 0)
    time--;			// wait for ADC to be fully powered on
}

void
envoi_PLL (unsigned char *entree)
{
  unsigned char i;
  GP4DAT &= ~0x00080000;	// CS# P4.y = 1
  delay (5);
  for (i = 0; i < 3; i++)
    {
      SPITX = entree[i];	// transmit command or any dummy data
      do
	{
	}
      while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
    }
  delay (40 * attend);
  GP4DAT |= 0x00080000;
  delay (attend);
}

void
envoi_DDS (unsigned char *entree, int n)
{
  unsigned char i;
  unsigned short v;
  if (LBT == 1)
    {
      do
	{			// on attend pour interroger que la bande soit silencieuse
	  ADCCON = 0x7e3;	// 7E3: 5 kHz avec delay=50, 15 kHz avec delay=0
	  while (!ADCSTA)
	    {
	    };			// 17 kHz si on passe a 0E3 avec tps de charge=25 ou 15
	  v = ((unsigned short) (ADCDAT >> 16));
	}
      while (v > SEUILMIN);
    }
  GP4DAT &= ~0x00100000;	// CS# P4.x = 0
  delay (attend);
  for (i = 0; i < n; i++)
    {
      SPITX = entree[i];	// transmit command or any dummy data
      do
	{
	}
      while ((SPISTA & 0x01) == 0x01);	// wait for data received status bit
    }
  delay (attend);

  GP4DAT |= 0x00100000;		// GP4DAT = 0x14100000; // CS# P4.4 = 1
  delay (attend);
  GP4DAT |= 0x00040000;		// GP4DAT = 0x14140000; // IOUP   P4.2
  delay (attend);
  GP4DAT &= ~0x00040000;	// GP4DAT = 0x14100000;
  delay (attend);
}

void
puiss (unsigned char puissance)
{
  unsigned char p;
  p = (puissance & 0x02) << 3 | (puissance & 0x04) << 1 | (puissance & 0x08) >> 1 | (puissance & 0x10) >> 3 | (puissance & 0x01);	// V1, V2, V3 ,V4, V5
  GP0DAT = ((GP0DAT & 0xFFE00000) | (int) p << 16);	// Mot constitue : FF XXXP PPPP  
}

unsigned short
interroge (unsigned char puissance)
{
  unsigned short res;
  unsigned short v;
  envoi_DDS (FTW0, 5);
#ifndef v2450
  GP4DAT &= ~0x00600000;	// F1decl passant
#else
  GP2DAT &= ~0x00600000;	// F1decl passant
#endif

#ifdef SetH
  GP0DAT |= 0x00400000;
#endif
  if (NY < puissance)
    {
      for (res = NY; res < puissance; res++)
	{
	  puiss (pulse_shape[res]);
	}
      delay (EMET);	// tps de charge  PLUS C'EST LONG, PLUS LE PIC EST FIN
      for (res = puissance - 1; res >= NY; res--)
	{
	  puiss (pulse_shape[res]);
	}
      puiss (0);
    }
  else
    {
      puiss (puissance);
      delay (EMET);	// tps de charge
    }

  puiss (0);
#ifndef v2450
  GP4DAT |= 0x00200000;		// F1dec2 en emission
  GP4DAT |= 0x00400000;		// F1dec2 coupe'
#else
  GP2DAT |= 0x00200000;		// F1dec2 en emission
  GP2DAT |= 0x00400000;		// F1dec2 coupe'
#endif

  delay (HATTENTE);		// modif alstom ornans
#ifdef SetH
  GP0DAT &= ~0x00400000;
#endif

  ADCCON = 0x7e3;		// 7E3: 5 kHz avec delay=50, 15 kHz avec delay=0 et indep du tps de charge
  while (!ADCSTA)
    {
    };				// 17 kHz si on passe a 0E3 avec tps de charge=25 ou 15
  v = ((unsigned short) (ADCDAT >> 16));
  // if ((mode_balayage_continu==1) &&  -> affiche meme en mode 3 points
  if (convertit_temperature == 0)
    {
      if (deux_points == 0)
	DAC0DAT = (v << 16);
    }
  delay (ATTENTE);
#ifdef v2450
  GP2DAT &= ~0x00600000;	// F1decl passant
#endif
  return (v);
}

void
IRQ_Handler (void)		//void IRQ_Handler() __irq 
{
  if (IRQSIG & UART_BIT)
    {				// UART Interrupt      
      // global_tab[global_index] = my_getchar ();
      //if (global_index < (NB_CHARS - 1))
      // global_index++;
        pxMBFrameCBByteReceived(  );
    }
  if (IRQSIG & GP_TIMER_BIT)
    {				// Timer1 Interrupt      
      GLOBAL_TIMER1++;
      T1CLRI = 1;		// Clear Timer 1 interrupt
    }
  if (IRQSIG & RTOS_TIMER_BIT)
    {				// Timer0 Interrupt      
      if (tim0<255) tim0++;
      if (tim0>=2) ( void )pxMBPortCBTimerExpired(  );
      tim1++; if(tim1==9999)tim1=0;
#ifdef RTC0
      seconde++;
      if (seconde > 600)
	{
	  seconde = 0;
	  minute++;
	}			// seconde en 1/10 seconde
      if (minute > 60)
	{
	  minute = 0;
	  heure++;
	}
      if (heure > 24)
	{
	  heure = 0;
	}
#endif
      T0CLRI = 0;
    }

  if (IRQSIG & MONITOR_BIT)	// Input capture
    {
      if (CMPCON & 0x01)
	{
	  CMPCON = 0x4C5;
	  montim0 = 1;
	}
      if (CMPCON & 0x02)
	{
	  CMPCON = 0x4C6;
	  montim0 = 2;
	  if (mont1 == 0)
	    {
	      T1LD = 0;
	      mont1 = 1;
	    }			// T1VAL;}
	  else if (mont2 == 0)
	    mont2 = T1VAL;
	}			// -mont1;}
    }
#ifdef periodique
  if (IRQSIG & WAKEUP_TIMER_BIT)
     {T2CLRI=0xff;
      reveil=15; // nombre de mesures
     }
#endif
}

void
init_CPU (void)
{// unsigned long k;
#ifdef LCD
  char s[20];
#endif

  GP1PAR = 0x00010000;
  GP1CON = 0x02220011;		// config SPI, setup tx & rx pins on P1.0 and P1.1
  GP1DAT = 0x8C800000;          // P1.2-P1.3 as output
  SPIDIV = 0x06;		// set SPI clock 40960000/(2x(1+SPIDIV))->0x00=21MHz
  SPICON = 0x1043;
  // slave select will stay low during the all transmission
  GP4DAT = 0xff100000;		// P4.4 et P4.2 sont configurées en sortie.
  GP2DAT = 0xff0F0000;		// P2.3 = 0;
  GP3DAT = 0xff070000;
  // Start setting up UART at RS_BAUDRATE
  COMCON0 = 0x080;		// Setting DLAB = autorise modification baudrate
  COMDIV0 = RS_BAUDRATE;	// 0x0b=115200, 0x17 pour 57600, 0x88=9600, 0x44=19200
  COMDIV1 = 0x000;		// NE PAS LANCER PAR RUN MAIS RESET DE LA CARTE
  COMCON0 = 0x007;		// Clearing DLAB = protege baudrate
  COMIEN0 = 1;
//mesure de la fonction delay
/*	while(1)
	{
 	GP4DAT &= ~0x00400000;
	delay(45);
	GP4DAT |= 0x00400000;	 
	 delay(45);	
	}*/

//   write_str("Init CPU\r\n");
  GP4DAT |= 0x00020000;		// RESET
  delay (50);
  GP4DAT &= ~0x00020000;
  delay (50);

  ADCpoweron (20000);		// power on ADC
  REFCON = 0x01;		// internal 2.5V reference. 2.5V on Vref pin
  ADCCP = 0;			// channel number


// DAC configuration
  DAC0CON = 0x12;		// DAC configuration VREF/AGND
  DAC0DAT = 0x00000000;		// start from midscale
  DAC1CON = 0x12;		// DAC configuration AVdd/AGND
  DAC1DAT = 0x00000000;		// start from midscale
  DAC2CON = 0x13;		// DAC configuration AVdd/AGND
  DAC2DAT = 0x0ffF0000;		// start from midscale 85F pour att, D5F pour ant
  DAC3CON = 0x13;		// DAC configuration AVdd/AGND
  DAC3DAT = 0x00000000;		// start from midscale

  // Configuration lcd
  //LCD_init();                           // [RRRGGGBB]
  //GP2DAT &= ~LCD_SCK;

  //LCDSetPixel(30, 30, YELLOW);
  //LCDPutChar('E', 30, 30, MEDIUM, BLACK, YELLOW);

  //SPIDIV = 0x03;		// speed up : set SPI clock 40960000/(2x(1+SPIDIV)) ---> 0x00 = 21 MHz  0xCC

  GP0DAT = (0xFF000000);	// sortie antenne 4
//GP0DAT|=(0xFF000000|(int)0x0F<<17); // puissance max

  IRQ = IRQ_Handler;
/* mise en marche des interruptions */
  T0CON = 0xC8;			// /256
  T0LD = (16320/10);		// 100 Hz

  //T1LD=0;      //  32 bits; 
  //T1CON=0x188; //  8 => core/256, 1=count UP
  CMPCON = 0x4C4;

  T1LD = 1500;			// 41780000/256, 32 bits; 
  T1CON = 0x44;			// 4=> core/256, periodic mode, inactif (C4 pour actif)
#ifdef periodique 
 T2CON=0xE8;      // Timer2 counts down, format Hr:Min:Sec:1/100Sec and prescale 256
 T2LD=0x00050000; // intervalle de temps en min (ici 50000=5 min) 
#endif

#ifdef compte_tour
#ifdef periodique
  IRQEN = UART_BIT | MONITOR_BIT | RTOS_TIMER_BIT | WAKEUP_TIMER_BIT; // | GP_TIMER_BIT;
#else
  IRQEN = UART_BIT | RTOS_TIMER_BIT | MONITOR_BIT;
#endif
#else
#ifdef periodique
  IRQEN = UART_BIT | RTOS_TIMER_BIT | WAKEUP_TIMER_BIT | GP_TIMER_BIT;; // 
#else
  IRQEN = UART_BIT | RTOS_TIMER_BIT | GP_TIMER_BIT;
#endif
#endif

/*
  sortie_courant_init();
  while (1) {
   for (k=0;k<0xfff;k+=1) 
    {
     sortie_courant(k<<4);
     writeHEXi(k);write_str("\r\n");delay(0xfff);
    }
  }
*/
}

void sortie_antenne(int antenne)
{
#ifdef wrc
      GP0DAT =
        (GP0DAT & 0xff1fffff) + (((antenne + 1) & 0x01) << 21) +
        (((antenne + 1) & 0x02) << 22);
#else
      GP0DAT =
        (GP0DAT & 0xff1fffff) + (((antenne) & 0x01) << 21) +
        (((antenne) & 0x02) << 22);
#endif
}

void sortie_led(int led)
{                 GP3DAT |= 0x00070000; // active le vert rouge:P3.2
                  if (led > 100)
                    GP3DAT &= ~0x00040000;      // vert: P3.2
                  else if (led > 0)
                    GP3DAT &= ~0x00020000;      // orange
                  else
                    GP3DAT &= ~0x00010000;      // rouge: P3.0
}

void remont_CS_PLL() {GP4DAT &= ~0x00080000;} // Latch enable: store data when high

void set_serial_speed(int ntmp)
{                     COMCON0 = 0x080;  // Setting DLAB
                      COMDIV0 = ntmp;   // 0x0b=15200, 0x17 pour 57600, 
                      COMCON0 = 0x007;  // Clearing DLAB
}

void set_SPI_speedDiv(int i) {SPIDIV=i;}

void set_DAC0(int i) {DAC0DAT = i;}
void set_DAC1(int i) {DAC1DAT = i;}
void set_DAC2(int i) {DAC2DAT = i;}

#ifdef periodique
void power_down()
{// power down by writing 0x40 to POWCON -> inactive until IRQ
 POWKEY1=0x01;
 POWCON=0x30; // Timer2 can wakeup
 POWKEY2=0xF4;
}
#endif

void sortie_courant_init()
{// P1.2=RST=latch
 GP1DAT &= ~0x00040000;         // LATCH fall
 set_SPI_speedDiv(16);
 SPITX = (unsigned char)(0x55); 
    do { } while ((SPISTA & 0x01) == 0x01);
 SPITX = (unsigned char)(0x10); 
    do { } while ((SPISTA & 0x01) == 0x01);
 SPITX = (unsigned char)(0x05); 
    do { } while ((SPISTA & 0x01) == 0x01);	
 delay (attend*2);
 GP1DAT |= 0x00040000;          // LATCH rise
 delay (40 * attend);
 set_SPI_speedDiv(3);
}

void sortie_courant(int val)
{// P1.2=RST=latch
 GP1DAT &= ~0x00040000;         // LATCH fall
 set_SPI_speedDiv(16);
// SPICON = 0x1047; // inutile pour rising edge
 SPITX = (unsigned char)(0x01); 
    do { } while ((SPISTA & 0x01) == 0x01);
 SPITX = (unsigned char)((val&0xff00)>>8); 
    do { } while ((SPISTA & 0x01) == 0x01);
 SPITX = (unsigned char)(val&0xff); 
    do { } while ((SPISTA & 0x01) == 0x01);	
 delay (attend*2);
 GP1DAT |= 0x00040000;          // LATCH rise
 delay (40 * attend);
// SPICON = 0x1043;
 set_SPI_speedDiv(3);
}

void clignote() {
GP3DAT ^= 0x00040000;
}

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{if (xTxEnable==TRUE) xMBRTUTransmitFSM();
  
}

BOOL
xMBPortSerialInit( UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{return TRUE;}


BOOL
xMBPortSerialPutByte( CHAR ucByte)
{/*
  volatile int reg[20];
  int i=0;
    for(i=0;i<8;i++)writeHEXc (reg[i]);
*/
    jmf_putchar (ucByte);
    return TRUE;
}


BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{ 
  *pucByte=my_getchar ();
  return TRUE;
}


/////////////////////////////
int nbpics[2];
unsigned char ACR[4] = { 0x06, 0x00, 0x13, 0xFF };
unsigned char FTW0[5] = { 0x04, 0x31, 0x00, 0x70, 0xA3 };

unsigned char global_tab[NB_CHARS];
volatile int global_index = 0, tim0 = 0, heure = 0, minute = 0, seconde = 0;
volatile int montim0 = 0;
volatile unsigned int mont1 = 0, mont2 = 0;
volatile unsigned int GLOBAL_TIMER1 = 0;
#ifdef periodique
volatile int reveil = 0;
#endif

void
writeDECs(unsigned short d)
{
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDECc(unsigned char d)
{
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDEC0i(int d)
{
  if (d < 0)
    {
      jmf_putchar ('-');
      d = -d;
    }
  if (d > 99999)
    d = 99999;
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else
    jmf_putchar (48);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDECi(int d)
{
  int sup = 0;
  if (d < 0)
    {
      jmf_putchar ('-');
      d = -d;
    }
  if (d > 999999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000000) + 48);
      d = d - (d / 1000000000) * 1000000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99999999)
    {
      sup = 1;
      jmf_putchar ((d / 100000000) + 48);
      d = d - (d / 100000000) * 100000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9999999)
    {
      sup = 1;
      jmf_putchar ((d / 10000000) + 48);
      d = d - (d / 10000000) * 10000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000) + 48);
      d = d - (d / 1000000) * 1000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99999)
    {
      sup = 1;
      jmf_putchar ((d / 100000) + 48);
      d = d - (d / 100000) * 100000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9999)
    {
      sup = 1;
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 999)
    {
      sup = 1;
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99)
    {
      sup = 1;
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else if (sup != 0)
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeHEXi(unsigned int ptr)
{
  unsigned char b;
  {
    b = ((ptr & 0xf0000000) >> 28);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0f000000) >> 24);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00f00000) >> 20);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000f0000) >> 16);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0000f000) >> 12);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00000f00) >> 8);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000000f0) >> 4);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0000000f));
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    jmf_putchar (32);
  }
}

void writeHEXs(unsigned short ptr)
{
  unsigned char b;
  {
    b = ((ptr & 0xf000) >> 12);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0f00) >> 8);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00f0) >> 4);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000f));
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    jmf_putchar (32);
  }
}

void writeHEXc(unsigned char ptr)
{
  unsigned char b;
  b = ((ptr & 0x00f0) >> 4);
  if (b < 10)
    jmf_putchar (b + 48);
  else
    jmf_putchar (b + 55);
  b = ((ptr & 0x000f));
  if (b < 10)
    jmf_putchar (b + 48);
  else
    jmf_putchar (b + 55);
  jmf_putchar (32);
}

//-------------------------------------------------------------------------------------------------------

/*void
_kill ()
{
}

void
_getpid ()
{
}

void
_exit ()
{
}
*/

void            EnterCriticalSection( void ) {};
void            ExitCriticalSection( void ) {};

/* ----------------------- Defines ------------------------------------------*/
/* Timer ticks are counted in multiples of 50us. Therefore 20000 ticks are
 * one second.
 */
#define MB_TIMER_TICKS          ( 10L )

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usTimerOCRADelta;
static USHORT   usTimerOCRBDelta;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timeout50us )
{
  /*  BOOL            bInitialized = FALSE;
    ULONG           *ulReloadValue = tim0 ; //  = ( ACLK * ( ULONG )usTim1Timeout50us ) / MB_TIMER_TICKS;
    
    if( ulReloadValue <= 1 )
    {
        ulReloadValue = 1;
    }
    else
    {
        ulReloadValue -= 1;
    }
*/
    tim0=0;

    return TRUE;
}

void
vMBPortTimersEnable( void )
{
 /*
  #if MB_TIMER_DEBUG == 1
    PIO_Set( &xTimerDebugPins[0] );
#endif
    TCX->TC_CHANNEL[TCCHANNEL].TC_IER = TC_IERX_CPAS;
    TC_Start( TCX, 0 );
*/
  tim0=0;
}

void
vMBPortTimersDisable( void )
{tim0=255;
  /*    TC_Stop( TCX, 0 );
#if MB_TIMER_DEBUG == 1
    PIO_Clear( &xTimerDebugPins[0] );
#endif
*/
}

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {  
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {  
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {  
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{   
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
 usAddress+=REG_HOLDING_START;
    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {   
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {   
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {   

                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{   
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
