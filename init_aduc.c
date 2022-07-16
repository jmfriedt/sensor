#include<ADuC7026.h>
#include<DDSinc.h>
#include<DDSvar.h>
#include <stdlib.h>

#include <racine/racine.h>

#ifdef at25			// memoire flash sur port d'extension derniere ver interro classique
#include "at25.h"
#endif
#ifdef eeprom			// memoire interne de l'ADuC7026
#include "eeprom.h"
#endif

#ifdef LCD
#include <LCD_driver.h>
#endif

#ifdef AD9958
#error *** je ne pense pas que AD9958 actif soit correct ***
#endif

#ifdef MODBUS
#include "port.h"
#include "mbport.h"
#include "mb.h"
#include "config_modbus.h"

extern USHORT   usRegInputStart;
extern USHORT   usRegInputBuf[REG_INPUT_NREGS];
extern USHORT   usRegHoldingStart;
extern USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
#endif

void    (*IRQ)(void);           //      Function Pointers for Interupts         

extern unsigned char global_tab[NB_CHARS];
extern volatile int global_index , tim0 ,tim0modbus, heure , minute , seconde ;
extern volatile int montim0;
extern volatile unsigned int mont1;
extern volatile unsigned int mont2; 
extern volatile unsigned int GLOBAL_TIMER1;
extern unsigned int nbpics[ANTENNES_MAX];
extern unsigned char affiche_rs232;

////////////////// fin methode 2 points ////////////////////////////
#ifndef AD9958
//----------------------------------------------------------------------------//
// registres DDS AD9954
//----------------------------------------------------------------------------//
//#    Les modes de programmation : Single Tone Mode / RAmcontrolled Mode
//#             - RAmcontrolled Mode : Direct Switch / Ramp Up / Bidirectional Ramp / Continuous Bidirectional Ramp / Conti Recirculation
//#             - Single Tone Mode :utilise, emission d'une frequence
//#     Configuration des registres: Balayage de plage de frequence FTW0                       -
//#       Configuration Clock multiplier, Bit 7 a 3 CFR2
//#       Calcul du mot de fréquence => world = (fout*2^32)/FsystClk (Attention 433=> 400+33), le mot tient comte que du 33
//# Si mode RAM : Configuration de RSCW0 : Configuration de RAM segment 0 Mode et des addresses de debut/fin et du compteur
//# Un pas de comptage= 1/4 de SYSCLK
//# Ecriture en RAM: envoie de 0x0B (mot d'ecriture) puis toutes les freqs
//#     de 32bits.
//#
//----------------------------------------------------------------------------//
// Pour l'emission des registres, nous envoyons le bit de poid fort en premier
// Registres DDS AD9954
unsigned char CFR1[5] = { 0x00, 0x00, 0x00, 0x00, 0x40 };       // CFR1         Coupe le Comparateur

#ifndef f400
unsigned char CFR2[4] = { 0x01, 0xC4, 0x02, 0x50 };     // CFR2 REFCLK Multiplier $50=x10 => 200 MHz bits 3-7, 02 pour xtal out enabled
#else
unsigned char CFR2[4] = { 0x01, 0xC4, 0x02, 0xa4 };     // CFR2 REFCLK Multiplier $50=x10 => 400 MHz bits 3-7, 02 pour xtal out enabled
#endif
unsigned char ASF[3] = { 0x02, 0x04, 0x55 };    // Auto Ramp Rate Speed control=11  //ASF Amplitude Scale Factor Register =  0x450
unsigned char ARR[2] = { 0x03, 0xff };  // ARR      Amplitude Ramp Rate register
unsigned char POW0[3] = { 0x05, 0x00, 0x00 };   // POW0 PHASE Offset World <13:0>  Not Used= 00
unsigned char FTW1[5] = { 0x06, 0x2C, 0x8B, 0x43, 0x95 };       // FTW1  Frequency Tuning Word
unsigned char RSCW0[6] = { 0x07, 0x32, 0x00, 0x1D, 0x7D, 0x8F };        // RSCW0        // Profile 0 RAM Segment Control World // Non utilisé

                //Ram Segment 0 Beginning Adress = 0xDF MOde Ramp Up            //No Dwell Active =0
                //Ram Segment 0 mode = %100                             //Ram Segment 0 Final Address=0x11D
                //Ram Segment 0 Beginning Adress = 0x80                 //Ram Segment 0 Adress Ramp Rate = 0032
unsigned char RSCW1[6] = { 0x08, 0x32, 0x00, 0x00, 0x21, 0x8D };        // RSCW1                // Profile 1 RAm Segment Control World

                //Ram Segment 1 mode                                    // No Dwell
                //Ram Segment 1 Beninning Adress :                              //Ram Segment 1 Final Adress :
                //Ram segment 1 Adress Ramp rate:
#else
unsigned char CSR[2]={0x00,0xF0};
//unsigned char FR1[4]={0x01,0xD0,0x00,0x00}; // x20,  en dernier bits 7..0  D0=400 MHz, C8=360 MHz
unsigned char FR1[4]={0x01,0xC4,0x00,0x00}; // x20,  en dernier bits 7..0  D0=400 MHz, C8=360 MHz
unsigned char FR2[3]={0x02,0x00,0x00};
unsigned char CFR[4] ={0x03, 0x00, 0x03,0x00 };
unsigned char ACR[4] ={0x06,0x00,0x13,0xFF};
#endif
unsigned char FTW0[5] = { 0x04, 0x31, 0x00, 0x70, 0xA3 };       // FTW0 2A 00 70 A3 = 32.814 MHz 2A FF 70 A3 = 33.592 // Frequency tuning World

// Registres PLL
#ifndef ADF4111                 // ancienne PLL (avant 11/2012)
unsigned char PLL_R[3] = { 0x06, 0x03, 0x20 };  // FoLD=0, CPo=4, R=320
unsigned char PLL_T[3] = { 0x00, 0x00, 0x82 };  // timer
unsigned char PLL_N[3] = { 0x10, 0x3E, 0x81 };  // counter
#else // nouvelle PLL, ADF4111
unsigned char PLL_R[3] = { 0x18, 0x80, 0xf3 };  // FoLD=0, CPo=4, R=320
unsigned char PLL_T[3] = { 0x00, 0x03, 0x20 };  // timer
unsigned char PLL_N[3] = { 0x01, 0xf4, 0x01 };  // counter
#endif

#ifdef periodique
extern volatile int reveil;
#endif

void init_PLL()
{
  envoi_PLL (PLL_R);
  delay (10);
  envoi_PLL (PLL_T);
  delay (10);
  envoi_PLL (PLL_N);
  delay (20000);
  remont_CS_PLL ();             // CS_PLL# = false
}

void init_DDS()
{int SEC;
  for (SEC = 0; SEC < 10; SEC++)
    {
// init DDS
#ifndef AD9958
      envoi_DDS (CFR1, 5);
      envoi_DDS (CFR2, 4);
      envoi_DDS (ASF, 3);
      envoi_DDS (ARR, 2);
      envoi_DDS (FTW0, 5);
      envoi_DDS (POW0, 3);
      envoi_DDS (FTW1, 5);
      envoi_DDS (RSCW0, 6);
      envoi_DDS (RSCW1, 6);
      //envoi_DDS(RSCW2,6);
      //envoi_DDS(RSCW3,6);
#else
        envoi_DDS (CSR,2);
        envoi_DDS (FR1,4);
        envoi_DDS (FR2,3);
        envoi_DDS (CFR,4);
        envoi_DDS (FTW0,5);
        envoi_DDS (ACR,4);
#endif
      delay (50);
    }
}

void delay (int length)
{ volatile int tmplength=length/2+length/4;	// 161121 : astuce gcc-6.2 : retire inline asm et ajuste length pour verifer calibrage ci-dessous
  while (tmplength >=0) tmplength--; 		// 45=28.43 us teste', inline asm => compiler en -O1
//  asm volatile (
//       "   ADD         R13,#0x4	       \n": : :);
}

void
jmf_putchar (int a,char *chaine, int* chaine_index,__attribute__((unused))int port)
{
  if (chaine != NULL)
    {chaine[*chaine_index]=a;
     *chaine_index=*chaine_index+1;
    }
  else
    {
     if (affiche_rs232==1)
       {do { } while ((COMSTA0 & 0x020) != 0x20);
        COMTX = a;
       }
    }
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
      #ifdef SetH
          GP0DAT &= ~0x00400000;
      #endif
	  ADCCON = 0x7e3;	// 7E3: 5 kHz avec delay=50, 15 kHz avec delay=0
	  while (!ADCSTA)
	    {
	    };			// 17 kHz si on passe a 0E3 avec tps de charge=25 ou 15
	  v = ((unsigned short) (ADCDAT >> 16));
      #ifdef SetH
          GP0DAT |= 0x00400000;
      #endif
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
puiss (unsigned short puissance,__attribute__((unused)) unsigned char unused)
{
  unsigned char p;
  p = (puissance & 0x02) << 3 | (puissance & 0x04) << 1 | (puissance & 0x08) >> 1 | (puissance & 0x10) >> 3 | (puissance & 0x01);	// V1, V2, V3 ,V4, V5
  GP0DAT = ((GP0DAT & 0xFFE00000) | (int) p << 16);	// Mot constitue : FF XXXP PPPP  
}

unsigned short
interroge (unsigned short puissance,unsigned int freq,unsigned int offset,__attribute__((unused))unsigned char chan)
{
  unsigned short res;
  unsigned short v;

  FTW0[1] = (freq & 0xFF000000) >> 24;
  FTW0[2] = (freq & 0xFF0000) >> 16;
  FTW0[3] = (freq & 0xFF00) >> 8;
  FTW0[4] = (freq & 0xFF);
 
  set_DAC2(offset<<16);
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
	  puiss (pulse_shape[res],3);
	}
      delay (EMET);	// tps de charge  PLUS C'EST LONG, PLUS LE PIC EST FIN
      for (res = puissance - 1; res >= NY; res--)
	{
	  puiss (pulse_shape[res],3);
	}
      puiss (0,3);
    }
  else
    {
      puiss (puissance,3);
      delay (EMET);	// tps de charge
    }

// while (1) puiss (31,3); // TP RF

  puiss (0,3);
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
#ifdef MODBUS
        pxMBFrameCBByteReceived(  );
#else
      global_tab[global_index] = my_getchar ();
      if (global_index < (NB_CHARS - 1))
	global_index++;
#endif
    }
  if (IRQSIG & GP_TIMER_BIT)
    {				// Timer1 Interrupt      
      GLOBAL_TIMER1++;
      T1CLRI = 1;		// Clear Timer 1 interrupt
    }
  if (IRQSIG & RTOS_TIMER_BIT)
    {				// Timer0 Interrupt      
      tim0++;
#ifdef MODBUS
      tim0modbus++;
    {				// Timer0 Interrupt      
      if (tim0modbus<255) tim0modbus++;
      if (tim0modbus>=2) ( void )pxMBPortCBTimerExpired(  );
//   T0LD = (16320/10);		// 100 Hz
    }
#endif
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
#ifdef arcelor
  GP1DAT = 0x88800000;          // P1.3 as output, P1.2 as input (Arcelor switch)
#else
  GP1DAT = 0x8C800000;          // P1.3 & P1.2 as output (extension port)
#endif
  SPIDIV = 0x06;		// set SPI clock 40960000/(2x(1+SPIDIV))->0x00=21MHz
  SPICON = 0x1043;              // 3: SPI ENA, Master 4: transfer by writing to SPITX 1: continuous
  // slave select will stay low during the all transmission
  GP4DAT = 0xff100000;		// P4.4 et P4.2 sont configurées en sortie.
  GP2DAT = 0xff0F0000;		// P2.3 = 0;
  GP3DAT = 0xff070000;
  // Start setting up UART at RS_BAUDRATE
  COMCON0 = 0x080;		// Setting DLAB = autorise modification baudrate
  COMDIV1 = 0x000;		// NE PAS LANCER PAR RUN MAIS RESET DE LA CARTE
  set_serial_speed(RS_BAUDRATE,0);// COMCON0 est mis a 7 dans set_serial
  COMIEN0 = 1;
//mesure de la fonction delay
/*	while(1)
	{
 	GP4DAT &= ~0x00400000;
	delay(45);
	GP4DAT |= 0x00400000;	 
	 delay(45);	
	}*/

#ifdef debug
write_str("Init CPU\r\n",NULL,NULL,0);
#endif
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
  T0LD = (16320);		// 10 Hz

  //T1LD=0;      //  32 bits; 
  //T1CON=0x188; //  8 => core/256, 1=count UP
  CMPCON = 0x4C4;

  T1LD = 1500;			// 41780000/256, 32 bits; 
  T1CON = 0x44;			// 4=> core/256, periodic mode, inactif (C4 pour actif)
#ifdef periodique 
 T2CON=0xE8;      // Timer2 counts down, format Hr:Min:Sec:1/100Sec and prescale 256
 T2LD=0x00050000; // intervalle de temps en min (ici 50000=5 min) 
#endif

  // 8/07/2013 : ajout du watchdog avec timeout a 4 s
  T3LD=512;  // decrement from T3LD (load BEFORE setting T3CON MMR): 512=4 s
  T3CON=0x00A8;// watchdog mode, timer enable, 32768/256=128 Hz steps

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

#ifdef auto_offset
void
corrige_offset (unsigned int *dac2)
{
  int adc;
  int cpt = 0, antenne, moy;
  for (antenne = 0; antenne < ANTENNES; antenne++)
    {
      write_str ("auto offset",NULL,NULL,0);
      jmf_putchar ('0'+antenne,NULL,NULL,0);
      write_str (": ",NULL,NULL,0);
      dac2[antenne] = 0;
      do
	{
	  DAC2DAT = ((dac2[antenne]) << 16); // ERREUR : dac2 est index'e par CAPTEUR maintenant
	  adc = 0;
	  for (moy = 0; moy < NBMOY; moy++)
	    adc += interroge (31,Fintsta[antenne * nbpics[antenne]],dac2[antenne],3);
	  adc /= NBMOY;
	  dac2[antenne] += (adc - (SEUILMIN)) / 16;
	  if (dac2[antenne] < 0)
	    dac2[antenne] = 0;
	  if (dac2[antenne] > 0xFFF)
	    dac2[antenne] = 0xfff;
#ifdef debug
	  writeHEXi (dac2[antenne],NULL,NULL,0);
	  jmf_putchar (' ',NULL,NULL,0);
#endif
	  cpt++;
	}
      while ((adc > SEUILMIN) && (cpt < 50));
      if (dac2[antenne] < (0xf7f))
	dac2[antenne] += (0x080);
      else
	dac2[antenne] = (0xfff);
      write_str ("-> ",NULL,NULL,0);
      writeHEXi (dac2[antenne],NULL,NULL,0);
    }
  write_str ("\r\n",NULL,NULL,0);
}
#endif

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

char read_switch()
{ return(GP1DAT&0x04); // return P1.2, 100 kohms pull up enabled
}

void set_serial_speed(int ntmp,int parity)
{ if ((ntmp==115200) || (ntmp==57600) || (ntmp==19200) || (ntmp==38400) || 
     (ntmp==9600))  // ajouter 4800 ?
  {
  COMCON0 = 0x080;  // Setting DLAB
  switch (ntmp)
    {case 115200: COMDIV0= 0x0b; break;
     case 57600 : COMDIV0= 0x17; break;
     case  9600 : COMDIV0= 0x88; break;
     case 38400 : COMDIV0 = 0x22;break;
     case 19200 : 
     default:     COMDIV0= 0x44; break;
    }
  COMCON0 = 0x007;  // Clearing DLAB
  }
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

void kick_watchdog()
{T3CLRI=1; // kick the watchdog
}

void clignote() {
GP3DAT ^= 0x00040000;
}

#ifdef MODBUS
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
    jmf_putchar (ucByte,NULL,NULL,1);
    return TRUE;
}


BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{ 
  *pucByte=my_getchar ();
  return TRUE;
}

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
    tim0=0;
    return TRUE;
}

void
vMBPortTimersEnable( void )
{
  tim0=0;
}

void
vMBPortTimersDisable( void )
{tim0=255;
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

#endif

void dephase(unsigned short phase,__attribute__((unused)) unsigned char chan)
{DAC3DAT = (phase << 16);}

void safran_init()
{ // power up
 GP1DAT&=~(1<<19); // P1.3 low (CS#)
 SPITX  = 0x10;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x00;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x0F;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(100);
 GP1DAT|=(1<<19); // P1.3 low (CS#)
 delay(100);
   // output 10V range both DAC
 GP1DAT&=~(1<<19); // P1.3 low (CS#)
 SPITX  = 0x0C;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x00;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x01;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(1);
 GP1DAT|=(1<<19); // P1.3 low (CS#)
 delay(100);
   // load
 GP1DAT&=~(1<<19); // P1.3 low (CS#)
 SPITX  = 0x0C;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x00;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 SPITX  = 0x01;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(100);
 GP1DAT|=(1<<19); // P1.3 low (CS#)
 delay(100);
}

void safran_dac(char chan,unsigned short v) // chan = 1 ou 2
{GP1DAT&=~(1<<19); // P1.3 low (CS#)
 SPITX  = chan;
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(1);
 SPITX  = ((v&0xff00)>>8);
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(1);
 SPITX  = (v&0x00ff);
 do {} while ((SPISTA & 0x01) == 0x01);	// wait for data rcv status bit
 delay(100);
 GP1DAT|=(1<<19); // P1.3 low (CS#)
 delay(100);
}
