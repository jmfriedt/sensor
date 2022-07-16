// generaliser bande2 a plus que 2 resonances 

// virer acquisition pour remplacer par balaie_ism ?

// mesure 120201: NBMOY=1, NBPICS=3, mode_balayage_continu=0, bandes=1 
//      => 57600 bauds=163 Hz, 115200 bauds=285 Hz d'echantillonage

// ATTENTION : dans le cas de manuel, on ne peut PAS exploiter nbpics variable
//   car j'indexe Fintsta/Fintsto par [npic+antenne*nbpics[antenne]] qui ne
//   marche que si nbpics[antenne]=NBPICS pour toute valeur de antenne

// tester LBT
// fit des moyennes au lieu de moyenne des fits -> valable pour du bruit non-blanc (DP)
// bandes == 0 et 3 points possible ?

// ATTENTION : pour que le mode 3 points marche, il FAUT que NBRATES>1
//   UNE ALTERNATIVE EST DE METTRE un test >0 au lieu de >100 ligne 783
// modification sandvik 110708 : sortie de frequence 1 et 2 sur DAC1 et 3
//   (pins 11 et 13 respectivement)
// nbpics[antenne]=0 desactive l'antenne, verifie' 110803 
//   (correction fstep=fstop-fstart/nbpics -> /NBSTEPS)

// AT0 ne joue plus sur bande (AT=) mais A0 avec AT1 et AT2 + stockage flash

// TGCP : suppose que dephaseur est commande' par DAC3 (conflit avec sandvik)

extern unsigned char affiche_rs232;

#define K 1			// nombre de pts a droite et a gauche en 3points : 1 => 3 points

#include "DDSinc.h"
#include "DDSvar.h"
#include "version.h"

#include <stdlib.h>
#include <string.h>
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

#include "config_modbus.h"
#ifdef MODBUS
#include "mb.h"
#include "mbport.h"
#include "freemodbus-v1.5.0senseor/modbus/rtu/mbrtu.h" // definition des typedef enum eMBRcvState, eMBSndState
USHORT   usRegInputStart = REG_INPUT_START;
USHORT   usRegInputBuf[REG_INPUT_NREGS];
USHORT   usRegHoldingStart = REG_HOLDING_START;
USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
extern UCHAR    ucMBAddress;
extern eMBSndState eSndState;
extern eMBRcvState eRcvState;
UCHAR    ucMBAddress_tmp=MODBUS_SLAVE_ADD;  // voir dans freemodbus-v1.5.0senseor/modbus/rtu/mbrtu.c
volatile char init_status=0;
volatile int tim0modbus = 255;  // tim0modbus initialement desactiv'e
volatile int proportional=(PROPORTIONAL_INIT-1);  // proportional cst pour mise en route sinon ...
int      rs_tmp,rs_tmp2;
#else
//volatile int proportional=144; 	// ... proportional constant for 2-point method 
volatile int proportional=(PROPORTIONAL_INIT-1);  // proportional cst pour mise en route sinon ...
#endif

unsigned char global_tab[NB_CHARS];
volatile int global_index = 0, tim0 = 0, heure = 0, minute = 0, seconde = 0,modif_AT=0;
volatile int montim0 = 0;
volatile unsigned int mont1 = 0, mont2 = 0;
volatile unsigned int GLOBAL_TIMER1 = 0;
unsigned int nbpics[ANTENNES_MAX];
unsigned short courant=0;

#ifdef periodique
volatile int reveil=0;
#endif

#ifdef gilloux
volatile phase_mag=0;
#endif

//////////////////// methode 2 points ///////////////////////////////

//#ifndef AD9958
//extern unsigned char CFR1[5];  // residu de la definition du DDS dans DDS.c : a deplacer dans init_*.c
//#endif

// ATTENTION : attente de 15 us au lieu de 30 dans la configuration actuelle

// separer asservissement puissance senseor et FM

// Programme d'identification et d'acquisition de freq. de resonance //

#define ASSERVISSEMENT_PUISSANCE
#define Affichage_frequence_non_trouve
#undef Mode_FM_2p_et_senseor

// Declaration des fonctions             
//-----------------------------------------------------------------------------/
int acquisition (unsigned int *, unsigned int *, unsigned int *, int *,
		 unsigned short *, unsigned int *, unsigned int *);
int asservissement_sur_phase (unsigned int *, unsigned char, unsigned short *,
			      unsigned int *, unsigned short *,
			      unsigned char *);
void recherche_asservissement_puissance_senseor (unsigned int *,
						 unsigned int *,
						 unsigned int *,
						 unsigned short *);
////////////////// fin methode 2 points ////////////////////////////

unsigned short jmfhtons(unsigned short i)
{return((((i&0xff00)>>8)|(i&0x00ff)<<8));}

unsigned int jmfhtonl(unsigned int i)
{return(((i&0xff000000)>>24)|((i&0x00ff0000)>>8)|((i&0x0000ff00)<<8)|((i&0x000000ff)<<24));}

void write_str (char *pString,char* chaine,int* chaine_index,int port)
{
  while (*pString != 0x00)
    jmf_putchar (*pString++,chaine,chaine_index,port);
}

// modif 26-11-2010 : ajout des 0 en debut de trame
void
writeDECs (unsigned short d,char *chaine,int* chaine_index,int port)
{
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48,chaine,chaine_index,port);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000) * 1000;
    }
  else				// if (sup!=0) 
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48,chaine,chaine_index,port);
      d = d - (d / 100) * 100;
    }
  else				// if (sup!=0) 
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

void
writeDECheure (unsigned char d,char *chaine,int* chaine_index,int port)
{
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

// modif 26-11-2010 : ajout des 0 en debut de trame
void
writeDECc (unsigned char d,char *chaine,int* chaine_index,int port)
{
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48,chaine,chaine_index,port);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

//#ifdef convertit_temperature
void
writeDECTi (int d,char *chaine,int* chaine_index,int port)
{
  if (d < 0)
    {
      jmf_putchar ('-',chaine,chaine_index,port);
      d = -d;
    }
  if (d > 9999)
    d = 9999;
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000) * 1000;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48,chaine,chaine_index,port);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar ('.',chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

//#endif

void
writeDEC0i (int d,char *chaine,int* chaine_index,int port)
{
  if (d < 0)
    {
      jmf_putchar ('-',chaine,chaine_index,port);
      d = -d;
    }
  if (d > 99999)
    d = 99999;
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48,chaine,chaine_index,port);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000) * 1000;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48,chaine,chaine_index,port);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

// pour maximiser vitesse, commenter le writeDEC0i ci dessus et decommenter 
// ci-dessous
// #define writeDECi writeDEC0i

void
writeDECi (int d,char *chaine,int* chaine_index,int port)
{
  int sup = 0;
  if (d < 0)
    {
      jmf_putchar ('-',chaine,chaine_index,port);
      d = -d;
    }
  if (d > 999999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000000000) * 1000000000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99999999)
    {
      sup = 1;
      jmf_putchar ((d / 100000000) + 48,chaine,chaine_index,port);
      d = d - (d / 100000000) * 100000000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9999999)
    {
      sup = 1;
      jmf_putchar ((d / 10000000) + 48,chaine,chaine_index,port);
      d = d - (d / 10000000) * 10000000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000000) * 1000000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99999)
    {
      sup = 1;
      jmf_putchar ((d / 100000) + 48,chaine,chaine_index,port);
      d = d - (d / 100000) * 100000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9999)
    {
      sup = 1;
      jmf_putchar ((d / 10000) + 48,chaine,chaine_index,port);
      d = d - (d / 10000) * 10000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 999)
    {
      sup = 1;
      jmf_putchar ((d / 1000) + 48,chaine,chaine_index,port);
      d = d - (d / 1000) * 1000;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 99)
    {
      sup = 1;
      jmf_putchar ((d / 100) + 48,chaine,chaine_index,port);
      d = d - (d / 100) * 100;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48,chaine,chaine_index,port);
      d = d - (d / 10) * 10;
    }
  else if (sup != 0)
    jmf_putchar (48,chaine,chaine_index,port);
  jmf_putchar (d + 48,chaine,chaine_index,port);
}

void
writeHEXptri (int *ptr,char *chaine,int* chaine_index,int port)
{
  unsigned char b;
  {
    b = ((*ptr & 0xf0000000) >> 28);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x0f000000) >> 24);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x00f00000) >> 20);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x000f0000) >> 16);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x0000f000) >> 12);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x00000f00) >> 8);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x000000f0) >> 4);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((*ptr & 0x0000000f));
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
  }
}

void
writeHEXi (unsigned int ptr,char *chaine,int* chaine_index,int port)
{
  unsigned char b;
  {
    b = ((ptr & 0xf0000000) >> 28);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x0f000000) >> 24);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x00f00000) >> 20);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x000f0000) >> 16);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x0000f000) >> 12);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x00000f00) >> 8);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x000000f0) >> 4);
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x0000000f));
    if (b < 10) jmf_putchar (b + 48,chaine,chaine_index,port);
    else jmf_putchar (b + 55,chaine,chaine_index,port);
  }
}

void
writeHEXs (unsigned short ptr,char *chaine,int* chaine_index,int port)
{
  unsigned char b;
  {
    b = ((ptr & 0xf000) >> 12);
    if (b < 10)
      jmf_putchar (b + 48,chaine,chaine_index,port);
    else
      jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x0f00) >> 8);
    if (b < 10)
      jmf_putchar (b + 48,chaine,chaine_index,port);
    else
      jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x00f0) >> 4);
    if (b < 10)
      jmf_putchar (b + 48,chaine,chaine_index,port);
    else
      jmf_putchar (b + 55,chaine,chaine_index,port);
    b = ((ptr & 0x000f));
    if (b < 10)
      jmf_putchar (b + 48,chaine,chaine_index,port);
    else
      jmf_putchar (b + 55,chaine,chaine_index,port);
    jmf_putchar (32,chaine,chaine_index,port);
  }
}

void
writeHEXc (unsigned char ptr,char *chaine,int* chaine_index,int port)
{
  unsigned char b;
  b = ((ptr & 0x00f0) >> 4);
  if (b < 10)
    jmf_putchar (b + 48,chaine,chaine_index,port);
  else
    jmf_putchar (b + 55,chaine,chaine_index,port);
  b = ((ptr & 0x000f));
  if (b < 10)
    jmf_putchar (b + 48,chaine,chaine_index,port);
  else
    jmf_putchar (b + 55,chaine,chaine_index,port);
  jmf_putchar (32,chaine,chaine_index,port);
}

int
sauve_param ()
{
  int memoire[ANTENNES_MAX*(NBPICS_MAX+1)*3+26];
  int iindex;
  int k;
  unsigned int kk;

  iindex = 0;
  write_str("Writing to non-volatile memory\r\n\0",NULL,NULL,usart_port);
  memoire[iindex++] = 0x55;
  memoire[iindex++] = 0xAA;
  memoire[iindex++] = 0x00;  // size lo
  memoire[iindex++] = 0x00;  // size hi
  memoire[iindex++] = bandes;
  memoire[iindex++] = LBT;
  memoire[iindex++] = ATTENTE;
  memoire[iindex++] = HATTENTE;
  memoire[iindex++] = NBMOY;
  memoire[iindex++] = NBPICS;
  memoire[iindex++] = NBPICS_MAX;
  memoire[iindex++] = NBSTEPS;
  memoire[iindex++] = ANTENNES;
  memoire[iindex++] = ANTENNES_MAX;
  memoire[iindex++] = (SEUILMIN);
  memoire[iindex++] = (SEUILMAX);
  memoire[iindex++] = EMET;
  memoire[iindex++] = NY;
  memoire[iindex++] = RS_BAUDRATE;
  memoire[iindex++] = RS_PARITY;
  memoire[iindex++] = convertit_temperature;
  if (convertit_temperature == 1)
    {for (k=0;k<ANTENNES_MAX*NBPICS_MAX;k++)
       {memoire[iindex++] = A0[k];
        memoire[iindex++] = A1[k];
        memoire[iindex++] = A2[k];
       }
    }
/*
  for (k = 0; k < ANTENNES; k++)      // pas vraiment un pb car reflasher efface
    {
      memoire[iindex++] = nbpics[k];	//  le contenu de l'EEPROM avec aducloader
      for (kk = 0; kk < nbpics[k]; kk++) memoire[iindex++] = Fintsta[kk];
      for (kk = 0; kk < nbpics[k]; kk++) memoire[iindex++] = Fintsto[kk];
    }
*/
//  memoire[iindex++]=(int)VERSION;    // a revoir : VERSION est char*
  memoire[iindex++] = (int) mode_balayage_continu;
  memoire[iindex++] = (int) agc;
#ifdef MODBUS
  memoire[iindex++] = ucMBAddress;
#else
  memoire[iindex++] = 0x42;
#endif
  memoire[2]=(iindex&0xff);
  memoire[3]=((iindex>>16)&0xff);

writeDECi(iindex,NULL,NULL,usart_port);
#ifdef at25
  at25_erase (0x00);
  at25_write (0x00, (unsigned char *) memoire, sizeof (int) *iindex);
#endif
#ifdef eeprom
  eeprom_erase (0x00);
  eeprom_write (0x00, (unsigned char *) memoire,sizeof (int)*iindex);
#endif
  write_str(" data written\r\n\0",NULL,NULL,usart_port);
  return (iindex);
}

int
lit_param ()
{
  int memoire[ANTENNES_MAX*(NBPICS_MAX+1)*3+26];
  int iindex = 0;
  int k;
  unsigned int kk; // ma_version; 
  memoire[0]=0;memoire[1]=0;

#ifdef at25
  at25_read (0x00, (unsigned char *) memoire, 16);  // taille en char
#endif
#ifdef eeprom
  eeprom_read (0x00, (unsigned char *) memoire,16); // taille en char
#endif
  if ((memoire[0] == 0x55) && (memoire[1] == 0xAA))
    {
      iindex=(int)memoire[2]+256*(int)memoire[3];
#ifdef at25
  at25_read (0x0, (unsigned char *) memoire, sizeof (int)*(iindex);   // taille en char
#endif
#ifdef eeprom
  eeprom_read (0x0, (unsigned char *) memoire, sizeof (int)*(iindex)); // taille en char
#endif
      iindex=4;
      bandes = memoire[iindex++];
      LBT = memoire[iindex++];
      ATTENTE = memoire[iindex++];
      HATTENTE = memoire[iindex++];
      NBMOY = memoire[iindex++];
      NBPICS = memoire[iindex++];
      k= memoire[iindex++];
      if (k!=NBPICS_MAX) write_str("PROBLEM: NBPICS_MAX not consistent\r\n\0",NULL,NULL,usart_port);
      NBSTEPS = memoire[iindex++];
      ANTENNES = memoire[iindex++];
      k= memoire[iindex++];
      if (k!=ANTENNES_MAX) write_str("PROBLEM: ANTENNES_MAX not consistent\r\n\0",NULL,NULL,usart_port);
      SEUILMIN =memoire[iindex++];
      SEUILMAX =memoire[iindex++];
      EMET = memoire[iindex++];
      NY = memoire[iindex++];
      RS_BAUDRATE = memoire[iindex++];
#ifdef MODBUS
      rs_tmp=RS_BAUDRATE;
#endif
      RS_PARITY = memoire[iindex++];  
#ifdef MODBUS
      rs_tmp2=RS_PARITY;
#endif
      convertit_temperature = memoire[iindex++];
      if (convertit_temperature == 1)
	{
         for (k=0;k<ANTENNES_MAX*NBPICS_MAX;k++)
	   {A0[k] = memoire[iindex++];
	    A1[k] = memoire[iindex++];
	    A2[k] = memoire[iindex++];
           }
        }
/*
      for (k = 0; k < ANTENNES; k++)
	{
	  nbpics[k] = memoire[iindex++];
	  for (kk = 0; kk < nbpics[k]; kk++) Fintsta[kk] = memoire[iindex++]; 
	  for (kk = 0; kk < nbpics[k]; kk++) Fintsto[kk] = memoire[iindex++];
	}
*/
//      k=memoire[iindex++];  //  if (ma_version!=VERSION) ...  <- complique' car str()
      mode_balayage_continu = (unsigned char) memoire[iindex++];
      agc = (unsigned char) memoire[iindex++];
#ifdef MODBUS
      ucMBAddress=memoire[iindex++]; ucMBAddress_tmp=ucMBAddress;
#else
      k=memoire[iindex++];
#endif
      for (k=0;k<iindex;k++) {writeDECc(k,NULL,NULL,usart_port);  // 100 -> iindex
                          write_str(" \0",NULL,NULL,usart_port);
                          writeDECi(memoire[k],NULL,NULL,usart_port);
                          write_str("\r\n\0",NULL,NULL,usart_port);
                         }
      write_str("config. recovered\r\n\0",NULL,NULL,usart_port);
    }
  else write_str("NG\r\n\0",NULL,NULL,usart_port);
  return (iindex);
}

/*
void envoi_java(unsigned int *entree, int *hpic, unsigned char puissance)
{unsigned char tablo[3+6*NBPICS_MAX+3];
 int k;

 tablo[0]=0x55;
 tablo[1]=0xAA;									  
 tablo[2]=NBPICS;
 for (k=0;k<NBPICS-1;k++)
 {
 // freq1 * 4
 entree[k]=entree[NBPICS-1]-entree[k];
 tablo[k*6+3]=(entree[k]&0xFF000000)>>24;	   // 4 octets de frequence
 tablo[k*6+4]=(entree[k]&0x00FF0000)>>16;
 tablo[k*6+5]=(entree[k]&0x0000FF00)>>8;
 tablo[k*6+6]=(entree[k]&0x000000FF);

 tablo[k*6+7]=(hpic[k]&0x0000FF00)>>8;		   // 2 octets de hpic
 tablo[k*6+8]=(hpic[k]&0x000000FF);	
 }

 tablo[(NBPICS-1)*6+3]=((entree[NBPICS-1])&0xFF000000)>>24;	   // 4 octets de frequence
 tablo[(NBPICS-1)*6+4]=((entree[NBPICS-1])&0x00FF0000)>>16;
 tablo[(NBPICS-1)*6+5]=((entree[NBPICS-1])&0x0000FF00)>>8;
 tablo[(NBPICS-1)*6+6]=((entree[NBPICS-1])&0x000000FF);

 tablo[(NBPICS-1)*6+7]=(hpic[k]&0x0000FF00)>>8;		   // 2 octets de hpic
 tablo[(NBPICS-1)*6+8]=(hpic[k]&0x000000FF);	


 tablo[NBPICS*6+3]=puissance;   // gain
 tablo[NBPICS*6+4]=0x15; // DAC
 tablo[NBPICS*6+5]=0x16; // DAC
 my_write(tablo,3+6*NBPICS+3);				// NBPICS=2 => 150 ms !
}
*/

int
parabole (unsigned short *entree, unsigned short i, int fstep)
{
  int f0;
  int denom;
  denom = (int) ((entree[i - 1] + entree[i + 1] - 2 * entree[i])) * 2;
  if (denom != 0)
    f0 = fstep * (int) ((entree[i - 1] - entree[i + 1])) / denom;	// 
  else
    f0 = 0;
  return (f0);
}

//----------------------------------------------------------------------------//
// Cette fonction recherche le max de la courbe dans les 2 seuils
//----------------------------------------------------------------------------//
void
bete_max (unsigned short *entree, int n, int *moyenne)
{
  int max_courbe = 0, max = 0, nmax = 0, i;
  for (i = 1; i < n - 1; i++)
    {
#ifdef debug
      // writeDECi(entree[i],NULL,NULL);write_str(" \0",NULL,NULL);
#endif
      if (entree[i] > max_courbe) {max_courbe = entree[i];}
      if ((entree[i] < SEUILMAX) && (entree[i] > SEUILMIN))
	{if ((entree[i-1] < entree[i]) && (entree[i+1] < entree[i])
	      && (entree[i-1] > SEUILMIN) && (entree[i+1] > SEUILMIN))
	    {if (entree[i] > max)
		{		//printf(" %d %d %d %d",entree[i-1],entree[i],entree[i+1],i);
		  nmax = i;
		  max = entree[i];
		}
	    }
	}
    }
  if (max_courbe > SEUILMAX)	// signal depassant le max => ne comptabilise pas pic
    { moyenne[0] = 0; moyenne[1] = max_courbe; }
     else if (((nmax==1)||(nmax==(n-2)))&&(mode_balayage_continu==1)) {moyenne[0]=nmax;moyenne[1]=SEUILMAX;}
       else { moyenne[0] = nmax; moyenne[1] = max; }
}

void
aff_osc (int freq, int antenne, unsigned short p,unsigned int offset,char *chaine,int* chaine_index)
{
  unsigned short res = 0;
  res = interroge (p,freq,offset,3)&0xffff;
#ifndef LMD
  writeDECc ((unsigned char) (nbpics[antenne]),chaine,chaine_index,usart_port);
  jmf_putchar (' ',chaine,chaine_index,usart_port);
  writeDEC0i ((int) SEUILMIN,chaine,chaine_index,usart_port);
  jmf_putchar (' ',chaine,chaine_index,usart_port);
#endif
  jmf_putchar ('4',chaine,chaine_index,usart_port);
#ifndef f400
  writeDECi (((((freq-0x20000000) >> 4) * 40) / 859) * 16 + 24999809,chaine,chaine_index,usart_port);
#else
#ifndef AD9958
  writeDECi (((((freq-0x15000000) >> 4) * 80) / 859) * 16 + 32812250,chaine,chaine_index,usart_port);
#else
  writeDECi (((((freq-0x40000000) >> 5) *155) /1958) * 32 +24999991,chaine,chaine_index,usart_port);  
  //rats(340e6/2^32) 249999991=0x40000000/1958*155-60 MHz ; 32/2^32*340 MHz=2.5 Hz
#endif
#endif
  jmf_putchar (' ',chaine,chaine_index,usart_port);
#ifndef LMD
  writeDEC0i ((int) SEUILMAX,chaine,chaine_index,usart_port);
  jmf_putchar (' ',chaine,chaine_index,usart_port);
#endif
  writeDEC0i (res,chaine,chaine_index,usart_port);
#ifndef LMD
  jmf_putchar (' ',chaine,chaine_index,usart_port);
  write_str ("01 \0",chaine,chaine_index,usart_port);
  writeDECs (temperature (),chaine,chaine_index,usart_port);
#endif
  jmf_putchar ('\r',chaine,chaine_index,usart_port);
  jmf_putchar ('\n',chaine,chaine_index,usart_port);
  chaine[*chaine_index]=0;                // jusqu'ici on a forme' la chaine, ...
  write_str(chaine,NULL,NULL,usart_port); // ... et ici on l'envoie sur le port serie *chaine_index=0;
  *chaine_index=0;
}

void
oscilloscope (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep,
	      unsigned short *puissance,unsigned int *offset,int *phases)
{
  unsigned int npic;
  int antenne;
  unsigned int freq;
#ifndef LMD
  unsigned short res = 0;
#endif
  char chaine[80];
  int chaine_index=0;

#ifdef LMD
  jmf_putchar ('*',chaine,&chaine_index,usart_port);
  jmf_putchar (' ',chaine,&chaine_index,usart_port);
#endif
  for (antenne = 0; antenne < ANTENNES; antenne++)
    {
      sortie_antenne (antenne);
      if (bandes == 1)
	for (npic = 0; npic < nbpics[antenne]; npic++)	// Fint au lieu de fstart/stop pour le cas 3points
{
#ifdef dephaseur
          dephase(phases[antenne * NBPICS_MAX+npic],3);
#endif
	  for (freq=fstart[npic+antenne*NBPICS_MAX];freq<fstop[npic+antenne*NBPICS_MAX];
//	  for (freq=Fintsta[antenne*nbpics[antenne]+npic];freq<Fintsto[antenne*nbpics[antenne]+npic];
	       freq += fstep[npic+antenne*NBPICS_MAX])
	         {aff_osc(freq,antenne,puissance[antenne*NBPICS_MAX+npic],offset[antenne*NBPICS_MAX/2+npic/2],chaine,&chaine_index);
                  kick_watchdog();
                 }
}
      else {
#ifdef dephaseur
          dephase(phases[antenne * NBPICS_MAX],3);
#endif
   	  for (freq=fstart[antenne*NBPICS_MAX];freq<fstop[antenne*NBPICS_MAX];
	       freq+=fstep[antenne*NBPICS_MAX])
	       { aff_osc (freq,antenne,puissance[antenne*NBPICS_MAX],offset[antenne*NBPICS_MAX/2],chaine,&chaine_index);
                 kick_watchdog();
               }
            }
    }
#ifndef LMD
  writeDECc ((unsigned char) (nbpics[0]),chaine,&chaine_index,usart_port); // attention a ne pas
  jmf_putchar (' ',chaine,&chaine_index,usart_port);                       // indexer nbpics[antenne]
  writeHEXi ((int) 0xffffffff,chaine,&chaine_index,usart_port);            // car en fin de boucle
  jmf_putchar (' ',chaine,&chaine_index,usart_port);                       // antenne n'est pas valide
  writeDECs (res,chaine,&chaine_index,usart_port);
  jmf_putchar (' ',chaine,&chaine_index,usart_port);
  writeHEXi ((int) 0xffffffff,chaine,&chaine_index,usart_port);
  jmf_putchar (' ',chaine,&chaine_index,usart_port);
  writeDECs (res,chaine,&chaine_index,usart_port);
  jmf_putchar (' ',chaine,&chaine_index,usart_port);
  write_str ("01 \0",chaine,&chaine_index,usart_port);
  writeDEC0i ((int) temperature (),chaine,&chaine_index,usart_port);
  jmf_putchar ('\r',chaine,&chaine_index,usart_port);
  jmf_putchar ('\n',chaine,&chaine_index,usart_port);
  chaine[chaine_index]=0;
  write_str(chaine,NULL,NULL,usart_port); // jusqu'ici on a forme' la chaine, et ici on l'envoie sur le port serie
#endif
}

int
ma_classe (const void *i, const void *j)
{
  if (*(unsigned int *const *) i == 0)
    return (1);
  if (*(unsigned int *const *) j == 0)
    return (-1);		// renvoie les 0 a la fin ?
  if (*(unsigned int *const *) i < *(unsigned int *const *) j)
    return (-1);
  else
    return (1);
}

void filtre_hpf(short *entree,int n)   // convolution par [-1 0 0 0 2 0 0 0 -1]
// FAUX : la convolution exploite [i-4] qui a ete modifie' lors du balaye => il faut un tablo tmp
{int i;
 for (i = 4; i < n - 4; i++) entree[i-4]=entree[i]+entree[i]-entree[i-4]-entree[i+4];
 for (i = 4; i < n - 4; i++) entree[i]=entree[i-4];     // jmfxx : la copie est inutile ?
 for (i = 0; i < 4; i++) {entree[i]=0;entree[n-1-i]=0;} // jmfxx : juste mettre 8 derniers pts a 0 ?
#ifdef debug
 write_str("\r\n filtrage\0",NULL,NULL,usart_port);
#endif
}

int balayage_bande0 (unsigned int *fstart, unsigned int *fstop,
		 unsigned int *fstep, int *hpic, unsigned int *parab,
		 unsigned short *puissance, unsigned int *variance,
		 char *balayage_primaire,unsigned int *offset,
		 unsigned char Nb_resonateur, int antenne, int *phases)
{
  int SEC = 0;
  unsigned int npic, tmp2; 
  int bsp=0,total=0;
  int moyenne[2], somme;
  unsigned int freq, freqres;	// bonsweep[NBPICS],
  unsigned int tab_variance[NBPICS_MAX * (NBMOY_MAX + 1) * NBRATES];
  int tmp ;
  unsigned int deja_detect = 0;
  int deja_detect_pos[NBPICS_MAX];
  int deja_detect_freq[NBPICS_MAX];
  int deja_detect_hpic[NBPICS_MAX];
  unsigned short tmpc;
#ifdef detec_tous
  unsigned char bsptmp[NBMOY_MAX * NBRATES * NBPICS_MAX];
  unsigned short save_sec[NBPICS_MAX];
  unsigned short tab0[(NBSTEPS_MAX + 2) * NBPICS_MAX];	// *NBMOY
  unsigned short nbmoy[NBPICS_MAX];
#else
  unsigned short bsptmp[2], save_sec[2];
  unsigned short tab0[NBSTEPS_MAX + 2];	// *NBMOY
  unsigned short nbmoy[2];
#endif
  int inv_p = 0,mes;

  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      hpic[npic] = 0;
      parab[npic] = 0;
      nbmoy[npic] = 0;
    }
  for (npic = 0; npic < (nbpics[antenne] * NBMOY); npic++)
    bsptmp[npic] = 0;

  total = 0;
  bsp = 0;

// ATTENTION, il faut penser a classer les puissances pour aller du plus faible (Pmin) au 
// plus fort (Pmax)
  if (nbpics[antenne] == 2)
    if (puissance[0] > puissance[1])  // modification 130326 bruno
      {
	tmpc = puissance[0];
	puissance[0] = puissance[1];
	puissance[1] = tmpc;
	inv_p = 1;
      }
  // qsort (classe, nbpics[antenne], sizeof (unsigned int), ma_classe);
  do
    {				// reitere NBMOY fois
      total++;
      for (deja_detect = 0; deja_detect < nbpics[antenne]; deja_detect++)
	{			// balaie la bande ISM nbpics[antenne] fois
	  SEC = 0;
	  tmp = 0;

#ifdef dephaseur
          dephase(phases[antenne * NBPICS_MAX + deja_detect],3);
#endif
	  for (freq=fstart[deja_detect]; freq<fstop[deja_detect]; freq+=fstep[deja_detect])
	    { tab0[SEC]=0;
              for (mes=0;mes<NBMES;mes++)
                   tab0[SEC] +=interroge (puissance[deja_detect],freq,offset[deja_detect/2],3)&0xffff;
              tab0[SEC]/=NBMES;
              if (SEC>(NBSTEPS_MAX*NBPICS_MAX)) write_str("\r\nERREUR\r\n\0",NULL,NULL,usart_port);
#ifdef debug2
	      writeDECi (tab0[SEC],NULL,NULL,usart_port);
	      jmf_putchar (' ',NULL,NULL,usart_port);
#endif
   	      SEC++;
	    }
	    save_sec[0] = SEC;

	  SEC = 0;		// la longueur de chaque scan est dans save_sec 
//      for (npic = 0; npic < nbpics[antenne]; npic++)
	  {
	    moyenne[0] = 0;
	    moyenne[1] = 0;
	    for (tmp2 = 0; tmp2 < deja_detect; tmp2++)
	      {
//		tmp = deja_detect_pos[tmp2];
// LARGEUR DOIT ETRE AJUSTE'E AU Q DU RESONATEUR
// 430-450=20 MHz ; 20 MHz/664 points=30 kHz ~ 434/3/10=15 kHz

#define LARGEUR 18
               if ((deja_detect_pos[tmp2]>=LARGEUR) && (deja_detect_pos[tmp2]<=save_sec[0]-LARGEUR))
                  for (tmp=(deja_detect_pos[tmp2]-LARGEUR);tmp<deja_detect_pos[tmp2]+LARGEUR; tmp++) 
//		do
		  {
		   tab0[tmp] = 0;
//		    tmp--;
		  }
//               else jmf_putchar('*');
//		while ((tmp > 0) && (tab0[tmp] > 2 * SEUILMIN)); // couper jusqu'au min(pic)
//		tmp = deja_detect_pos[tmp2];
//		do
//		  {
//		    tab0[tmp] = 0;
//		    tmp++;
//		  }
//		while ((tmp < save_sec[0]) && (tab0[tmp] > 2 * SEUILMIN));

	      }
#ifdef hpf
            filtre_hpf(tab0,save_sec[0]);
#endif
	    bete_max (tab0, save_sec[0], moyenne);	// travaille sur ttes les mesures
	    deja_detect_hpic[deja_detect] = moyenne[1];
	    if (moyenne[0]<LARGEUR) deja_detect_pos[deja_detect] = LARGEUR;
               else {if (moyenne[0]>(save_sec[0]-LARGEUR)) 
                        deja_detect_pos[deja_detect]=(save_sec[0]-LARGEUR);
                     else deja_detect_pos[deja_detect]=moyenne[0];
                    }

//for (npic=0;npic<save_sec[0];npic++)
//{writeDECs(tab0[npic]);jmf_putchar(' ');}
//jmf_putchar('\n');

	    if ((moyenne[1] > SEUILMIN) && (moyenne[1] < SEUILMAX))
	      {
		freqres = (unsigned int) ((fstart[0] + ((int) moyenne[0]) * fstep[0]));
		deja_detect_freq[deja_detect] = ((freqres) + parabole (tab0, moyenne[0], fstep[0]));	// donnees, position du max, pas de freq
	      }
	    else
	      deja_detect_freq[deja_detect] = 0;
	  }
	}			// ici on a detecte' les nbpics[antenne] resonances

      // classe classe[] et ajoute le result a parab[npic]
      if (nbpics[antenne] == 2)
	{
	  if (((deja_detect_freq[1] > 0) && (deja_detect_freq[0] > 0)
	       && (deja_detect_freq[0] > deja_detect_freq[1]))
	      || ((deja_detect_freq[1] > 0) && (deja_detect_freq[0] == 0)))
	    {
	      npic = deja_detect_freq[0]; // inverse freq
	      deja_detect_freq[0] = deja_detect_freq[1];
	      deja_detect_freq[1] = npic;

	      npic = deja_detect_hpic[0]; // inverse hpic
	      deja_detect_hpic[0] = deja_detect_hpic[1];
	      deja_detect_hpic[1] = npic;

	      npic = nbmoy[0];            // inverse nbmoy
	      nbmoy[0] = nbmoy[1];
	      nbmoy[1] = npic;

	      tmpc = puissance[0];        // inverse puissance
	      puissance[0] = puissance[1];
	      puissance[1] = tmpc;

	    }	// a ce stade, j'ai class'e les deja_detect_freq et hpic dans l'ordre croissant, et les 0 a la fin
	  if ((deja_detect_freq[1] == 0) && (deja_detect_freq[0] == 0))
	    if (inv_p == 1)
	      {
		tmpc = puissance[0];
		puissance[0] = puissance[1];
		puissance[1] = tmpc;
	      }
	}
      // qsort (classe, nbpics[antenne], sizeof (unsigned int), ma_classe);
      for (npic = 0; npic < nbpics[antenne]; npic++)
	{
	  if ((deja_detect_hpic[npic] > SEUILMIN) && (deja_detect_hpic[npic] < SEUILMAX))
	    {
	      parab[npic]+=(deja_detect_freq[npic] & 0x1fffffff)>>2;	// finir de replacer/moyenner/variance
	      tab_variance[(bsptmp[npic])+npic*NBMOY_MAX]=(deja_detect_freq[npic] & 0x1fffffff) >> 2;
	      bsptmp[npic]++;
	    }
	  if (deja_detect_hpic[npic] > SEUILMIN)
	    {
	      hpic[npic] += deja_detect_hpic[npic];
	      nbmoy[npic]++;
	    }
	}

      bsp = NBMOY * NBRATES + 1;
      for (npic = 0; npic < nbpics[antenne]; npic++)	// cherche le plus petit bsptmp
	{
	  if (bsp > bsptmp[npic])
	    bsp = bsptmp[npic];
	  // writeDECi(bsp);write_str(" \0");writeDECi(bsptmp[npic]);
	}			//;write_str(" \r\n\0");
    }
  while ((bsp < NBMOY) && (total < (NBMOY * NBRATES)));

  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      if (bsptmp[npic] > 0)
	{
	  somme = 0;		// DEBUT VARIANCE
	  parab[npic] /= bsptmp[npic];
	  for (SEC = 0; SEC < bsptmp[npic]; SEC++)
	    {
	      tmp = ((int) (tab_variance[SEC + NBMOY_MAX * npic] - parab[npic]) >> 8); // 10
	      somme += tmp * tmp;
	    }
	  variance[npic] = (somme / bsptmp[npic]) / bsptmp[npic];        // VARIANCE bsptmp^2
	  parab[npic] = parab[npic] << 2;
#ifndef f400
	  parab[npic] += 0x20000000;
#else
	  parab[npic] += 0x40000000;
#endif
	}
      else
	{
	  variance[npic] = 0;
	  parab[npic] = 0;
	}
      if (nbmoy[npic] > 0) hpic[npic] /= nbmoy[npic];	// moyennes des puissances recues
    }

#ifdef debug2
  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      writeHEXi (parab[npic],NULL,NULL,usart_port);
      jmf_putchar (' ',NULL,NULL,usart_port);
      writeDECi (hpic[npic],NULL,NULL,usart_port);
      jmf_putchar (' ',NULL,NULL,usart_port);
      writeDECi (bsptmp[npic],NULL,NULL,usart_port);
      jmf_putchar (' ',NULL,NULL,usart_port);
      writeDECi (variance[npic],NULL,NULL,usart_port);
      jmf_putchar (' ',NULL,NULL,usart_port);
    }
#endif

  if (bsp == NBMOY)
    return (100 + total);
  else
    return (bsp);
}


int
balayage_primaire0 (unsigned int *fstart, unsigned int *fstop,
		    unsigned int *fstep, int *hpic, unsigned int *parab,
		    unsigned short *puissance, unsigned int *variance,
		    char *balayage_primaire, unsigned int* offset,
		    unsigned char Nb_resonateur, int antenne, int *phases)
{
  int SEC = 0;
  unsigned int npic;
  int moyenne[2], somme;
  unsigned int freq, parabtmp, freqres;
  int bsp=0,total=0;
  unsigned int tab_variance[NBPICS_MAX * (NBMOY_MAX + 1) * NBRATES];
  int tmp = 1, tmp2 = 1,tmp3=0,mes;
#ifdef detec_tous
  unsigned char bsptmp[NBMOY_MAX * NBRATES * NBPICS_MAX];
  unsigned short save_sec[NBPICS_MAX];
  unsigned short tab0[(NBSTEPS_MAX + 2) * NBPICS_MAX];	// *NBMOY
  unsigned short nbmoy[NBPICS_MAX];
#else
  unsigned char bsptmp[2], save_sec[2];
  unsigned short tab0[NBSTEPS_MAX + 2];	// *NBMOY
  unsigned short nbmoy[2];
#endif

//----------------------------------------------------------------------------//
// Cas du mode 3 points, balayage grossier pour determiner les resonances
// Quand elles sont trouvees, balayage_primaire=1; on passe a la seconde partie
// de la fonction. Si on est en mode continu, on saute cette partie de code
//----------------------------------------------------------------------------//

  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      hpic[npic] = 0;
      parab[npic] = 0;
      nbmoy[npic] = 0;
      if (*balayage_primaire == 0)
	tmp2 = 0;		// tmp passe a 0 si un seul 3points n'est pas actif
    }
  for (npic = 0; npic < (nbpics[antenne] * NBMOY); npic++)
    {
      bsptmp[npic] = 0;
    }

  total = 0;
  bsp = 0;
  do
    {
      total++;
      SEC = 0;
      tmp = 0;
      if ((bandes == 1) || ((mode_balayage_continu == 0) && (tmp2 == 1)))	// tous les balayage_primaire est a 1
	{
	  for (npic = 0; npic < nbpics[antenne]; npic++)
	    {tmp3=0;
#ifdef dephaseur
	      dephase(phases[antenne * NBPICS_MAX + npic],3);
#endif

	      for (freq = fstart[npic]; freq < fstop[npic];
		   freq += fstep[npic])
	        { tab0[SEC]=0;
                  for (mes=0;mes<NBMES;mes++)
  		     tab0[SEC] += interroge (puissance[npic],freq,offset[npic/2],3)&0xffff;
                  tab0[SEC]/=NBMES;
// corrige offset jmfxx
//                  if ((mode_balayage_continu==1) && (tmp3==0) && (puissance[npic]==0) 
//                      &&(tab0[SEC]>SEUILMIN)&&(offset[npic/2]<0xF00))
//                     {offset[npic/2]+=0x10;}
//                  if ((mode_balayage_continu==1) && (tmp3==0) && (puissance[npic]>=PUISS_MAX-1) 
//                      && (tab0[SEC]<SEUILMIN)&&(offset[npic/2]>OFFSET))   
//                     {offset[npic/2]-=0x10;} // 140127 : quelle condition pour remonter l'offset ?

		  SEC++;tmp3=1; // on ne change offset QUE sur la mesure du 1er point de la bande
		}
	      save_sec[npic] = SEC - tmp;
	      tmp = SEC;
	    }
	}

      SEC = 0;			// la longueur de chaque scan est dans save_sec 
      for (npic = 0; npic < nbpics[antenne]; npic++)
	{
	  moyenne[0] = 0;
	  moyenne[1] = 0;
	  if ((bandes == 1) || ((mode_balayage_continu == 0) && (tmp2 == 1)))
	    bete_max (&tab0[SEC], save_sec[npic], moyenne);	// travaille par segment 

#ifdef debug2
	  jmf_putchar ('\r',NULL,NULL,usart_port);
	  jmf_putchar ('\n',NULL,NULL,usart_port);
	  jmf_putchar ('%',NULL,NULL,usart_port);
	  writeDECi (save_sec[0],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  writeDECi (moyenne[0],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  writeDECi (moyenne[1],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  jmf_putchar ('\r',NULL,NULL,usart_port);
	  jmf_putchar ('\n',NULL,NULL,usart_port);
#endif
	  if (moyenne[1] > SEUILMIN)
	    {
	      nbmoy[npic]++;
	      hpic[npic] += moyenne[1];
	    }
	  // moyenner les magnitudes au lieu de prendre max
	  //  if (hpic[npic]<moyenne[1]) hpic[npic]=moyenne[1];  
	  if ((moyenne[1] > SEUILMIN) && (moyenne[1] < SEUILMAX)
	      && (bsptmp[npic] < NBMOY))
	    {
	      if ((bandes == 1)
		  || ((mode_balayage_continu == 0) && (tmp2 == 1)))
		{
		  freqres = (unsigned int) ((fstart[npic] + ((int) moyenne[0]) * fstep[npic]));
		  parabtmp = ((freqres) + parabole (&tab0[SEC], moyenne[0], fstep[npic]));	// donnees, position du max, pas de freq
// 110526 : SEC <- SEC*npic
		}
	      parab[npic] += ((parabtmp & 0x1fffffff) >> 2);	// ICI SOMME=DEPASSEMENT
	      tab_variance[(bsptmp[npic])+npic*NBMOY_MAX]=(parabtmp & 0x1fffffff) >> 2;
	      bsptmp[npic]++;
	    }
// on va eliminer le pic qui vient d'etre identifie' et recommencer la recherche
	  SEC += save_sec[npic];	// indice de la prochaine bande
	}			// fin npic

      bsp = NBMOY * NBRATES + 1;
      for (npic = 0; npic < nbpics[antenne]; npic++)	// cherche le plus petit bsptmp
	{
	  if (bsp > bsptmp[npic])
	    bsp = bsptmp[npic];
	  // writeDECi(bsp);write_str(" \0");writeDECi(bsptmp[npic]);
	}			//;write_str(" \r\n\0");
    }
  while ((bsp < NBMOY) && (total < (NBMOY * NBRATES)));

  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      if (bsptmp[npic] > 0)
	{
	  somme = 0;		// DEBUT VARIANCE
	  parab[npic] /= bsptmp[npic];
	  for (SEC = 0; SEC < bsptmp[npic]; SEC++)
	    {
	      tmp = ((int) (tab_variance[SEC + NBMOY_MAX * npic] - parab[npic]) >> 8);	// >> 10
	      somme += tmp * tmp;
	    }
	  variance[npic] = (somme / bsptmp[npic]) / bsptmp[npic];	// VARIANCE bsptmp^2
	  parab[npic] = parab[npic] << 2;
#ifndef f400
	  parab[npic] += 0x20000000;
#else
	  parab[npic] += 0x40000000;
#endif
	}
      else
	{
	  variance[npic] = 0;
	  parab[npic] = 0;
	}
      if (nbmoy[npic] > 0)
	hpic[npic] /= nbmoy[npic];	// moyennes des puissances recues
    }
  if (bsp == NBMOY)
    return (100 + total);
  else
    return (bsp);
}

int
balaie_ism (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep,
	    int *hpic, unsigned int *parab, unsigned short *puissance,
	    unsigned int *variance, char *balayage_primaire, unsigned int *offset,
	    unsigned char Nb_resonateur, int antenne, int *phases)
{
  unsigned int npic;
#ifdef detec_tous
// unsigned int temps_ecoule[NBPICS];           
// unsigned short coef_fenetre[NBPICS];
#else
// unsigned int temps_ecoule[1];                
// unsigned short coef_fenetre[2];
#endif
  int tmp = 0;

#ifdef debug
  antenne=0;
  write_str (" \r\n\0",NULL,NULL,usart_port);
  write_str (" arrive dans Balaie_ism: antenne= \0",NULL,NULL,usart_port);
  writeHEXi (antenne,NULL,NULL,usart_port); 
  write_str(" nbpics[antenne]=\0",NULL,NULL,usart_port);
  writeHEXi (nbpics[0],NULL,NULL,usart_port); write_str ("\r\n\0",NULL,NULL,usart_port);
  for (npic = 0; npic < nbpics[antenne]; npic++)
    {
      write_str ("Npic[\0",NULL,NULL,usart_port);
      writeDEC0i (npic,NULL,NULL,usart_port);
      write_str ("] \0",NULL,NULL,usart_port);
      write_str (" Fstar \0",NULL,NULL,usart_port);
      writeHEXi (fstart[npic],NULL,NULL,usart_port);
      write_str (" Fstop \0",NULL,NULL,usart_port);
      writeHEXi (fstop[npic],NULL,NULL,usart_port);
      write_str (" Fstep \0",NULL,NULL,usart_port);
      writeHEXi (fstep[npic],NULL,NULL,usart_port);
      write_str (" => \0",NULL,NULL,usart_port);
      writeHEXi ((fstop[npic] - fstart[npic]) / fstep[npic],NULL,NULL,usart_port);
      write_str (" puiss \0",NULL,NULL,usart_port);
      writeDEC0i(puissance[npic],NULL,NULL,usart_port);
      write_str ("\r\n\0",NULL,NULL,usart_port);
      write_str (" baly  \0",NULL,NULL,usart_port);
      writeDEC0i(*balayage_primaire,NULL,NULL,usart_port);
      write_str (" Nb_resonateur \0",NULL,NULL,usart_port);
      writeDEC0i(Nb_resonateur,NULL,NULL,usart_port);
      write_str ("\r\n\0",NULL,NULL,usart_port);
#ifdef manuel
      write_str (" Fintsta \0",NULL,NULL,usart_port);
      writeHEXi (Fintsta[npic + Nb_resonateur],NULL,NULL,usart_port);
      write_str (" Fintsto \0",NULL,NULL,usart_port);
      writeHEXi (Fintsto[npic + Nb_resonateur],NULL,NULL,usart_port);
#endif
      write_str ("NBPICS :\0",NULL,NULL,usart_port);
      writeDEC0i (nbpics[antenne],NULL,NULL,usart_port);
      write_str (" \r\n\0",NULL,NULL,usart_port);
    }
#endif
  if (bandes == 0) 
{
    tmp=balayage_bande0(fstart,fstop,fstep,hpic,parab,puissance,variance,
                        balayage_primaire,offset, Nb_resonateur,antenne,phases);
}
  else
    tmp=balayage_primaire0(fstart, fstop, fstep, hpic, parab, puissance,
			   variance, balayage_primaire, offset, Nb_resonateur,
			   antenne, phases);
  if (mode_balayage_continu == 0)	// on est en 3 points
    {if (tmp > 0)
	*balayage_primaire = 1;	// on a trouve' les pics, continue en 3 pts
//           temps_ecoule[0]=(unsigned char)GLOBAL_TIMER1; // temps_ecoule[npic]
     else
	*balayage_primaire = 0;
    }				// passe en mode 3 points
  else
    return (tmp);		// continu => fini
//----------------------------------------------------------------------------//
//      Seconde partie de code
//      Frequence localisee => pas de notion de sous bande ou non (3 points)
//----------------------------------------------------------------------------//
// on n'est ici que si mode_balayage_continu==0 => 3 points
  if (*balayage_primaire == 1)
    {				//jmf_putchar('c');
//   {for (npic=0;npic<NBPICS;npic++) // coef_fenetre[npic]=1;
//     {
      // GLOBAL_TIMER1=0;
      // T1CON=0xC4;  // jmfriedt : voir si on peut pas mettre 44 et reduire 60

//      if (((unsigned char)GLOBAL_TIMER1-temps_ecoule[npic])>60)
//          coef_fenetre[npic]++;
//      if (coef_fenetre[npic]>(NBSTEPS/2)) *balayage_primaire=0;
      for (npic = 0; npic < nbpics[antenne]; npic++)	// recentre la frequence
	{fstart[npic+Nb_resonateur]=(parab[npic+Nb_resonateur])-K*(fstep[npic+Nb_resonateur]);
#ifdef debug
	  writeHEXi (Fintsta[npic + Nb_resonateur],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  writeHEXi (fstart[npic + Nb_resonateur],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  writeHEXi (fstop[npic + Nb_resonateur],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
	  writeHEXi (Fintsto[npic + Nb_resonateur],NULL,NULL,usart_port);
	  jmf_putchar (' ',NULL,NULL,usart_port);
#endif
	  fstop[npic+Nb_resonateur]=(parab[npic+Nb_resonateur])+(K+1)*fstep[npic+Nb_resonateur];	// 2* car on n'atteint jamais la borne
	  if (bandes == 1)
	    {
	      if ((fstart[npic+Nb_resonateur]<Fintsta[npic+Nb_resonateur]) 
                 || (fstop[npic]>Fintsto[npic+Nb_resonateur])) // + Nb_resonateur sur Fintsta et sto
		*balayage_primaire = 0;
	    }
	  else
	    {
	      if ((fstart[npic + Nb_resonateur] < Fintsta[0])
		  || (fstop[npic + Nb_resonateur] >
		      Fintsto[nbpics[antenne] - 1]))
		*balayage_primaire = 0;
	    }
	}
    }
  if (*balayage_primaire == 0)
    for (npic = 0; npic < nbpics[antenne]; npic++)
      {		// coef_fenetre[npic]=1;// on est perdu, on rescanne toute la bande

	if (bandes == 1)
	  { fstart[npic] =Fintsta[npic + Nb_resonateur];
	    fstop[npic] = Fintsto[npic + Nb_resonateur];
	  }
	else
	  { fstart[npic]=Fintsta[0];
	    fstop[npic] = Fintsto[nbpics[antenne] - 1];
	  }
/********************************************************
	  if (bandes == 1)
	    {
#ifdef manuel
	      fstart[npic]=Fintsta[antenne * nbpics[antenne] + npic];
	      fstop[npic]=Fintsto[antenne * nbpics[antenne] + npic];
#else  // non-manuel
	      fstart[antenne*NBPICS_MAX+npic]=FSTART+((FSTOP-FSTART)/nbpics[antenne]) * npic;
	      fstop[antenne*NBPICS_MAX+npic]=fstart[antenne*NBPICS_MAX+npic]+(FSTOP-FSTART)/nbpics[antenne];
	      Fintsta[antenne * nbpics[antenne] + npic] = fstart[antenne * NBPICS_MAX + npic];
	      Fintsto[antenne * nbpics[antenne] + npic] = fstop[antenne * NBPICS_MAX + npic];
#endif
	      fstep[antenne*NBPICS_MAX+npic] =(fstop[antenne*NBPICS_MAX+npic]-fstart[antenne*NBPICS_MAX+npic])/NBSTEPS;
	    }
	  else			// absence de bande, on prend toute la gamme de mesures
	    {
#ifdef manuel
	      fstart[antenne* NBPICS_MAX+npic]=Fintsta[antenne*NBPICS_MAX];
	      fstop[antenne * NBPICS_MAX+npic]=Fintsto[antenne*NBPICS_MAX+nbpics[antenne]-1];
#else
	      fstart[antenne * NBPICS_MAX + npic] =FSTART;
	      fstop[antenne * NBPICS_MAX + npic] = FSTOP;
	      Fintsta[antenne * nbpics[antenne] + npic] = fstart[antenne * NBPICS_MAX + npic];
	      Fintsto[antenne * nbpics[antenne] + npic] = fstop[antenne * NBPICS_MAX + npic];
#endif
	      fstep[antenne*NBPICS_MAX+npic]=((fstop[antenne*NBPICS_MAX+npic]-fstart[antenne*NBPICS_MAX+npic])/NBSTEPS)/nbpics[antenne];
	    }
*********************************************************/


      }
  return (tmp);			// continu => fini
}				//Fin fonction

void change_Fsta (int mon_FSTART)
{
  int tim = 0, ntmp = 0, flag = -1, i;
  short tmp_ant = 1;
  Fintsta[0] = mon_FSTART;
  do
    {
      if (global_index != 0)
	{
	  tim = tim0;
	  for (i = 0; i < global_index; i++)
	    {
	      if ((global_tab[i] == '\r') || (global_tab[i] == '\n')
		  || (global_tab[i] == ' ') || (global_tab[i] == 'q'))
		flag = 999;
	      if ((global_tab[i] > 64) && (flag >= 0))
		ntmp = ntmp * 16 + global_tab[i] - 55;
	      else
		ntmp = ntmp * 16 + global_tab[i] - 48;
	      flag++;
	      if (flag == 8)	// 8 caracteres par antenne
		{
		  flag = 0;
		  Fintsta[tmp_ant] = ntmp;
		  write_str ("F=\0",NULL,NULL,usart_port);
		  writeHEXi (Fintsta[tmp_ant],NULL,NULL,usart_port);
		  write_str ("\r\n\0",NULL,NULL,usart_port);
		  tmp_ant++;
		}
	    }
	  global_index = 0;
	}
    }
  while (((tim0 - tim) < 50) && (flag != 999) && (tmp_ant < ANTENNES));	// 5 secondes
}

void change_Fsto (int mon_FSTOP)
{
  int tim = 0, ntmp = 0, flag = -1, i;
  short tmp_ant = 1;
  Fintsto[0] = mon_FSTOP;
  do
    {
      if (global_index != 0)
	{
	  tim = tim0;
	  for (i = 0; i < global_index; i++)
	    {
	      if ((global_tab[i] == '\r') || (global_tab[i] == '\n')
		  || (global_tab[i] == ' ') || (global_tab[i] == 'q'))
		flag = 999;
	      if ((global_tab[i] > 64) && (flag >= 0))
		ntmp = ntmp * 16 + global_tab[i] - 55;
	      else
		ntmp = ntmp * 16 + global_tab[i] - 48;
	      flag++;
	      if (flag == 8)	// 8 caracteres par antenne
		{
		  flag = 0;
		  Fintsto[tmp_ant] = ntmp;
		  write_str ("G=\0",NULL,NULL,usart_port);
		  writeHEXi (Fintsto[tmp_ant],NULL,NULL,usart_port);
		  write_str ("\r\n\0",NULL,NULL,usart_port);
		  tmp_ant++;
		}
	    }
	  global_index = 0;
	}
    }
  while (((tim0 - tim) < 50) && (flag != 999) && (tmp_ant < ANTENNES));	// 5 secondes
}

#ifdef MODBUS
void maj_AT (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep,
	       unsigned short *puissance, unsigned int *offset,
	       int *nbpic_total)
{int antenne;
 for (antenne = 0; antenne < (ANTENNES*NBPICS/2); antenne++) 
     offset[antenne] = usRegHoldingBuf[NBPICS_MAX*ANTENNES_MAX*(REG_PAR_RESONANCE+1)+antenne];
#ifdef eeprom
 eeprom_erase (0x00);
#endif
#ifdef at25
 at25_erase (0x00);
#endif
 sauve_param ();
}
#endif

void
communication (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep,
	       unsigned short *puissance, unsigned int *offset,
	       int *nbpic_total,char *schneider,int *phases)
{
  int i, flag = 0, antenne, tim = 0;
  unsigned int npic;
  int ntmp = 0;
  int signe = 1;
#ifndef LMD
  write_str ("communication\r\n\0",NULL,NULL,usart_port);
#endif
// T0CON=0xC4; // 2=> core/256, periodic mode, actif
  do
    {
      if (global_index != 0)
	{
	  tim = tim0;
	  // printf("gl=%d : %d %x ",global_index,flag,ntmp);
	  for (i = 0; i < global_index; i++)
	    {			// printf("%c\r\n",global_tab[i]);
	      if ((global_tab[i] == '\r') || (global_tab[i] == '\n')
		  || (global_tab[i] == ' '))
		flag = 0;
	      if ((global_tab[i] == 'A') && (flag == 0))
		{
		  flag++;
		}
	      if ((global_tab[i] == 'T') && (flag == 1))
		{
		  flag++;
		}
	      if (global_tab[i] == 'q')
		{
		  flag = 999;
		}

	      if ((flag >= 10) && (flag < (15)))	// 15 pour commande suivante, mais
		{
		  flag++;	// ensuite 10+ANTENNES => max 4 ANTENNES
		  if ((flag == 11) && (global_tab[i]) == '?')
		    {
		      for (antenne = 0; antenne < ANTENNES; antenne++)
			{
			  writeHEXc ((int) nbpics[antenne],NULL,NULL,usart_port);
			  jmf_putchar (' ',NULL,NULL,usart_port);
			}
		      jmf_putchar ('\r',NULL,NULL,usart_port);
		      jmf_putchar ('\n',NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == (10 + ANTENNES))
			{
			  write_str ("P=\0",NULL,NULL,usart_port);
			  writeHEXi ((int) ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  for (antenne = 0; antenne < ANTENNES; antenne++)
			    {
			      if ((ntmp & 0x000f) <= NBPICS_MAX)
				{
				  nbpics[antenne] = (ntmp & 0x0f);
				  if (NBPICS < nbpics[antenne])
				    NBPICS = nbpics[antenne];
				  for (npic = 0; npic < nbpics[antenne];
				       npic++)
				    {
#ifndef manuel
				      fstart[antenne * NBPICS_MAX + npic] =
					FSTART + ((FSTOP - FSTART) * npic) / nbpics[antenne];
				      // writeHEXi(fstart[antenne*NBPICS_MAX+npic]);jmf_putchar(' ');
				      fstop[antenne * NBPICS_MAX + npic] =
					fstart[antenne * NBPICS_MAX + npic] +
					(FSTOP - FSTART) / nbpics[antenne];
				      // writeHEXi(fstop[antenne*NBPICS_MAX+npic]);jmf_putchar(' ');
				      Fintsta[antenne * nbpics[antenne] + npic] =
					fstart[antenne * NBPICS_MAX + npic];
				      Fintsto[antenne * nbpics[antenne] + npic] =
					fstop[antenne * NBPICS_MAX + npic];
#else
				      fstart[antenne * NBPICS_MAX + npic] =
					Fintsta[antenne * nbpics[antenne] +
						npic];
				      fstop[antenne * NBPICS_MAX + npic] =
					Fintsto[antenne * nbpics[antenne] + npic];
#endif
				      fstep[antenne * NBPICS_MAX + npic] =
					(fstop[antenne * NBPICS_MAX + npic] -
					 fstart[antenne * NBPICS_MAX + npic]) / (NBSTEPS);
				    }
				}
			      ntmp = ntmp >> 4;
			    }
			  *nbpic_total = 0;
			  for (antenne = 0; antenne < ANTENNES; antenne++)
			    *nbpic_total += nbpics[antenne];
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}

	      if ((flag >= 15) && (flag < 20))
		{
		  flag++;
		  if ((flag == 16) && (global_tab[i]) == '?')
		    {
		      writeDEC0i (NY,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      // printf("%x\r\n",NY);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 17)
			{
			  write_str ("Y=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  // printf("Y=%d\r\n",ntmp);
			  NY = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}

	      if ((flag >= 20) && (flag < 25))
		{
		  flag++;
		  if ((flag == 21) && (global_tab[i]) == '?')
		    {
		      writeHEXs (NBMOY,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      // printf("%x\r\n",NBMOY);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 23)
			{
			  write_str ("M=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  // printf("M=%d\r\n",ntmp);
			  NBMOY = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}

	      if ((flag >= 30) && (flag < 40))
		{
		  flag++;
		  if ((flag == 31) && (global_tab[i]) == '?')
		    {
#ifndef manuel
		      writeHEXi (FSTART,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
#else
		      writeHEXi (fstart[0],NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
#endif
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 38)
			{
			  write_str ("F=\0",NULL,NULL,usart_port);
			  writeHEXi (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  // printf("F=%x\r\n",ntmp);
#ifdef manuel
			  change_Fsta (ntmp);
#else
			  FSTART = ntmp;
#endif
			  for (antenne = 0; antenne < ANTENNES; antenne++)
			    for (npic = 0; npic < nbpics[antenne]; npic++)
			      {
#ifndef manuel
				fstart[antenne * NBPICS_MAX + npic] =
				  FSTART +
				  ((FSTOP - FSTART) * npic) / nbpics[antenne];
				fstop[antenne * NBPICS_MAX + npic] =
				  fstart[antenne * NBPICS_MAX + npic] +
				  (FSTOP - FSTART) / nbpics[antenne];
				Fintsta[antenne * nbpics[antenne] + npic] =
				  fstart[antenne * NBPICS_MAX + npic];
				Fintsto[antenne * nbpics[antenne] + npic] =
				  fstop[antenne * NBPICS_MAX + npic];
#else
				fstart[antenne * NBPICS_MAX + npic] =
				  Fintsta[antenne * nbpics[antenne] + npic];
#endif
			      }
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 40) && (flag < 50))
		{
		  flag++;
		  if ((flag == 41) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",FSTOP);
		      ntmp = 0;
		      flag = 0;
#ifndef manuel
		      writeHEXi (FSTOP,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
#else
		      writeHEXi (fstop[0],NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
#endif
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 48)
			{	// printf("G=%x\r\n",ntmp);
			  write_str ("G=\0",NULL,NULL,usart_port);
			  writeHEXi (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
#ifdef manuel
			  change_Fsto (ntmp);
#else
			  FSTOP = ntmp;
#endif
			  for (antenne = 0; antenne < ANTENNES; antenne++)
			    for (npic = 0; npic < nbpics[antenne]; npic++)
			      {
#ifndef manuel
				fstart[antenne + NBPICS_MAX * antenne +
				       npic] =
				  FSTART +
				  ((FSTOP - FSTART) * npic) / nbpics[antenne];
				fstop[antenne + NBPICS_MAX * antenne + npic] =
				  fstart[antenne * NBPICS_MAX + npic] +
				  (FSTOP - FSTART) / nbpics[antenne];
				Fintsta[antenne * nbpics[antenne] + npic] =
				  fstart[antenne * NBPICS_MAX + npic];
				Fintsto[antenne * nbpics[antenne] + npic] =
				  fstop[antenne * NBPICS_MAX + npic];
#else
				fstop[antenne * NBPICS_MAX + npic] =
				  Fintsto[antenne * nbpics[antenne] + npic];
#endif
			      }
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 50) && (flag < 55))
		{
		  flag++;
		  if ((flag == 51) && (global_tab[i]) == '?')
		    {
		      writeHEXs (LBT,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 51)
			{
			  write_str ("L=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  LBT = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 55) && (flag < 60))
		{
		  flag++;
		  if ((flag == 56) && (global_tab[i]) == '?')
		    {
		      write_str ("N=\0",NULL,NULL,usart_port);
		      writeDEC0i (NBSTEPS,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      // printf("%x\r\n",NBSTEPS);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 58)
			{
			  // printf("N=%d\r\n",ntmp);
			  write_str ("N=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  if (ntmp <= NBSTEPS_MAX)
			    {
			      NBSTEPS = ntmp;
			      for (antenne = 0; antenne < ANTENNES; antenne++)
				for (npic = 0; npic < nbpics[antenne]; npic++)
				  fstep[antenne * NBPICS_MAX + npic] =
				    (fstop[antenne * NBPICS_MAX + npic] -
				     fstart[antenne * NBPICS_MAX +
					    npic]) / NBSTEPS;
			    }
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 60) && (flag < 65))
		{
		  flag++;
		  if ((flag == 61) && (global_tab[i]) == '?')
		    {
		      for (antenne = 0; antenne < ANTENNES; antenne++)
			{
			  writeHEXi (offset[antenne*NBPICS_MAX/2],NULL,NULL,usart_port);
			  jmf_putchar (' ',NULL,NULL,usart_port);
			}
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      // printf("%x\r\n",*offset);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 63)
			{	// printf("O=%d\r\n",ntmp);
			  write_str ("O=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
#ifdef auto_offset
			  if (ntmp == 0)
			    corrige_offset (offset);
			  else
#endif
			    {
			      for (antenne = 0; antenne < (ANTENNES_MAX*NBPICS_MAX/2); antenne++)
				offset[antenne] = ntmp;
			    }
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 65) && (flag < 70))
		{
		  flag++;
		  if ((flag == 66) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",HATTENTE);
		      writeHEXi (HATTENTE,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 67)
			{	// printf("H=%d\r\n",ntmp);
			  write_str ("H=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  HATTENTE = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 70) && (flag < 75))
		{
		  flag++;
		  if ((flag == 71) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",ATTENTE);
		      writeHEXi (ATTENTE,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 74)
			{	// printf("D=%d\r\n",ntmp);
			  write_str ("D=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ATTENTE = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 75) && (flag < 80))
		{
		  flag++;
		  if ((flag == 76) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",SEUILMAX);
		      writeHEXi (SEUILMAX,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 78)
			{	// printf("B=%d\r\n",ntmp);
			  write_str ("B=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  SEUILMAX = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}

	      if ((flag >= 95) && (flag < 99))
		{
		  flag++;
		  if ((flag == 96) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",SEUILMIN);
		      writeHEXi (SEUILMIN,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 98)
			{	// printf("C=%d\r\n",ntmp);
			  write_str ("C=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  SEUILMIN = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 80) && (flag < 85))
		{
		  flag++;
		  if ((flag == 81) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",EMET);
		      ntmp = 0;
		      flag = 0;
		      writeHEXi (EMET,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 84)
			{	// printf("E=%d\r\n",ntmp);
			  write_str ("E=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  EMET = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
	      if ((flag >= 85) && (flag < 90))
		{
		  flag++;
		  if ((flag == 86) && (global_tab[i]) == '?')
		    {
		      for (antenne = 0; antenne < ANTENNES*NBPICS_MAX; antenne++)
			{
			  writeHEXi (puissance[antenne],NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			}
		      ntmp = 0;
		      flag = 0;
		    }
		  else		// ((flag==86)&&(global_tab[i]!='?'))
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 87)
			{
			  write_str ("Q=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  if ((ntmp >= 0) && (ntmp <= PUISS_MAX))
			    {
			      for (antenne = 0; antenne < ANTENNES*NBPICS_MAX; antenne++)
			  	  puissance[antenne] = ntmp;
			    }
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
/*
	   if ((flag>=90)&&(flag<95)) 
	       {flag++;
	        if ((flag==91) && (global_tab[i])=='?') 
		   {printf("%x\r\n",agc);ntmp=0;flag=0;}
	        else //  ((flag==91)&&(global_tab[i]!='?')) 
		   {if (global_tab[i]>64) ntmp=ntmp*16+global_tab[i]-55;
	               else ntmp=ntmp*16+global_tab[i]-48;
		    printf("A=%d\r\n",ntmp);
                    if ((ntmp>=0) && (ntmp <2)) agc=ntmp;
		    ntmp=0;flag=200;
		    
		    
		   }
	       }
*/
	      if ((flag >= 25) && (flag < 30))
		{
		  flag++;
		  if ((flag == 26) && (global_tab[i]) == '?')
		    {		// printf("%x\r\n",ANTENNES);
		      writeHEXi (ANTENNES,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else		//  ((flag==91)&&(global_tab[i]!='?')) 
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      // printf("X=%d\r\n",ntmp);
		      write_str ("X=\0",NULL,NULL,usart_port);
		      writeHEXi (ntmp,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      if ((ntmp >= 0) && (ntmp < 9))
			if (ntmp <= ANTENNES_MAX)
			  ANTENNES = ntmp;
		      *nbpic_total = 0;
		      for (antenne = 0; antenne < ANTENNES; antenne++)
			*nbpic_total += nbpics[antenne];
		      ntmp = 0;
		      flag = 200;
		    }
		}

	      if ((flag >= 100) && (flag < 105))
		{
		  flag++;
		  if (global_tab[i] > 64)
		    ntmp = ntmp * 16 + global_tab[i] - 55;
		  else
		    ntmp = ntmp * 16 + global_tab[i] - 48;
		  if (flag == 103)
		    {
		      write_str ("W=\0",NULL,NULL,usart_port);
		      writeDEC0i (ntmp,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      if (ntmp == 0x000)
			{
			  lit_param ();
			  *nbpic_total = 0;
			  for (antenne = 0; antenne < ANTENNES; antenne++)
			    *nbpic_total += nbpics[antenne];
			}
		      if (ntmp == 0x666)
                        {
#ifdef eeprom
			 eeprom_erase (0x00);
#endif
#ifdef at25
			 at25_erase (0x00);
#endif
                        }
		      if (ntmp == 0xFFF)
			sauve_param ();
		      ntmp = 0;
		      flag = 200;
		    }
		}

	      if ((flag >= 105) && (flag < 107))	// commande 3 points
		{
		  flag++;
		  if (flag == 106)
		    {
		      if (global_tab[i] == '?')
			{
			  writeHEXc (mode_balayage_continu,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		      else
			{
			  if (global_tab[i] > 64)
			    ntmp = global_tab[i] - 55;
			  else
			    ntmp = global_tab[i] - 48;
			  mode_balayage_continu = ntmp;
			  write_str ("3=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}

	      if ((flag >= 118) && (flag < 120))	// commande affiche_temperature
		{
		  flag++;
		  if (flag == 119)
		    {
		      if (global_tab[i] == '?')
			{
			  writeHEXc (convertit_temperature,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		      else
			{
			  if (global_tab[i] > 64)
			    ntmp = global_tab[i] - 55;
			  else
			    ntmp = global_tab[i] - 48;
			  convertit_temperature = ntmp;
			  write_str ("convert_temperature=\0",NULL,NULL,usart_port);
			  writeDEC0i (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}


	      if ((flag >= 110) && (flag < 117))	// RS232 speed ATs
		{
		  flag++;
		  if (flag == 111)
		    {
		      if (global_tab[i] == '?')
			{
			  writeDECi (RS_BAUDRATE,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
                      else ntmp = global_tab[i] - 48;
		    }
		  else
		    {//if (global_tab[i] > 64)
		     //   ntmp = ntmp * 16 + global_tab[i] - 55;
		     //else
		     ntmp = ntmp * 10 + global_tab[i] - 48;
		     if (flag == 116)
		     {
                      if ((ntmp==9600)||(ntmp==19200)||(ntmp==38400)||(ntmp==57600)||(ntmp==115200)) 
                         {RS_BAUDRATE=ntmp;set_serial_speed(RS_BAUDRATE,RS_PARITY);
	  	          write_str ("ATs=\0",NULL,NULL,usart_port);
		          writeDECi (ntmp,NULL,NULL,usart_port);
		          write_str ("\r\n\0",NULL,NULL,usart_port);
                         }
                      else 
                         {writeDECi (ntmp,NULL,NULL,usart_port);
                          write_str (" not supported\r\n\0",NULL,NULL,usart_port);
                         }
		      ntmp = 0;
		      flag = 200;
                     }
		    }
               }

	      if ((flag >= 175) && (flag < 177))	// RS232 parity ATp
		{
		  flag++;
		  if (flag == 176)
		    {
		      if (global_tab[i] == '?')
			{
			  writeDECi (RS_PARITY,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
                      else 
                        {ntmp = global_tab[i] - 48;
                         if ((ntmp==0)||(ntmp==1)||(ntmp==2)) 
                         {RS_PARITY=ntmp;set_serial_speed(RS_BAUDRATE,RS_PARITY);
	  	          write_str ("ATp=\0",NULL,NULL,usart_port);
		          writeDECi (ntmp,NULL,NULL,usart_port);
		          write_str ("\r\n\0",NULL,NULL,usart_port);
                         }
                      else 
                         {writeDECi (ntmp,NULL,NULL,usart_port);
                          write_str (" not supported\r\n\0",NULL,NULL,usart_port);
                         }
		      ntmp = 0;
		      flag = 200;
                     }
		    }
               }


	      if ((flag >= 150) && (flag < 154))	// ATg = proportional constant for 2-point
		{
		  flag++;
		  if ((flag == 151) && (global_tab[i]) == '?')
		    {
		      writeDECi (proportional,NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if (global_tab[i] > 64)
			ntmp = ntmp * 16 + global_tab[i] - 55;
		      else
			ntmp = ntmp * 16 + global_tab[i] - 48;
		      if (flag == 153)
			{	// printf("C=%d\r\n",ntmp);
			  write_str ("g=\0",NULL,NULL,usart_port);
			  writeDECi (ntmp,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  proportional = ntmp;
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
//#ifdef convertit_temperature
	      if ((flag >= 120) && (flag < 127))	// commande A0 : 5 chars
		{
		  flag++;
		  if (flag == 121)
		    {
		      if (global_tab[i] == '?')
			{
			  writeDECi (A0[0],NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		      if (global_tab[i] == '-')
			signe = -1;
		    }
		  if ((global_tab[i] > 47) && (global_tab[i] < 58))
		    ntmp = ntmp * 10 + global_tab[i] - 48;
		  if (flag == 126)
		    {
		      A0[0] = signe * ntmp;
		      write_str ("A0=\0",NULL,NULL,usart_port);
		      writeDECi (A0[0],NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      signe = 1;
		      ntmp = 0;
		      flag = 200;
		    }
		}

	      if ((flag >= 130) && (flag < 141))	// commande A1 : 10 chars
		{
		  flag++;
		  if (flag == 131)
		    {
		      if (global_tab[i] == '?')
			{
			  writeDECi (A1[0],NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		      if (global_tab[i] == '-')
			signe = -1;
		    }

		  if ((global_tab[i] > 47) && (global_tab[i] < 58))
		    ntmp = ntmp * 10 + global_tab[i] - 48;
		  if (flag == 140)
		    {
		      A1[0] = signe * ntmp;
		      write_str ("A1=\0",NULL,NULL,usart_port);
		      writeDECi (A1[0],NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      signe = 1;
		      ntmp = 0;
		      flag = 200;
		    }
		}

	      if ((flag >= 142) && (flag < 149))	// commande A2 : 6 chars
		{
		  flag++;
		  if (flag == 143)
		    {
		      if (global_tab[i] == '?')
			{
			  writeDECi (A2[0],NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		      if (global_tab[i] == '-')
			signe = -1;
		    }
		  if ((global_tab[i] > 47) && (global_tab[i] < 58))
		    ntmp = ntmp * 10 + global_tab[i] - 48;
		  if (flag == 148)
		    {
		      A2[0] = signe * ntmp;
		      write_str ("A2=\0",NULL,NULL,usart_port);
		      writeDECi (A2[0],NULL,NULL,usart_port);
		      write_str ("\r\n\0",NULL,NULL,usart_port);
		      signe = 1;
		      ntmp = 0;
		      flag = 200;
		    }
		}
//#endif
#ifdef MODBUS
	      if ((flag >= 170) && (flag < 173))	// MODBUS slave @
		{
		  flag++;
		  if (flag == 171)
		    {
		      if (global_tab[i] == '?')
			{
			  writeHEXi (ucMBAddress,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 0;
			}
		    }
		  if (global_tab[i] > 64)
		    ntmp = ntmp * 16 + global_tab[i] - 55;
		  else
		    ntmp = ntmp * 16 + global_tab[i] - 48;
		  if (flag == 172)
		    {
		      write_str ("ATm=",NULL,NULL,usart_port);
		      writeHEXi (ntmp,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ucMBAddress_tmp=(ntmp);
		      ntmp = 0;
		      flag = 200;	
		    }
		}
#endif

#ifdef RTC0
	      if ((flag >= 160) && (flag < 167))	// heure
		{
		  flag++;
		  if ((flag == 161) && (global_tab[i] == '?'))
		    {
		      writeDECheure (heure,NULL,NULL,usart_port);
		      writeDECheure (minute,NULL,NULL,usart_port);
		      writeDECheure (seconde / 10,NULL,NULL,usart_port);	// seconde en 1/10 seconde
		      write_str ("\r\n",usart_port);
		      ntmp = 0;
		      flag = 0;
		    }
		  else
		    {
		      if ((global_tab[i] > 47) && (global_tab[i] < 58))
			ntmp = ntmp * 10 + global_tab[i] - 48;
		      if (flag == 166)
			{
			  heure = ntmp / 10000;
			  ntmp = ntmp - heure * 10000;
			  if (heure > 24)
			    heure = 0;
			  minute = ntmp / 100;
			  ntmp = ntmp - minute * 100;
			  if (minute > 60)
			    minute = 0;
			  seconde = ntmp * 10;
			  if (seconde > 600)
			    seconde = 0;
			  writeDECheure (heure,NULL,NULL,usart_port);
			  writeDECheure (minute,NULL,NULL,usart_port);
			  writeDECheure (seconde / 10,NULL,NULL,usart_port);
			  write_str ("\r\n",NULL,NULL,usart_port);
			  ntmp = 0;
			  flag = 200;
			}
		    }
		}
#endif

	      if (flag == 2)
		{		// DOIT etre a la fin sinon conflit avec avant
		  if (global_tab[i] == 'P')
		    {
		      write_str ("ATP OK\r\n",NULL,NULL,usart_port);
		      flag = 10;
		    }
		  if (global_tab[i] == 'Y')
		    {
		      write_str ("ATY OK\r\n",NULL,NULL,usart_port);
		      flag = 15;
		    }
		  if (global_tab[i] == 'M')
		    {
		      write_str ("ATM OK\r\n",NULL,NULL,usart_port);
		      flag = 20;
		    }
		  if (global_tab[i] == 'X')
		    {
		      write_str ("ATX OK\r\n",NULL,NULL,usart_port);
		      flag = 25;
		    }
		  if (global_tab[i] == 'F')
		    {
		      write_str ("ATF OK\r\n",NULL,NULL,usart_port);
		      flag = 30;
		    }
		  if (global_tab[i] == 'G')
		    {
		      write_str ("ATG OK\r\n",NULL,NULL,usart_port);
		      flag = 40;
		    }
		  if (global_tab[i] == 'L')
		    {
		      write_str ("ATL OK\r\n",NULL,NULL,usart_port);
		      flag = 50;
		    }
		  if (global_tab[i] == 'N')
		    {
		      write_str ("ATN OK\r\n",NULL,NULL,usart_port);
		      flag = 55;
		    }
		  if (global_tab[i] == 'O')
		    {
		      write_str ("ATO OK\r\n",NULL,NULL,usart_port);
		      flag = 60;
		    }
		  if (global_tab[i] == 'H')
		    {
		      write_str ("ATH OK\r\n",NULL,NULL,usart_port);
		      flag = 65;
		    }
		  if (global_tab[i] == 'D')
		    {
		      write_str ("ATD OK\r\n",NULL,NULL,usart_port);
		      flag = 70;
		    }
		  if (global_tab[i] == 'B')
		    {
		      write_str ("ATB OK\r\n",NULL,NULL,usart_port);
		      flag = 75;
		    }
		  if (global_tab[i] == 'E')
		    {
		      write_str ("ATE OK\r\n",NULL,NULL,usart_port);
		      flag = 80;
		    }
		  if (global_tab[i] == 'Q')
		    {
		      write_str ("ATQ OK\r\n",NULL,NULL,usart_port);
		      flag = 85;
		    }
		  if (global_tab[i] == 'C')
		    {
		      write_str ("ATC OK\r\n",NULL,NULL,usart_port);
		      flag = 95;
		    }
		  if (global_tab[i] == 'W')
		    {
		      write_str ("ATW OK\r\n",NULL,NULL,usart_port);
		      flag = 100;
		    }
		  if (global_tab[i] == 's')
		    {
		      write_str ("ATs OK\r\n",NULL,NULL,usart_port);
		      flag = 110;
		    }
		  if (global_tab[i] == 'g')
		    {
		      write_str ("ATg OK\r\n",NULL,NULL,usart_port);
		      flag = 150;
		    }
		  if (global_tab[i] == 'h')
		    {
		      write_str ("ATh OK\r\n",NULL,NULL,usart_port);
		      flag = 160;
		    }
		  if (global_tab[i] == 'm')
		    {
		      write_str ("ATm OK\r\n",NULL,NULL,usart_port);
		      flag = 170;
		    }
		  if (global_tab[i] == 'p')
		    {
		      write_str ("ATp OK\r\n",NULL,NULL,usart_port);
		      flag = 175;
		    }
		  if (convertit_temperature == 1)
		    {
//#ifdef convertit_temperature
		      if (global_tab[i] == '0')
			{
			  write_str ("AT0 OK\r\n",NULL,NULL,usart_port);
			  flag = 120;
			}
		      if (global_tab[i] == '1')
			{
			  write_str ("AT1 OK\r\n",NULL,NULL,usart_port);
			  flag = 130;
			}
		      if (global_tab[i] == '2')
			{
			  write_str ("AT2 OK\r\n",NULL,NULL,usart_port);
			  flag = 142;
			}
		    }
//#endif
		  if (global_tab[i] == '3')
		    {		//mode_balayage_continu=1-mode_balayage_continu;
		      //write_str("3POINTS=");writeDEC0i(mode_balayage_continu);
		      //write_str("\r\n");
		      //ntmp=0;flag=200;
		      write_str ("AT3 OK\r\n",NULL,NULL,usart_port);
		      flag = 105;
		    }
		  if (global_tab[i] == 'o')
		    {
		      write_str ("ATo OK\r\n",NULL,NULL,usart_port);
		      flag = 118;
		    }
		  if (global_tab[i] == '=')
		    {
		      bandes = 1 - bandes;
		      write_str ("BANDES=",NULL,NULL,usart_port);
		      writeDEC0i (bandes,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }
		  if (global_tab[i] == '!')
		    {
		      deux_points = 1 - deux_points;
		      write_str ("2point=",NULL,NULL,usart_port);
		      writeDEC0i (deux_points,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }
		  if (global_tab[i] == 'S')
		    {
		      *schneider= 1 - *schneider;
		      write_str ("S=",NULL,NULL,usart_port);
		      writeDECc (*schneider,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }
		  if (global_tab[i] == 'Z')
		    {
		      agc = 1 - agc;
		      write_str ("AGC=",NULL,NULL,usart_port);
		      writeDEC0i (agc,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }

		  if (global_tab[i] == 'e')
		    {
		      trame_minimale = 1 - trame_minimale;
		      write_str ("ATe=",NULL,NULL,usart_port);
		      writeDEC0i (trame_minimale,NULL,NULL,usart_port);
		      write_str ("\r\n",NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }
		  if (global_tab[i] == 'b')
		    {
		      oscilloscope (fstart, fstop, fstep, puissance,offset,phases);
		      ntmp = 0;
		      flag = 200;
		    }
		  if (global_tab[i] == 't')
		    {
		      writeDECs (temperature (),NULL,NULL,usart_port);
		      jmf_putchar ('\r',NULL,NULL,usart_port);
		      jmf_putchar ('\n',NULL,NULL,usart_port);
		      ntmp = 0;
		      flag = 200;
		    }
		}
	      if (flag == 200)
		flag = 2;	// 200 etait valeur tmp pour AT0
	    }
	  global_index = 0;
	}
    kick_watchdog();
      // } while (flag!=99); // retour chariot
    }
  while (((tim0 - tim) < 50) && (flag != 999));	// 5 secondes
  // printf("DONE %d\r\n",tim0);
#ifndef LMD
  write_str ("DONE ",NULL,NULL,usart_port);
  writeDEC0i (tim0,NULL,NULL,usart_port);
  write_str ("\r\n",NULL,NULL,usart_port);
#endif
// T0CON=0x44; // 4=> core/256, periodic mode, inactif
}

int
main (void)
{
  char affichage_2pts = 1;
#ifdef Mode_FM_2p_et_senseor
  unsigned short premiere_foi;
#endif
  unsigned short localisation = 0, asservissement_phase = 0;
  unsigned int recu = 0, plus = 0, antenne, decallage = 0;
  unsigned int npic;
  long tempracine=0, tempdefault=-400;
  int nbpic_total;
#ifdef LMD
  char affiche_lmd = 0;
#endif
  unsigned short puissance[ANTENNES_MAX * NBPICS_MAX]; 
  unsigned char balaie[NBPICS_MAX], compteur = 0, Nb_resonateur = 0;	// puissance = 0x0F
  // char chaine[(ANTENNES_MAX * NBPICS_MAX)*44+4+12+40]; // +40 au cas ou` ... pour options additionnelles d'affichage
  char chaine[400]; // le mode 2 points necessite une enorme chaine (300 ne passe pas) ! 150716
  int chaine_index=0;
  unsigned int parab_conti[NBPICS_MAX*ANTENNES_MAX],dac2[ANTENNES_MAX*NBPICS_MAX/2],variance[ANTENNES_MAX*NBPICS_MAX+1];
  unsigned int fstart[ANTENNES_MAX*NBPICS_MAX+1],fstop[ANTENNES_MAX * NBPICS_MAX+1],fstep[ANTENNES_MAX*NBPICS_MAX*2];
  char balayage_primaire[ANTENNES_MAX];	// 1 par antenne, PAS par pic
  int hpic[NBPICS_MAX * ANTENNES_MAX * 2];
#ifdef at25
  unsigned char memoire[20];
#endif
  int phases[NBPICS_MAX * ANTENNES_MAX];	// pour TGCP
// variables pour 2points uniquement
// unsigned int parab[NBPICS_MAX * ANTENNES_MAX];
  unsigned int parab1[NBPICS_MAX * ANTENNES_MAX],parab2[NBPICS_MAX * ANTENNES_MAX];
  unsigned int frequence_asservissement = 0;
  char schneider=0;
  unsigned char nb_pic_trouve = 0, choix_emission[NBPICS_MAX * ANTENNES_MAX];
  unsigned short diff_de_phase[2 * NBPICS_MAX * ANTENNES_MAX];	// methode 1 point doit memoriser l'autre etat
#ifdef dephaseur
   int cpt_phase[NBPICS_MAX/2*ANTENNES_MAX];
#endif
#ifdef MODBUS
  int *i_tmp,temporisation_modbus,temporisation_modbus_cst=1;
  float f_tmp;
//  modbus_t ctx;
//  extern modbus_mapping_t mb_mapping;
//  uint8_t query[5]; // MODBUS_TCP_MAX_ADU_LENGTH];
//  int rc;
   eMBErrorCode    eStatus;
//   UCHAR cmd_modbus[8]={0xff,0x06,0x00,0x0e,0x01,0x01,0x00,0x00}; // dummy 06 @hi @lo valhi valo
//   int temperature_current[ANTENNES_MAX];			  // temperature par capteur / antenne (max 4 pour DAT3024)
//   int modbus_alternate = 0;					  // pour alterner le rafraichissement des registres du DAT3024 a chaque balayage
//   int i;

//   for (i = 0; i < ANTENNES_MAX; i++)   // cas du DATEXEL avec lecteur senseor en client MODBUS
//	temperature_current[i] = 0;
   rs_tmp=RS_BAUDRATE;
   rs_tmp2=RS_PARITY;
#endif

  init_CPU ();
#ifdef LCD
  char s[20];
#endif

//////////////////////// stress test pour malloc et floating
// attention dans le cas de l'ADuC : reboot a cause du WDT
/*
 unsigned short tempe=0x1234; 
 const char welcome[] = "\r\nHello from a STM32 \0" ;

 volatile double mf=1.,lf=2.;
 volatile int mi=1,li=2;
 char *t;

  jmf_putchar('+',NULL,NULL,usart_port);
 for (tempe=1;tempe<10;tempe++) {mf=mf*lf;}
  writeHEXi((int)mf,NULL,NULL,usart_port);
  jmf_putchar('+',NULL,NULL,usart_port);
  for (tempe=1;tempe<10;tempe++) {mi=mi*li;}
  writeHEXi((int)mi,NULL,NULL,usart_port);
  jmf_putchar('+',NULL,NULL,usart_port);
  t=(char*)malloc(20);
  
  npic=0;
  while (1)
  {
   write_str(welcome,NULL,NULL,usart_port);
   if (npic<10) jmf_putchar(npic+'0',NULL,NULL,usart_port); else jmf_putchar(npic+'A'-10,NULL,NULL,usart_port);
   npic++;if (npic==16) npic=0;
   jmf_putchar(' ',NULL,NULL,usart_port);
   writeHEXi(tempe,NULL,NULL,usart_port);
   delay(10000);
   jmf_putchar(' ',NULL,NULL,usart_port);

   memset(t,0x55,20);
   jmf_putchar('^',NULL,NULL,usart_port);
   writeHEXc((int)t[0],NULL,NULL,usart_port);
   jmf_putchar('^',NULL,NULL,usart_port);
   writeHEXc((int)t[19],NULL,NULL,usart_port);
  }
*/
////////////////////////
  
#ifdef at25
// npic=sauve_param(); // writeDECi(npic);jmf_putchar(' ');
  at25_id (memoire);
  if ((memoire[0] == 0x1f) && memoire[1] == 0x43)
    {
      write_str ("FLASH found\r\n",NULL,NULL,usart_port);
      npic = lit_param ();
      if (npic > 10)
	{write_str ("configuration recovered\r\n",NULL,NULL,usart_port);
         set_serial_speed(RS_BAUDRATE,RS_PARITY);
        }
    }
// at25_read(0x00,memoire,10);
// at25_erase(0x00);
// at25_write(0x00,memoire,10);
#endif

#ifdef eeprom
  eeprom_init ();
  write_str ("reading EEPROM\r\n",NULL,NULL,usart_port);
  npic = lit_param ();
  write_str ("reading param\r\n",NULL,NULL,usart_port);
  if (npic > 10)
    {write_str ("configuration recovered\r\n",NULL,NULL,usart_port);
     set_serial_speed(RS_BAUDRATE,RS_PARITY);
    }
#endif

/* TEST EEPROM 
while(1) {
 write_str("erase ",NULL,NULL,usart_port); eeprom_erase(0x00);
 for (npic=0;npic<6;npic++) memoire[npic]=npic; 
 memoire[6]=0x55; memoire[7]=0xaa; memoire[8]=1; memoire[9]=0x55; memoire[10]=0xaa;
 write_str("write ",NULL,NULL,usart_port);
 eeprom_write(0x00,memoire,10);
 
 for (npic=0;npic<10;npic++) memoire[npic]=0; 
 write_str("read ",NULL,NULL,usart_port);
 eeprom_read(0x00,memoire,10);
 for (npic=0;npic<10;npic++) 
    {writeDECc(memoire[npic],NULL,NULL,usart_port);jmf_putchar(' ',NULL,NULL,usart_port);}
 write_str("\r\n",NULL,NULL,usart_port);
 for (npic=0;npic<10;npic++) delay(0xffff);
}
*/

// init PLL
#ifdef debug
  write_str("envoi_PLL()\r\n",NULL,NULL,usart_port); // ATTENTION : doit se faire en SPI lent sur ADuC sinon
#endif                          // ne se configure pas (SPIDIV=6, *PAS* 3: 130118)
  init_PLL();
#ifdef debug
  write_str("envoi_DDS()\r\n",NULL,NULL,usart_port);
#endif
  init_DDS();

  // Configuration lcd
  //LCD_init();                           // [RRRGGGBB]
  //GP2DAT &= ~LCD_SCK;

  //LCDSetPixel(30, 30, YELLOW);
  //LCDPutChar('E', 30, 30, MEDIUM, BLACK, YELLOW);
#ifdef debug
  write_str("antenne\r\n",NULL,NULL,usart_port);
#endif

  if (NBMOY>NBMOY_MAX)    
    while (1) {write_str("NBMOY trop grand\r\n",NULL,NULL,usart_port);delay(0xffff);}
  if (NBPICS>NBPICS_MAX)    
    while (1) {write_str("NBPICS trop grand\r\n",NULL,NULL,usart_port);delay(0xffff);}
  if (NBMES>NBMES_MAX)      
    while (1) {write_str("NBMES trop grand\r\n",NULL,NULL,usart_port);delay(0xffff);}
  if (NBSTEPS>NBSTEPS_MAX)  
    while (1) {write_str("NBSTEPS trop grand\r\n",NULL,NULL,usart_port);delay(0xffff);}
  if (ANTENNES>ANTENNES_MAX) 
    while (1) {write_str("ANTENNES trop grand\r\n",NULL,NULL,usart_port);delay(0xffff);}

  for (antenne = 0; antenne < ANTENNES_MAX * NBPICS_MAX; antenne++)
    {
#ifndef gilloux
      puissance[antenne] =  0; // PUISS_MAX; // commence a la puissance min 24/02/2014
#else
      puissance[antenne] =  PUISS_MAX;
#endif
      phases[antenne] = 0;
#ifdef dephaseur
      cpt_phase[antenne/2]=0;
#endif
    }
  for (antenne = 0; antenne < ANTENNES_MAX; antenne++)
    {
      nbpics[antenne] = NBPICS;
    }

// essai jmf avec 1=temperature et 2=pression
// nbpics[0]=2; nbpics[1]=3;

#ifdef debug
  write_str ("Init vars ",NULL,NULL,usart_port);
  writeHEXi(ANTENNES,NULL,NULL,usart_port);
  write_str (" ",NULL,NULL,usart_port);
  writeHEXi(nbpics[ANTENNES-1],NULL,NULL,usart_port);
  write_str ("\r\n",NULL,NULL,usart_port);
#endif

  for (antenne = 0; antenne < ANTENNES; antenne++)
    {
      for (npic = 0; npic < nbpics[antenne]; npic++)
	{
#ifdef debug
	  write_str ("nbpics: ",NULL,NULL,usart_port);
	  writeHEXi (nbpics[antenne],NULL,NULL,usart_port);
#endif
	  balayage_primaire[antenne] = 0;	// rechercher dans la bande
	  if (bandes == 1)
	    {
#ifdef manuel
	      fstart[antenne*NBPICS_MAX+npic]=Fintsta[antenne * nbpics[antenne] + npic];
	      fstop[antenne *NBPICS_MAX+npic]=Fintsto[antenne * nbpics[antenne] + npic];
#else  // non-manuel
	      fstart[antenne*NBPICS_MAX+npic]=FSTART+((FSTOP-FSTART)/nbpics[antenne]) * npic;
	      fstop[antenne*NBPICS_MAX+npic]=fstart[antenne*NBPICS_MAX+npic]+(FSTOP-FSTART)/nbpics[antenne];
	      Fintsta[antenne * nbpics[antenne] + npic] = fstart[antenne * NBPICS_MAX + npic];
	      Fintsto[antenne * nbpics[antenne] + npic] = fstop[antenne * NBPICS_MAX + npic];
#endif
	      fstep[antenne*NBPICS_MAX+npic] =(fstop[antenne*NBPICS_MAX+npic]-fstart[antenne*NBPICS_MAX+npic])/NBSTEPS;
	    }
	  else			// absence de bande, on prend toute la gamme de mesures
	    {
#ifdef manuel
	      fstart[antenne* NBPICS_MAX+npic]=Fintsta[antenne*NBPICS_MAX];
	      fstop[antenne * NBPICS_MAX+npic]=Fintsto[antenne*NBPICS_MAX+nbpics[antenne]-1];
#else
	      fstart[antenne * NBPICS_MAX + npic] =FSTART;
	      fstop[antenne * NBPICS_MAX + npic] = FSTOP;
	      Fintsta[antenne * nbpics[antenne] + npic] = fstart[antenne * NBPICS_MAX + npic];
	      Fintsto[antenne * nbpics[antenne] + npic] = fstop[antenne * NBPICS_MAX + npic];
#endif
	      fstep[antenne*NBPICS_MAX+npic]=((fstop[antenne*NBPICS_MAX+npic]-fstart[antenne*NBPICS_MAX+npic])/NBSTEPS)/nbpics[antenne];
	    }

#ifdef debug
	  write_str ("\r\n",NULL,NULL,usart_port);
#endif
	}
    }

// CAS EUROCOPTER
/*  fstart[0]=FSTART; fstop[0]=(FSTOP+FSTART)/2; */

  for (antenne = 0; antenne < (ANTENNES_MAX*NBPICS_MAX/2); antenne++)
    {
    dac2[antenne] = OFFSET;
    }

#ifdef debug
  write_str("deb memset\r\n",NULL,NULL,usart_port);
#endif
  for (npic=0;npic<NBPICS_MAX*ANTENNES_MAX;npic++) parab_conti[npic]=0;
  for (npic=0;npic<NBPICS_MAX*ANTENNES_MAX;npic++) hpic[npic]=0;
//  bzero(parab_conti, sizeof (int) * NBPICS_MAX * ANTENNES_MAX);
//  memset (parab_conti, 0, sizeof (int) * NBPICS_MAX * ANTENNES_MAX);
//  memset (hpic, 0, sizeof (int) * NBPICS_MAX * ANTENNES_MAX);
#ifdef debug
  write_str("fin memset\r\n",NULL,NULL,usart_port);
#endif

#ifdef LCD
  for (npic = 0; npic < 5; npic++)
    {
      LCD_init ();
      delay (20000);
    }
  clearScreen ();
  delay (20);
  LCDPutStr ("Frequence", 15, 30, MEDIUM, GREEN, BLACK);
  delay (20);
  LCDSetLine (38, 0, 38, 131, BLACK);
  delay (20);
  LCDSetLine (50, 0, 50, 131, BLACK);
  delay (20);
  LCDSetLine (70, 0, 70, 131, BLACK);
  delay (20);
  LCDSetLine (82, 0, 82, 131, BLACK);
  delay (20);
  LCDPutStr ("Pic1: (hz)", 40, 15, MEDIUM, BLUE, WHITE);
  delay (20);

  LCDPutStr ("Pic2: (hz)", 72, 15, MEDIUM, BLUE, WHITE);
  delay (20);
#endif

  compteur = 0;
  nbpic_total = 0;
  for (antenne = 0; antenne < ANTENNES; antenne++)
    nbpic_total += nbpics[antenne];

// NE MARCHE PAS A L'INITIALISATION ?!
//#ifdef auto_offset
//      for (antenne = 0; antenne < ANTENNES; antenne++)
//      corrige_offset (&dac2[antenne]);
//#endif

  set_SPI_speedDiv(0x03); // accelere SPI
  sortie_courant_init();

#ifdef debug
  write_str("entre boucle\r\n",NULL,NULL,usart_port);
#endif

#ifdef MODBUS
//  modbus_new_rtu(&ctx, "hello", 38400, 'N', 8, 1);
//  modbus_set_debug(&ctx, FALSE);
//  modbus_set_slave(&ctx, 1);
//  modbus_connect(&ctx);
//  modbus_mapping_new(TAILLE_JMF, TAILLE_JMF, TAILLE_JMF, TAILLE_JMF);
//  for (npic=0;npic<10;npic++) mb_mapping.tab_registers[npic]=npic;
        if( ( eStatus = eMBInit( MB_RTU, MODBUS_SLAVE_ADD, 0, 38400, MB_PAR_EVEN ) ) != MB_ENOERR )
        {
#ifdef debug
         write_str("MODBUS init error\r\n",NULL,NULL,usart_port);
#endif
        }
        else
          {
#ifdef debug
           write_str("Init OK\r\n",NULL,NULL,usart_port);
#endif
          if( ( eStatus = eMBEnable(  ) ) != MB_ENOERR ) 
            {write_str("MODBUS enable error\r\n",NULL,NULL,usart_port);} // erreur d'Enable
          else
            {
#ifdef debug
             write_str("MODBUS init success\r\n",NULL,NULL,usart_port);
#endif
// cas serveur : on renverra une valeur contenue dans HoldingBuf
             init_status=1;
             for (npic=0;npic<REG_HOLDING_NREGS;npic++)   // initialisation des valeurs des Holding Registers
               {//usRegInputBuf[npic]=npic;   // inutile pour le moment
                usRegHoldingBuf[npic]=jmfhtons(npic+100); // c'est ce resultat qui est lu par la commande 0x03
               }
             f_tmp=(float)(tempdefault/10); //               init les T = -40
             i_tmp=(int*)(&f_tmp);
             *i_tmp=jmfhtonl(*i_tmp);  
             for (antenne=0;antenne<ANTENNES;antenne++)
               for (npic=0;npic<NBPICS_MAX;npic+=2)
                 {usRegHoldingBuf[(npic+antenne*NBPICS_MAX)+1]=((*i_tmp)&0xffff0000)>>16;// npic pair
                  usRegHoldingBuf[(npic+antenne*NBPICS_MAX)]=((*i_tmp)&0x0000ffff); // npic est pair
//writeHEXi(((*i_tmp)&0xffff),NULL,NULL,usart_port);
//write_str("\r\n",NULL,NULL,usart_port);
                 }
// cas client : on initialise le DAT3024
//              cmd_modbus[3]=0x0d;  // destination = config
//              cmd_modbus[4]=0;     // current output
//              cmd_modbus[5]=0;     // current output
//              eMBRTUSend(0x01,&cmd_modbus[1],5);   // 5 car elimine le 1er et les 2 derniers du CRC
            }
          } 
#endif

#ifdef AVERAGE
  init_sliding_avg();
#endif
  while (1)
    {
//  usb_comm();
  sortie_courant(courant<<4);courant+=50; if (courant>=0xfff) courant=0;
  kick_watchdog(); // reinitialise le watchdog

#ifdef periodique
      if (reveil==0) {delay(10000);power_down();} 
         else reveil--; // reveil est initialise' dans IRQ
#endif
      if (tim0 > 99999) tim0 = 0;
#ifdef compte_tour
      if (montim0 != 0)
	{
	  if (montim0 == 2)
	    if (mont2 != 0)
	      {
		jmf_putchar ('%',NULL,NULL,usart_port);
		jmf_putchar (' ',NULL,NULL,usart_port);
		writeDEC0i (mont2,NULL,NULL,usart_port);
		jmf_putchar ('\r',NULL,NULL,usart_port);
		jmf_putchar ('\n',NULL,NULL,usart_port);
		mont1 = 0;
		mont2 = 0;
		montim0 = 0;
	      }
	  CMPCON = 0x4C4;
	}
#endif
#ifdef MODBUS     // serveur, on repond aux requetes du client
//      rc = modbus_receive(&ctx, query);  // rc is the query size            CAS LIBMODBUS
//      if (rc != -1) {modbus_reply(&ctx, query, rc);}                        CAS LIBMODBUS
      ( void )eMBPoll( );                                       // CAS FREEMODBUS EN SERVEUR
      if ((ucMBAddress!=ucMBAddress_tmp)&&(eSndState == STATE_TX_IDLE)&&(eRcvState == STATE_RX_IDLE)) 
         {if ((ucMBAddress_tmp>0)&&(ucMBAddress_tmp<248)) 
             {ucMBAddress=ucMBAddress_tmp;      // update addr
#ifdef eeprom
	      eeprom_erase (0x00);
#endif
#ifdef at25
	      at25_erase (0x00);
#endif
              sauve_param ();
             }
             else ucMBAddress_tmp=ucMBAddress; // sinon cancel request
         }
      if ((rs_tmp!=RS_BAUDRATE)&&(eSndState == STATE_TX_IDLE)&&(eRcvState == STATE_RX_IDLE)) 
         if ((rs_tmp==9600)||(rs_tmp==19200)||(rs_tmp==38400)||(rs_tmp==57600)) 
            {RS_BAUDRATE=rs_tmp;set_serial_speed(RS_BAUDRATE,RS_PARITY);
#ifdef eeprom 
              eeprom_erase (0x00);
#endif
#ifdef at25
              at25_erase (0x00);
#endif
              sauve_param ();
            }
      if ((rs_tmp2!=RS_PARITY)&&(eSndState == STATE_TX_IDLE)&&(eRcvState == STATE_RX_IDLE)) 
         if ((rs_tmp2>=0)&&(rs_tmp2<3)) 
            {RS_PARITY=rs_tmp2;set_serial_speed(RS_BAUDRATE,RS_PARITY);
#ifdef eeprom
	      eeprom_erase (0x00);
#endif
#ifdef at25
	      at25_erase (0x00);
#endif
              sauve_param ();
            }
      if ((proportional==PROPORTIONAL_INIT)&&(eSndState==STATE_TX_IDLE)&&(eRcvState==STATE_RX_IDLE)) 
         {// tempdefault+=10;
          for (antenne=0;antenne<ANTENNES;antenne++)
           for (npic=0;npic<NBPICS_MAX;npic+=2)
               {f_tmp=(float)(tempdefault/10); //               reinit les T -40
                i_tmp=(int*)(&f_tmp);
                *i_tmp=jmfhtonl(*i_tmp);  
                usRegHoldingBuf[(npic+antenne*NBPICS_MAX)+1]=(((*i_tmp)&0xffff0000)>>16);// npic pair
                usRegHoldingBuf[(npic+antenne*NBPICS_MAX)]=((*i_tmp)&0x0000ffff); // npic est pair
 
#ifdef AVERAGE
               init_sliding_avg();
#endif
              }
         }
      if ((eSndState==STATE_TX_IDLE)&&(eRcvState==STATE_RX_IDLE)) 
        {
//         if (proportional==PROPORTIONAL_INIT) deinit_serial();
//         if (proportional==1) set_serial_speed(RS_BAUDRATE);
         if (proportional==(PROPORTIONAL_INIT+2))   // initialise le timer avant d'emettre
               {temporisation_modbus=tim0;proportional--;}
         if ((proportional>PROPORTIONAL_INIT)&&(tim0-temporisation_modbus)>(10+50*(MODBUS_SLAVE_ADD-1)*temporisation_modbus_cst))
               proportional--;                    // attente timer finie, on va commencer a emettre
         if ((proportional>0)&&(proportional<=PROPORTIONAL_INIT))
               proportional--;                      // on sonde PROPORTIONAL_INIT fois le capteur
        }
      if (modif_AT==1) 
         {// clignote();
          modif_AT=0; 
          maj_AT (fstart, fstop, fstep, puissance, dac2, &nbpic_total); // dac2 == offset
         }

//    cmd_modbus[3]=modbus_alternate+0x0e; // if (cmd_modbus[3]>0x0f) cmd_modbus[3]=0x0e;  // CAS CLIENT : BALAYAGE
// inutile de vouloir aller trop vite, il faut que DAT3024 aie le temps de timeout et renvoyer
// la reponse, donc soit on attend la reponse (a faire), soit on ne fait que une sortie par
// cycle de mesure du lecteur. ATTENTION au cas des mesures rapides (2 & 3 points).
//      cmd_modbus[4]=(((temperature_current[modbus_alternate])&0xff00)>>8); // argument hi (courant *5)
//      cmd_modbus[5]=((temperature_current[modbus_alternate])&0x00ff);      // argument lo (courant *5)
//      eMBRTUSend(0x01,&cmd_modbus[1],5);   // 5 car on elimine le 1er et les deux derniers du CRC
//      modbus_alternate++;		   // alterne la mise a jour des registres a chaque balayage
//      if (modbus_alternate >= ANTENNES) modbus_alternate = 0;
#endif
      // Gestion de le communication

/*
#ifdef MODBUS
           for (npic=0;npic<ANTENNES_MAX*NBPICS_MAX;npic+=2)  // test de transfert de float
               {f_tmp=((float)1234)/10.;
                i_tmp=(int*)(&f_tmp);
                *i_tmp=jmfhtonl(*i_tmp);  
                usRegHoldingBuf[npic+1]=((*i_tmp)&0xffff0000)>>16; // npic est pair
writeHEXi(((*i_tmp)&0xffff0000)>>16,NULL,NULL,usart_port);
write_str(" ",NULL,NULL,usart_port);
                usRegHoldingBuf[npic]=(*i_tmp)&0x0000ffff;     // npic est pair
writeHEXi(((*i_tmp)&0xffff),NULL,NULL,usart_port);
write_str(" ",NULL,NULL,usart_port);

               }
#endif
*/ 
      if (global_index != 0)
	{			// gestion de la communication
	  for (recu = 0; recu < global_index; recu++)
	    {			// jmf_putchar(global_tab[recu],NULL,NULL,usart_port); 
	      if ((global_tab[recu] == 'b') || (global_tab[recu] == 'B'))
		// oscilloscope (Fintsta, Fintsto, fstep, puissance);
		oscilloscope (fstart, fstop, fstep, puissance,dac2,phases);
#ifdef LMD
	      if ((global_tab[recu] == 'a') || (global_tab[recu] == 'A'))
		affiche_lmd = 1;
#endif
	      if ((global_tab[recu] == 'v') || (global_tab[recu] == 'V'))
		{
		  write_str (VERSION,NULL,NULL,usart_port);
		  write_str ("\r\n",NULL,NULL,usart_port);
		}
	      if (global_tab[recu] == '+')
		plus++;
	      else
		plus = 0;
#ifdef dephaseur
	      if (global_tab[recu] == 'p')
		for (antenne = 0; antenne < ANTENNES_MAX * NBPICS_MAX; antenne++)
		  {
		    phases[antenne] += 100;
		    if (phases[antenne] > 4000) phases[antenne] = 0;
		  }
	      if (global_tab[recu] == 'm')
		for (antenne = 0; antenne < ANTENNES_MAX * NBPICS_MAX; antenne++)
		  {
		    phases[antenne] -= 100;
		    if (phases[antenne] < 0) phases[antenne] = 4000;
		  }
#else
	      if (global_tab[recu] == 'p')
		{
		  decallage += 0x80000;
		  writeDECi (decallage,NULL,NULL,usart_port);
		  write_str ("\r\n",NULL,NULL,usart_port);
		}
	      if (global_tab[recu] == 'm')
		{
		  decallage -= 0x80000;
		  writeDECi (decallage,NULL,NULL,usart_port);
		  write_str ("\r\n",NULL,NULL,usart_port);
		}
#endif
#ifdef MODBUS
	      if (global_tab[recu] == 't')
		{
		  temporisation_modbus_cst =1-temporisation_modbus_cst;
		  writeDECi (temporisation_modbus_cst,NULL,NULL,usart_port);
		  write_str ("\r\n",NULL,NULL,usart_port);
		}
#endif
#ifdef gilloux
	      if (global_tab[recu] == 'P')
		{
                  phase_mag=1-phase_mag;
		}
#endif
	      if (global_tab[recu] == '!')
		{
		  deux_points = 1 - deux_points;
		}
	      if (global_tab[recu] == 'r')
		{
		  affiche_rs232= 1 - affiche_rs232;
		}
	      if (plus > 2)
		{
		  plus = 0;
		  communication (fstart, fstop, fstep, puissance, dac2,	// dac2 == offset
				 &nbpic_total,&schneider,phases);
		}
	    }
	  global_index = 0;
	}

#ifdef arcelor
      if (read_switch()==0) proportional=0; else proportional=PROPORTIONAL_INIT; // fermeture du switch met la broche a GND
#endif

      if (deux_points == 0)
	{       // desactive l'emission si proportional est a 0, cmd par ATg
	  if ((proportional > 0) && (proportional<=PROPORTIONAL_INIT))	
	    {
//     DAC1DAT = (((parab_conti[1]-0x2b000000)<<4)&0xfff0000); // sandvik, cas capteur 2
//     DAC3DAT = (((parab_conti[0]-0x29f00000)<<4)&0xfff0000); // sandvik, cas capteur 1
	      if (convertit_temperature == 0)
		{
//#ifndef convertit_temperature
		  if (mode_balayage_continu == 0)
		    set_DAC0 (0x0FFF0000);	// end pulse DAC
// DAC0DAT = (((parab_conti[1]-parab_conti[0]-0xa00000)&0xfff000)<<4);
		  else
		    {
// sinon DAC0DAT est defini dans interroge() et represente les pics sur oscillo
		      set_DAC0 (0xfff0000);
		      delay (1000);
		      set_DAC0 (0x00000000);
		    }
		}
//#endif
	      balaie[0] =0;
	      for (antenne = 0; antenne < ANTENNES; antenne++)
		{
#ifdef debug
		  write_str ("\r\n ",NULL,NULL,usart_port);
		  write_str ("Antenne ",NULL,NULL,usart_port);
		  writeDEC0i (antenne,NULL,NULL,usart_port);
		  write_str ("\r\n ",NULL,NULL,usart_port);
#endif
		  sortie_antenne (antenne);
#ifdef debug
		  write_str ("Nb_resonateur ",NULL,NULL,usart_port);
		  writeDEC0i (Nb_resonateur,NULL,NULL,usart_port);
		  write_str ("  \r\n ",NULL,NULL,usart_port);
#endif

		  if (nbpics[antenne] > 0)
		    {
		      // set_DAC2 (dac2[antenne] << 16); // desormais dans interroge()
#ifdef detec_tous
		      Nb_resonateur = 0;

#ifdef debug
                      write_str ("Appel balaie_ism: antenne=",NULL,NULL,usart_port);
                      writeHEXi (antenne,NULL,NULL,usart_port); 
#endif
		      balaie[0] =
			balaie_ism (&fstart[antenne * NBPICS_MAX],
				    &fstop[antenne * NBPICS_MAX],
				    &fstep[antenne * NBPICS_MAX],
				    &hpic[antenne * NBPICS_MAX],
				    &parab_conti[antenne * NBPICS_MAX],
				    &puissance[antenne * NBPICS_MAX],
				    &variance[antenne * NBPICS_MAX],
				    &balayage_primaire[antenne],
                                    &dac2[antenne * NBPICS_MAX/2],
				    Nb_resonateur, antenne, phases);
#else
		      nbtmp = nbpics[antenne];
		      nbpics[antenne] = 1;

		      for (Nb_resonateur = 0; Nb_resonateur < nbtmp;
			   Nb_resonateur++)
			{
			  balaie[Nb_resonateur] =
			    balaie_ism (&fstart[antenne*NBPICS_MAX+Nb_resonateur],
					&fstop [antenne*NBPICS_MAX+Nb_resonateur],
					&fstep [antenne*NBPICS_MAX+Nb_resonateur],
					&hpic  [antenne*NBPICS_MAX+Nb_resonateur],
					&parab_conti[antenne * NBPICS_MAX+Nb_resonateur],
					&puissance[antenne * NBPICS_MAX+Nb_resonateur],
					&variance[(Nb_resonateur+antenne*NBPICS_MAX)],
					&balayage_primaire[antenne],
                                        &dac2[antenne*NBPICS_MAX/2],
					Nb_resonateur, antenne, phases);

#ifdef debug
			  write_str (" var ",NULL,NULL,usart_port);
			  writeDEC0i (variance[Nb_resonateur+antenne*NBPICS_MAX],NULL,NULL,usart_port);
#endif
			}
		      nbpics[antenne] = nbtmp;
#endif
		    }
		  Nb_resonateur = 0;
#ifdef debug
		  write_str ("Balaie: ",NULL,NULL,usart_port);
		  writeDECc (balaie[Nb_resonateur],NULL,NULL,usart_port);
		  write_str ("\r\n",NULL,NULL,usart_port);
#endif
		  if (nbpics[antenne] > 0)
		    for (npic = 0; npic < (nbpics[antenne]); npic++)
		      {
#ifdef debug
			write_str ("SORTie Balaie_ism ",NULL,NULL,usart_port);
			write_str ("Npic[",NULL,NULL,usart_port);
			writeDEC0i (npic,NULL,NULL,usart_port);
			write_str ("] ",NULL,NULL,usart_port);
			write_str (" Fstar ",NULL,NULL,usart_port);
			writeHEXi (fstart[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" Fstop ",NULL,NULL,usart_port);
			writeHEXi (fstop[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" hpic ",NULL,NULL,usart_port);
			writeDEC0i (hpic[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" parab ",NULL,NULL,usart_port);
			writeHEXi (parab_conti[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" puiss ",NULL,NULL,usart_port);
			writeDEC0i (puissance[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" var ",NULL,NULL,usart_port);
			writeDEC0i (variance[npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
			write_str (" balaie   ",NULL,NULL,usart_port);
			writeDEC0i (balaie[npic],NULL,NULL,usart_port);
			write_str (" \r\n",NULL,NULL,usart_port);
#ifdef manuel
			write_str (" Fintsta ",NULL,NULL,usart_port);
			writeHEXi (Fintsta[npic],NULL,NULL,usart_port);
			write_str (" Fintsto ",NULL,NULL,usart_port);
			writeHEXi (Fintsto[npic],NULL,NULL,usart_port);
#endif
			write_str ("NBPICS :",NULL,NULL,usart_port);
			writeDEC0i (nbpics[antenne],NULL,NULL,usart_port);
			write_str (" ",NULL,NULL,usart_port);
			write_str (" balayage_primaire2 ",NULL,NULL,usart_port);
			writeDEC0i (balayage_primaire[antenne],NULL,NULL,usart_port);
			write_str (" \r\n",NULL,NULL,usart_port);
#endif

#ifdef detec_tous
			if ((balaie[0] > 0)
			    && (balayage_primaire[antenne] == 1)
			    && (mode_balayage_continu == 0))
#else
			if ((balaie[npic] > 0)
			    && (balayage_primaire[antenne] == 1)
			    && (mode_balayage_continu == 0))
#endif
			  {
			    // soit on rescanne toute la bande ou on se recentre sur le mode 3 points
			    //fstart[npic]=FSTART+((FSTOP-FSTART)*npic)/NBPICS;
			    //fstop[npic]=fstart[npic]+(FSTOP-FSTART)/NBPICS;

			    if (	// ( (variance[npic+antenne*NBPICS_MAX])<60000) && 
				(hpic[npic+NBPICS_MAX*antenne]>SEUILMIN) && (hpic[npic+NBPICS_MAX*antenne]<SEUILMAX))	// && (balaie[0]>10))
			      {
				fstart[antenne * NBPICS_MAX + npic] =
				  parab_conti[antenne*NBPICS_MAX+npic]-fstep[antenne*NBPICS_MAX+npic];
				fstop[antenne * NBPICS_MAX + npic] =
				  parab_conti[antenne*NBPICS_MAX+npic]+2*fstep[antenne*NBPICS_MAX+npic];
				// faux ? 26/05/2011 fstop[antenne*NBPICS_MAX+npic]=parab_conti[antenne*NBPICS_MAX+npic]+2*fstep[antenne*NBPICS_MAX+npic]; <- non, ok car la borne sup n'est
				//    jamais atteinte, on va < 2*fstep
				// fstep[npic]=0x44284; //13k  modification du 26/05/2011 : inutile
#ifdef debug
				write_str ("Fst et fstp \0",NULL,NULL,usart_port);
				writeHEXi (fstep
					   [antenne * NBPICS_MAX + npic],NULL,NULL,usart_port);
				jmf_putchar (' ',NULL,NULL,usart_port);
				writeHEXi (fstart
					   [npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
				jmf_putchar (' ',NULL,NULL,usart_port);
				writeHEXi (fstop
					   [npic + antenne * NBPICS_MAX],NULL,NULL,usart_port);
				jmf_putchar (' ',NULL,NULL,usart_port);
				write_str ("\r\n\0",NULL,NULL,usart_port);
#endif
			      }
			    //  else { balayage_primaire=0;}
			  }
			else 
if (bandes !=0)
			  {
#ifdef manuel
			    fstart[antenne*NBPICS_MAX+npic]=Fintsta[antenne*nbpics[antenne]+npic];
			    fstop[antenne*NBPICS_MAX +npic]=Fintsto[antenne*nbpics[antenne]+npic];
#else
			    fstart[antenne*NBPICS_MAX+npic]= FSTART+((FSTOP-FSTART)/nbpics[antenne])*npic;
			    fstop[antenne*NBPICS_MAX+npic] =fstart[antenne*NBPICS_MAX+npic]+(FSTOP-FSTART)/nbpics[antenne];
#endif
			    // fstep[antenne*NBPICS_MAX+npic]=(fstop[antenne*NBPICS_MAX+npic]-fstart[antenne*NBPICS_MAX+npic])/NBSTEPS; 
			  }
// fin de mode_3_point
		      }		// fin npic
		}		// fin antennes

	      // if (mode_balayage_continu==0) 
//#ifndef convertit_temperature
	      if (convertit_temperature == 0)
		set_DAC0 (0x00000000);	// end pulse DAC
//#endif
#ifdef periodique
	      if ((reveil>0) && (reveil<11)) {
#endif

#ifdef LMD
  	        if (affiche_lmd == 1)
	 	  {
		    jmf_putchar ('*',chaine,&chaine_index,usart_port);
		    jmf_putchar (' ',chaine,&chaine_index,usart_port);
                  }
#endif
	        if (trame_minimale == 0) // PAS trame minimale => affiche NBPICS
#ifdef LMD
  	        if (affiche_lmd == 1)
#endif
	          {
	           writeDECc ((char) nbpic_total,chaine,&chaine_index,usart_port);
	           jmf_putchar (' ',chaine,&chaine_index,usart_port);
		  }
#ifdef periodique
	      }	// on n'affiche que apres 4 premieres mesures pour faire converger AGC
#endif

// valeurs de test pour schneider (012714) jmfxx
//parab_conti[0]=0x44468700+0x268cf3         ;hpic[0]=3000;variance[0]=10; // inital = debut+100 k
//parab_conti[1]=0x44468700+0x268cf3+0x86ed53/2;hpic[1]=3001;variance[1]=11; // final = initial+700k
//parab_conti[2]=0x44468700+0x268cf3         ;hpic[2]=3000;variance[2]=10; // inital = debut+100 k
//parab_conti[3]=0x44468700+0x268cf3+0x86ed53;hpic[3]=3001;variance[3]=11; // final = initial+700k

	      for (antenne = 0; antenne < ANTENNES; antenne++)
		{		// on repasse sur antennes pour affichage
		  for (npic = 0; npic < (nbpics[antenne]); npic++)
		    {

#ifdef MODBUS
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+3]=jmfhtons(puissance[npic+antenne*NBPICS_MAX]);
 // parab_conti[npic +antenne * NBPICS_MAX]=0x12345678;
 // [01][03][00][00][00][08][44][0C]
 // Waiting for a confirmation...
 // <01><03><10><12><34><56><78><12><34><56><78><00><04><00><05><00><06><00><07><B1><9A>
 // 2:1234 3:5678 4:1234 5:5678 6:4 7:5 8:6 9:7 
//writeHEXi(parab_conti[npic +antenne * NBPICS_MAX],NULL,NULL,usart_port);
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+0]=jmfhtons((parab_conti[npic +antenne*NBPICS_MAX]&0xffff0000)>>16); 
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+1]=jmfhtons((parab_conti[npic +antenne*NBPICS_MAX]&0xffff)); 
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+2]=jmfhtons(hpic [npic+antenne*NBPICS_MAX]);
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+4]=jmfhtons((variance[npic+antenne*NBPICS_MAX])&0xffff);
#ifdef dephaseur
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+5]=jmfhtons(phases[antenne*NBPICS_MAX+npic]>>4);
#endif
                     usRegHoldingBuf[REG_PAR_RESONANCE*(npic+antenne*NBPICS_MAX)+NBPICS_MAX*ANTENNES_MAX+6]=jmfhtons(dac2[(npic/2+antenne*NBPICS_MAX)/2]);
#endif // MODBUS
// on a fini les mesures => controles automatique de gain et de phase
    	             if (agc == 1) // hpic=0 => puissance+=12 ; hpic=4095 => hpic-=3
		      {
//#ifdef AD9958      // 140515 : fonctionnel ? induit oscillation de la sortie du lecteur classique => retour a >>10
//                       puissance[antenne*NBPICS_MAX+npic]+=((CONSIGNE_PUISS-hpic[antenne*NBPICS_MAX+npic])>>8);
//#else
//                       puissance[antenne*NBPICS_MAX+npic]+=((CONSIGNE_PUISS-hpic[antenne*NBPICS_MAX+npic])>>7);
//#endif
                       puissance[antenne*NBPICS_MAX+npic]+=((CONSIGNE_PUISS-hpic[antenne*NBPICS_MAX+npic])>>10);
		       if (puissance[antenne * NBPICS_MAX + npic] > 32765)
		 	  puissance[antenne * NBPICS_MAX + npic] = 0;	// cas puissance < 0
		       if (puissance[antenne*NBPICS_MAX+npic]>PUISS_MAX)
		 	  puissance[antenne * NBPICS_MAX + npic] = PUISS_MAX; // seuil puissance
		      } // fin du controle de phase et de gain
#ifdef LMD
		     if (affiche_lmd == 1)
		      {
#endif
#ifdef periodique
		       if ((reveil>0) && (reveil<11)) // n'affiche que apres 4 mesures dans le vide
			{
#endif
		         if ((hpic[npic+NBPICS_MAX* antenne] > SEUILMIN)
		           && (hpic[npic+NBPICS_MAX*antenne]<SEUILMAX))
		           {if (variance[npic+NBPICS_MAX*antenne]<SEUILVAR)
		    	     {	// BMCI
			      if (trame_minimale == 0)
			        jmf_putchar ('4',chaine,&chaine_index,usart_port);
#ifndef f400
			      writeDECi (((((parab_conti[npic+antenne*NBPICS_MAX]-0x20000000)>>4)*40)/859)*16+24999809,chaine,&chaine_index,usart_port);
#else
#ifndef AD9958
			      writeDECi (((((parab_conti[npic+antenne*NBPICS_MAX] -
				 0x13000000)>>4)*80)/859)*16+29687500,chaine,&chaine_index,usart_port);
#else
                              writeDECi (((((parab_conti[npic+antenne*NBPICS_MAX]-0x40000000)
                                 >>5)*155)/1958)*32+25000000,chaine,&chaine_index,usart_port);  //rats(340e6/2^32) ; 32/2^32*340 MHz=2.5 Hz
#endif // AD9958
#endif // f400
			     }
			   else
			     {
                              if (trame_minimale==0)
			        write_str ("000000000\0",chaine,&chaine_index,usart_port);
                              else
			        write_str ("00000000\0",chaine,&chaine_index,usart_port);
			     }
			   jmf_putchar (' ',chaine,&chaine_index,usart_port);

			   if (trame_minimale == 0)
			    {
			     writeDECs (hpic [npic + antenne * NBPICS_MAX],chaine,&chaine_index,usart_port);
			     jmf_putchar (' ',chaine,&chaine_index,usart_port);
			     writeDECc (puissance [antenne * NBPICS_MAX + npic],chaine,&chaine_index,usart_port);
			     jmf_putchar (' ',chaine,&chaine_index,usart_port);
//#ifdef LMD
			     writeDEC0i (variance[npic + antenne * NBPICS_MAX],chaine,&chaine_index,usart_port);
			     jmf_putchar (' ',chaine,&chaine_index,usart_port);
			    }	// fin cas trame_minimale
			  }     // fin cas variance acceptable
			  else if (trame_minimale == 0)
			    {	// printf("00000000 %04d %d %d ",hpic[npic+antenne*NBPICS],puissance[npic+antenne*NBPICS],variance[npic+antenne*NBPICS]);
			      write_str ("000000000 \0",chaine,&chaine_index,usart_port);
			      writeDECs (hpic[npic + antenne * NBPICS_MAX],chaine,&chaine_index,usart_port);
			      jmf_putchar (' ',chaine,&chaine_index,usart_port);
			      writeDECc (puissance [antenne * NBPICS_MAX + npic],chaine,&chaine_index,usart_port);
			      jmf_putchar (' ',chaine,&chaine_index,usart_port);
//#ifdef LMD
			      writeDEC0i (variance
					  [npic + antenne * NBPICS_MAX],chaine,&chaine_index,usart_port);
			      jmf_putchar (' ',chaine,&chaine_index,usart_port);
//#else
//          writeDEC0i(variance[npic+antenne*NBPICS_MAX]);jmf_putchar(' ');     
//#endif
			    }
			  else {write_str ("00000000 \0",chaine,&chaine_index,usart_port);}

#ifdef dephaseur
                          if (schneider==1)
			   {writeDECi (phases[antenne * NBPICS_MAX + npic],chaine,&chaine_index,usart_port);
			    jmf_putchar (' ',chaine,&chaine_index,usart_port);
			    writeDECi (dac2[antenne*NBPICS_MAX/2+npic/2],chaine,&chaine_index,usart_port);
			    jmf_putchar (' ',chaine,&chaine_index,usart_port);
                           }
#endif
#ifdef periodique
			}
#endif
#ifdef LMD
			}
#endif
		      if ((convertit_temperature == 1) &&((npic%2)==0))
		       {		// for (npic=0;npic<nbpics[antenne];npic+=2) // si on veut afficher les temperatures pour toutes les paires de pics
		        if ((hpic[npic + antenne * NBPICS_MAX] >    SEUILMIN)
			  && (hpic[npic + 1 +antenne *NBPICS_MAX]>SEUILMIN)
			  && (hpic[npic + antenne * NBPICS_MAX] < SEUILMAX)
			  && (hpic[npic + 1 + antenne*NBPICS_MAX]<SEUILMAX)
			  && (variance[npic +antenne*NBPICS_MAX]< SEUILVAR)
			  && (variance[npic+1+antenne*NBPICS_MAX]<SEUILVAR))
			  {
			   if (nbpics[antenne]>1) {
/*
#ifndef AD9958
		  	    tempracine=A2[(antenne*NBPICS_MAX+npic)/2]*((((((parab_conti[npic+1+antenne*NBPICS_MAX]-parab_conti[npic+0+antenne*NBPICS_MAX])>>4)*40)/859)*16)/10);	
		  	    tempracine=A1[(antenne*NBPICS_MAX+npic)/2]+(tempracine/100);	
#else
		  	    tempracine=A2[(antenne*NBPICS_MAX+npic)/2]*((((((parab_conti[npic+1+antenne*NBPICS_MAX]-parab_conti[npic+0+antenne*NBPICS_MAX]))*155)/1958))/40);	
		  	    tempracine=A1[(antenne*NBPICS_MAX+npic)/2]+(tempracine/25);	
#endif

                  // f -> f/10 et A2*1E5 => au total sqrt(1E4)=1e2 et il faut A1*1e4
                  // remplacer par 1 et 0 si 1er pic pour T
  		  //tempracine=A1[npic/2]+A2[npic/2]*((((((parab_conti[npic+1+antenne*NBPICS_MAX]-   // remplacer par 1 et 0 si 1er pic pour T
			  //                           parab_conti[npic +
			  // ((((((parab_conti[3+antenne*NBPICS_MAX] -          // jerome 06/02/2012
			  //  parab_conti[2+antenne*NBPICS_MAX])>>4)*40)/859)*16)/10); // 06/02/2012
			    if (tempracine < 0) tempracine = 0;

			    tempracine=A0[(antenne*NBPICS_MAX+npic)/2]+jm_sqrt(tempracine);   	        // T en 1/10 K
	          // recu = 1600;      // test DAC: 1000 = 100 degC, et Vref=3.35 au lieu de 3.3
//#else   // cette version de modbus correspond a la sortie 4-20 mA representative de la freq

// ici version DATEXEL ou` on ENVOIE la commande de courant
// temperature_current[antenne]=(((((parab_conti[1+antenne*NBPICS_MAX]-parab_conti[0+antenne*NBPICS_MAX])>>4)*40)/859)*16);
// if ((temperature_current[antenne]<200000)||(temperature_current[antenne]>1224000)) // test en Hz
//    temperature_current[antenne] = 0;
// else
//   {temperature_current[antenne]=((temperature_current[antenne]-200000)>>6)+4000;
			// Df = 200.000 Hz -> 4 mA
			// Df = 1.224.000 Hz -> 20 mA
			// I = (Df - 200.000) / 64 + 4000 en uA
//		       }
//#endif  // MODBUS
*/
#ifndef dephaseur
//			   set_DAC0((((((int) tempracine + 500) << 16) / 625) << 10) & 0xfff0000);	// 4096/3300=1024/825 & 4096/2500=1024/625
//			   set_DAC1(((((parab_conti[1]-parab_conti[0]-contrainte0))*17)&0xfff0000));	// jerome 09/11/2012
			   set_DAC0(((((parab_conti[1]-parab_conti[0]-contrainte0))*17)&0xfff0000));	// Abelex 07/2014
			   // VREF=2.5 V
			   // f20-f10=433.97-433.32=0.65 MHz=0x1a9fbe7    1.25V=2048
			   // 130218 : demo sandvik
			   //set_DAC0 (((((int) parab_conti[1] - parab_conti[0] - contrainte0) / 6484 ) << 16) & 0xfff0000); // le contenu de la parenthese ne
			   //set_DAC1 (((((int) parab_conti[2] - parab_conti[0] - contrainte0) / 6484 ) << 16) & 0xfff0000); //    doit JAMAIS etre nul
#else
			   set_DAC0 ((((((int) tempracine + 500) << 16) / 625) << 10) & 0xfff0000);      // 4096/3300=1024/825 & 4096/2500=1024/625
#endif
                          if (schneider==1)
			     {writeDECTi ((int) tempracine,chaine,&chaine_index,usart_port);
                              jmf_putchar (' ',chaine,&chaine_index,usart_port);
                             }
#ifdef AVERAGE
                          tempracine=sliding_avg(tempracine,antenne,npic/2);
#endif
                          if (schneider==1)
                             {
                              writeDECTi ((int) tempracine,chaine,&chaine_index,usart_port);
                              jmf_putchar (' ',chaine,&chaine_index,usart_port);
                             }
#ifdef MODBUS
                            f_tmp=((float)tempracine)/10.;
                            i_tmp=(int*)(&f_tmp);
                            *i_tmp=jmfhtonl(*i_tmp);  
                            usRegHoldingBuf[(npic+antenne*NBPICS_MAX)+1]=((*i_tmp)&0xffff0000)>>16;// npic pair
                            usRegHoldingBuf[(npic+antenne*NBPICS_MAX)]=((*i_tmp)&0x0000ffff); // npic est pair
#endif
			    // DAC0DAT = (((parab_conti[0]-0x2aacc000-decallage)<<4)&0xfff0000);
			   }// cas d'un capteur de contrainte avec une seule resonance
			   else set_DAC0((((parab2[0]-0x2a400000-decallage)<<4)&0xfff0000));
			  } // si la mesure etait acceptable (criteres de variance et d'amplitude)
			else
			{
                         if (schneider==1) 
                            {writeDECTi (tempdefault,chaine,&chaine_index,usart_port); // print old value
                             jmf_putchar (' ',chaine,&chaine_index,usart_port);
#ifdef MODBUS
                             writeDECTi (usRegHoldingBuf[(npic+antenne*NBPICS_MAX)/2],chaine,&chaine_index,usart_port);
#else
                             writeDECTi (0,chaine,&chaine_index,usart_port);
#endif
                             jmf_putchar (' ',chaine,&chaine_index,usart_port);
                            }
			 set_DAC0 (0x0000);
//			 temperature_current[antenne] = 0;
			}
		      } // fin temperature == 1

// 140121 : on ne fait le dephasage que si la temperature a echoue
#ifdef dephaseur
// n'active le balayage de phase QUE si pas de signal recu && puiss. emise max
		        if (nbpics[antenne]>1) 
                          if ((npic%2)==0)
		           {if ((hpic[npic + antenne * NBPICS_MAX]   <SEUILMIN)
			      || (hpic[npic + 1 +antenne *NBPICS_MAX]<SEUILMIN)
			      || (hpic[npic + antenne * NBPICS_MAX]  >SEUILMAX)
			      || (hpic[npic + 1 + antenne*NBPICS_MAX]>SEUILMAX) 
                              || (variance[npic+antenne*NBPICS_MAX]  >SEUILVAR) 
                              || (variance[npic+1+antenne*NBPICS_MAX]>SEUILVAR)
		              || ((puissance[antenne*NBPICS_MAX+npic]==(PUISS_MAX))
		                 && (puissance[antenne*NBPICS_MAX+npic+1]==(PUISS_MAX)))
                              || ((dac2[npic/2+antenne*NBPICS_MAX/2]>0xf00)&&(OFFSET<=0xf00)))
                                 {if (cpt_phase[npic/2+antenne*NBPICS_MAX/2]==2)  // 3 fois = 0..2
		                      {phases[antenne * NBPICS_MAX + npic] += 100;
		                       if (phases[antenne * NBPICS_MAX + npic] > 4000)
		   	                  phases[antenne * NBPICS_MAX + npic] = 0;
		                       phases[antenne * NBPICS_MAX + npic+1] += 100;
		                       if (phases[antenne*NBPICS_MAX+npic+1] > 4000)
		   	                  phases[antenne* NBPICS_MAX+npic+1] = 0;
                                       cpt_phase[npic/2+antenne*NBPICS_MAX/2]=0;
			              } else cpt_phase[npic/2+antenne*NBPICS_MAX/2]++;  // on n'incremente la phase que 
                                  } else cpt_phase[npic/2+antenne*NBPICS_MAX/2]=0;      // ... si 3 mesures de suite erronees
                           }
#endif


		    }		// fin boucle sur npic
              sortie_led (balaie[0]);
//            DAC0DAT = (dac2 << 16) & 0xfff0000;
		}	// fin boucle antennes

#ifdef periodique
           if ((reveil>0) && (reveil<11)) // n'affiche que apres 4 mesures dans le vide
#endif
  	      if (trame_minimale == 0)
#ifdef rtc
#ifdef LMD
	         if (affiche_lmd == 1)
#endif
		    {
#ifndef RTC0
		     writeDEC0i (tim0,chaine,&chaine_index,usart_port);
#else
		     writeDECheure (heure,chaine,&chaine_index,usart_port);
		     writeDECheure (minute,chaine,&chaine_index,usart_port);
		     writeDECheure (seconde / 10,chaine,&chaine_index,usart_port);
#endif
		     jmf_putchar (' ',chaine,&chaine_index,usart_port);
		    }
//	    {writeDEC0i((int)temperature(),chaine,&chaine_index,usart_port);
//             jmf_putchar(' ',chaine,&chaine_index,usart_port);} // A VIRER pour gilloux
#else
#ifdef LMD
		 if (affiche_lmd == 1)
#endif
		    {writeDEC0i ((int) temperature (),chaine,&chaine_index,usart_port);
		     jmf_putchar (' ',chaine,&chaine_index,usart_port);
                    }
#endif

#ifdef periodique
           if ((reveil>0) && (reveil<11)) // n'affiche que apres 4 mesures dans le vide
#endif
	         if (trame_minimale == 0)
#ifdef detec_tous
#ifdef LMD
		   if (affiche_lmd == 1)
#endif
		     {
		      writeDECc (balaie[0],chaine,&chaine_index,usart_port);  // 110208 : retire un ' '
		     }
#else
	//for (npic=0;npic<(NBPICS);npic++)              
	//    { 
#ifdef LMD
		   if (affiche_lmd == 1)
#endif
		      writeDECc (balaie[1],NULL,NULL,usart_port);
		   // jmf_putchar(' ');
	//    }
#endif
        //GP3DAT |= 0x00010000;
#ifdef periodique
               if ((reveil>0) && (reveil<11)) // n'affiche que apres 4 mesures dans le vide
#endif
#ifdef LMD
	           if (affiche_lmd == 1)
#endif
		      write_str ("\r\n\0",chaine,&chaine_index,usart_port);
		   chaine[chaine_index]=0;
		   write_str (chaine,NULL,NULL,usart_port); // jusqu'ici on a forme' chaine, maintenant envoie
		   chaine_index=0;
#ifdef debug
		   write_str ("\r\n\0",NULL,NULL,usart_port);
#endif
		    //GP3DAT &= ~0x00010000;
		    //delay(100000);
#ifdef LMD
		   affiche_lmd = 0;
#endif
           }    // fin du cas proportional > 0 ie emission active par ATg (=0 inactif, >0 actif)
	    else
	    {puiss (0,3);}
	}	// fin boucle deux_points=0 (ie 3 points/fixe)
	else
	    {	// cas du mode deux points (copy paste du main de deux_points !

	//  for (antenne=0;antenne<ANTENNES_MAX;antenne++) puissance[antenne]=10;

	// memset(parab,0,sizeof(int)*NBPICS*ANTENNES); 
	// memset(hpic,0,sizeof(int)*NBPICS*ANTENNES);
	// for(i=0;i<NBPICS;i++){parab[npic]=0;}

	// recherche_asservissement_puissance_senseor(fstart, fstop, fstep, puissance);
					    // sinon DAC0DAT est defini dans interroge() et represente les pics sur oscillo

#ifdef Mode_FM_2p_et_senseor
	     asservissement_phase = 1;
	     premiere_foi = 0; 
#else
	     asservissement_phase = 0;
#endif
             while (deux_points == 1)
	       {	//clearScreen();
                kick_watchdog(); // reinitialise le watchdog
	        if (tim0 > 99999) tim0 = 0;
	        // printf("boucle\n"); // LCD_data(0x33);
	        //GP2DAT &= ~LCD_SCK;
	        //LCDSetPixel(30, 30, YELLOW);
	        // Gestion de le communication
	        if (global_index != 0)
		   {  // gestion de la communication
		    for (recu = 0; recu < global_index; recu++)
		      {
		       // jmf_putchar (global_tab[recu]);
		       if (global_tab[recu] == 'd')
		          affichage_2pts = 1 - affichage_2pts;
		       if (global_tab[recu] == 'p')
		         {decallage += 0x400;
			  writeDECi (decallage,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
                         }
		       if (global_tab[recu] == 'm')
		         {decallage -= 0x400;
			  writeDECi (decallage,NULL,NULL,usart_port);
			  write_str ("\r\n\0",NULL,NULL,usart_port);
                         }
		       if (global_tab[recu] == '!')
			 {deux_points = 1 - deux_points;
		          write_str ("\r\n!\r\n\0",NULL,NULL,usart_port);
                         }
		      }
		      global_index = 0;
                   }

		   for (antenne = 0; antenne < ANTENNES; antenne++)
		     {// set_DAC2 (dac2[antenne] << 16);	// fff avec mux : desormais dans interroge()
		      // balaie[antenne] = 0;
		      // GP0DAT=(GP0DAT&0xFF1FFFFF)+(antenne<<21); // OK Schrader, casse interrogateur WRC ?!
	//---------Alternance entre mode FM_2points et Balayage_senseor---------------//
#ifdef  Mode_FM_2p_et_senseor
		      asservissement_phase=1-asservissement_phase;
#endif
	   	      if (asservissement_phase == 0)
		         {if (localisation==0)	// etait while (localisation == 0)  120309
			    {for (npic=0;npic<nbpics[antenne];npic++) 
                                 choix_emission[antenne*NBPICS_MAX+npic]=2;   // pas encore init
			     localisation=acquisition (&fstart[antenne * NBPICS_MAX],
					 &fstop[antenne * NBPICS_MAX],&fstep[antenne * NBPICS_MAX],
					 &hpic[antenne * NBPICS_MAX],&puissance[antenne*NBPICS_MAX],
					 &parab1[antenne*NBPICS_MAX],&variance[antenne*NBPICS_MAX]);
#ifdef ASSERVISSEMENT_PUISSANCE
			     for (npic = 0; npic < nbpics[antenne]; npic++)
			        {puissance[antenne*NBPICS_MAX+npic]+=((CONSIGNE_PUISS-
                                    hpic[antenne*NBPICS_MAX+npic])>>10);
				 if (puissance[antenne*NBPICS_MAX+npic]>32765) 
                                    puissance[antenne * NBPICS_MAX+npic]=0;
			         if (puissance[antenne*NBPICS_MAX+npic]>PUISS_MAX) 
                                    puissance[antenne * NBPICS_MAX+npic]=PUISS_MAX;
                                }
#endif
			     if (localisation>0)	// 1 ou 0
			        {jmf_putchar ('%',chaine,&chaine_index,usart_port);
				 writeDECc (nbpics[antenne],chaine,&chaine_index,usart_port);
				 jmf_putchar (' ',chaine,&chaine_index,usart_port);
				 for (npic = 0; npic < nbpics[antenne];npic++)
				    {jmf_putchar ('4',chaine,&chaine_index,usart_port);
				     writeDECi ((((((parab1[antenne*NBPICS_MAX+npic]-0x15000000)>> 
                                         4)*80)/859)*16+32812500)/2,chaine,&chaine_index,usart_port);
				    // writeDECi ((((parab1[npic]-0x28000000)*40)/859 + 1250000));
				     jmf_putchar (' ',chaine,&chaine_index,usart_port);
				     writeDECs (hpic[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
		 		     jmf_putchar (' ',chaine,&chaine_index,usart_port);
				     writeDECc (puissance[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				     jmf_putchar (' ',chaine,&chaine_index,usart_port);
				     writeDECs (hpic[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				     jmf_putchar (' ',chaine,&chaine_index,usart_port);
                                    }
				}
			     else
			        {for (npic=0;npic<nbpics[antenne];npic++)
				   {
                                    writeDECc (nbpics[antenne],chaine,&chaine_index,usart_port);
//				    write_str ("000000000 \0",chaine,&chaine_index,usart_port);  // jmfxx probleme 150716
				    writeDECs (hpic[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				    writeDECc (puissance[antenne * NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				    jmf_putchar (' ',chaine,&chaine_index,usart_port);
				    writeDECs (hpic[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				    jmf_putchar (' ',chaine,&chaine_index,usart_port);
                                   }
				}
#ifdef rtc
			         writeDEC0i (tim0,chaine,&chaine_index,usart_port);
                                 jmf_putchar (' ',chaine,&chaine_index,usart_port);
#else
			         writeDEC0i ((int) temperature (),chaine,&chaine_index,usart_port);
			         jmf_putchar (' ',chaine,&chaine_index,usart_port);
#endif
				 writeDECc (0,chaine,&chaine_index,usart_port); 
                                 jmf_putchar ('\r',chaine,&chaine_index,usart_port); 
                                 jmf_putchar ('\n',chaine,&chaine_index,usart_port);
                            } //While(localisation)
			    else
			    {
// Les pics ont ete localises, enregistrement de la position 
#ifdef  Mode_FM_2p_et_senseor
			     if (premiere_foi == 0)
			        for (npic = 0; npic < nbpics[antenne]; npic++) 
                                    parab2[npic] = parab1[npic];
				premiere_foi = 1;
#else
				for (npic = 0; npic < nbpics[antenne]; npic++) 
                                    parab2[npic] = parab1[npic];
				asservissement_phase = 1;
#endif
				compteur = 0;
                            }	// fin du else de if (localisation == 0)
			}	// if (asservissement_phase == 0)
			else
			{	// Passage en mode asservissement sur phase
			 if (affichage_2pts == 1)
			    {writeDECc (nbpics[antenne],chaine,&chaine_index,usart_port);
			     jmf_putchar (' ',chaine,&chaine_index,usart_port);
                            }
			 nb_pic_trouve = 0;
			 for (npic = 0; npic < nbpics[antenne]; npic++)
			    {asservissement_sur_phase (&parab2[antenne * NBPICS_MAX], npic, 
                               &puissance[antenne*NBPICS_MAX],&frequence_asservissement, 
                               &diff_de_phase[2*antenne*NBPICS_MAX],
                               &choix_emission[antenne*NBPICS_MAX]); 
			     if (diff_de_phase[2*(antenne*NBPICS_MAX+npic)+choix_emission[antenne*NBPICS_MAX + npic]]>SEUILMIN
			         && diff_de_phase[2*(antenne*NBPICS_MAX+npic)+choix_emission[antenne * NBPICS_MAX+npic]]<SEUILMAX)
			        {nb_pic_trouve++;	// else{ write_str("variance");}
				 parab2[antenne * NBPICS_MAX + npic] =
				 frequence_asservissement;
                                }
#ifdef ASSERVISSEMENT_PUISSANCE
		             puissance[antenne*NBPICS_MAX+npic]+=((CONSIGNE_PUISS -
                                diff_de_phase[2 * antenne * NBPICS_MAX+npic+choix_emission[antenne*NBPICS_MAX+npic]])>>10);
			        if (puissance[antenne*NBPICS_MAX+npic]>32765) 
                                   puissance[antenne * NBPICS_MAX+npic]=0;
			        if (puissance[antenne * NBPICS_MAX+npic]>PUISS_MAX) 
                                   puissance[antenne*NBPICS_MAX + npic] = PUISS_MAX;
#endif
			    }	//for(npic=0....
			if (nb_pic_trouve == nbpics[antenne])
			   {if (affichage_2pts == 1)
			      {compteur++;
			       for (npic = 0; npic < nbpics[antenne]; npic++)
			         { // jmf_putchar('4');jmf_putchar('3');
                                   // writeHEXi((((parab2[npic]))));  // -0x28000000)*40)/859+1250000));
			          jmf_putchar ('4',chaine,&chaine_index,usart_port); 
				  writeDECi ((((((parab2[antenne*NBPICS_MAX+npic]-0x15000000)>>4)*80)/859)*16+32812500)/2,chaine,&chaine_index,usart_port); 
				  jmf_putchar (' ',chaine,&chaine_index,usart_port); 
				  writeDECs (diff_de_phase[2 * antenne * NBPICS_MAX + 2 * npic],chaine,&chaine_index,usart_port); 
				  jmf_putchar (' ',chaine,&chaine_index,usart_port); 
				  writeDECc (puissance[antenne * NBPICS_MAX + npic],chaine,&chaine_index,usart_port); jmf_putchar (' ',chaine,&chaine_index,usart_port); 
                                  writeDECs (diff_de_phase[2*antenne*NBPICS_MAX+2*npic+1],chaine,&chaine_index,usart_port); 
				  jmf_putchar (' ',chaine,&chaine_index,usart_port);
			         }	//for
//#else
			      }
			    // DAC0DAT = (((parab2[0]-0x2aacc000-decallage)<<4)&0xfff0000);
//			    if (nbpics[antenne] == 1)
//			       set_DAC0 ((((parab2[0]-0x2ab00000-decallage) << 4) & 0xfff0000));
			    //DAC0DAT = ((int)(choix_emission[0])*0xfff0000);  // test de vitesse
//			    if (nbpics[antenne]>= 2) 
//                               set_DAC0((((parab2[1]-parab2[0]-0x880000-decallage)<<4)&0xfff0000));	// 415 kHz
#ifdef  Mode_FM_2p_et_senseor
			    if (compteur > 9)
			      {asservissement_phase = 0;
			       localisation = 0; compteur = 0;
                              }
#endif
			   }	// if(nb_pic...)
			 else
			   {localisation = 0;
			    asservissement_phase = 0;
			    if (affichage_2pts == 1)
			      {for (npic = 0; npic < nbpics[antenne];npic++)
			         {
				  write_str ("000000000 \0",chaine,&chaine_index,usart_port);
				  writeDECs (diff_de_phase[2*antenne*NBPICS_MAX+2*npic],chaine,&chaine_index,usart_port);
				  writeDECc (puissance[antenne*NBPICS_MAX+npic],chaine,&chaine_index,usart_port);
				  jmf_putchar (' ',chaine,&chaine_index,usart_port);
				  writeDECs (diff_de_phase[2*antenne*NBPICS_MAX+2*npic+1],chaine,&chaine_index,usart_port);
				  jmf_putchar (' ',chaine,&chaine_index,usart_port);
			         }
			      }
#ifdef  Mode_FM_2p_et_senseor
			      for (npic = 0; npic < nbpics[antenne]; npic++) 
                                parab2[antenne*NBPICS_MAX+npic] =parab1[antenne * NBPICS_MAX+npic];
#endif
			   }
		           if (affichage_2pts == 1)
			    {
#ifdef rtc
			     writeDEC0i (tim0,chaine,&chaine_index,usart_port); 
                             jmf_putchar (' ',chaine,&chaine_index,usart_port);
#else
			     writeDEC0i ((int) temperature (),chaine,&chaine_index,usart_port);
			     jmf_putchar (' ',chaine,&chaine_index,usart_port);
#endif
			     writeDECc (0,chaine,&chaine_index,usart_port);	// temperature balaie
			     jmf_putchar ('\r',chaine,&chaine_index,usart_port); 
                             jmf_putchar ('\n',chaine,&chaine_index,usart_port);
	  
			     chaine[chaine_index]=0;
			     write_str(chaine,NULL,NULL,usart_port); // jusqu'ici forme' chaine, maintenant affiche
                            }
			  chaine_index=0; // pour 2 pics, chaine_index vaut 67 lors de l'affichage

#ifdef  Mode_FM_2p_et_senseor
			  asservissement_phase = 1;
#endif
			  compteur = 0;}	//else
			 }	// fin du for antenne
		       }	// fin du while(deux_points==1)
		    }	// fin de la boucle "methode 2 points"
	    }	// fin boucle while
         return (0);
}	// fin boucle main

int acquisition (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep,
int *hpic, unsigned short *puissance, unsigned int *parab, unsigned int *variance)
{
  unsigned short tab0[NBSTEPS_MAX * NBPICS_MAX + 2];
  int moyenne[2],mes; unsigned int FREQ = 0;
  unsigned char total = 0, SEC = 0, position_table = 0, bsp = 0, npic = 0, bsptmp = 0;
  unsigned int tab_variance[NBPICS_MAX * NBMOY_MAX], parabtmp[NBPICS_MAX * NBSTEPS_MAX + 2], freqres[NBPICS], tmp;
 //--------------Ajout de la methode de senseor----------------------/
  for (npic = 0; npic < NBPICS; npic++) parab[npic] = 0; 
  bsp = 0;
  do {
      total++;
      position_table = 0;
      for (npic = 0; npic < NBPICS; npic++)
        {
         for (FREQ = fstart[npic]; FREQ < fstop[npic]; FREQ += fstep[npic])
           {
	    // envoi_DDS (FTW0, 5);  retrait 130710 jmfriedt en passant FREQ en arg de interroge
	    tab0[position_table] =0;
            for (mes=0;mes<NBMES;mes++)
	        tab0[position_table] += interroge (puissance[npic],FREQ,OFFSET,3)&0xffff;
    	    tab0[position_table]/=NBMES;
	    position_table++;}
	}

#ifdef AMPLITUDE
      // Affichage amplitude des points
      for (i = 0; i < position_table; i++)
          {writeDECi (tab0[i],NULL,NULL,usart_port); jmf_putchar (' ',NULL,NULL,usart_port);}
      jmf_putchar ('\r',NULL,NULL,usart_port); jmf_putchar ('\n',NULL,NULL,usart_port);
#endif
      position_table /= NBPICS;
      //detection de max
      bsptmp = 0;
      for (npic = 0; npic < NBPICS; npic++)
         {hpic[npic] = 0;
          bete_max (&tab0[position_table * (npic)],
          position_table, moyenne);
          if (hpic[npic] < moyenne[1]) hpic[npic] = moyenne[1];
          if ((moyenne[1] > SEUILMIN) && (moyenne[1] < SEUILMAX))
	      {
	       bsptmp++; freqres[npic] = (unsigned int) ((fstart[npic] + ((int) moyenne[0]) * fstep[npic])); parabtmp[npic] = ((freqres[npic]) + parabole (&tab0[position_table * npic], moyenne[0], fstep[npic]));	// donnees, position du max, pas de freq
	      }
	   }	//write_str(" \r\n ");
         if (bsptmp == NBPICS)	// si tous les pics ont ete identifies
	   {bsp++;
	    for (npic = 0; npic < NBPICS; npic++)
	      {parab[npic] += (parabtmp[npic] & 0x07FFFFFF);
	       tab_variance[npic + (bsp - 1) * NBPICS] = (parabtmp[npic] & 0x07FFFFFF);}
	   }
    }
    while ((bsp < NBMOY) && (total < (NBRATES * NBMOY)));
	//----------------------------------------------------------------------------//
	     for (npic = 0; npic < NBPICS; npic++)
		{parab[npic] /= bsp; FREQ = 0;	// DEBUT VARIANCE
		 for (SEC = 0; SEC < bsp; SEC++)
		    {tmp =((int)abs (tab_variance[SEC * NBPICS + npic] -parab[npic]) >> 10);
		     FREQ += tmp * tmp;}
		 variance[npic] = FREQ / bsp;	// FIN VARIANCE  
		 parab[npic] += 0x28000000;}
		 if (bsp == NBMOY) return (1); 
		    else return (0);
}

void recherche_asservissement_puissance_senseor (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep, unsigned short *puissance)
{
  unsigned int npic = 0;
  int mes;
  unsigned short tab_tempo[NBSTEPS_MAX * NBPICS_MAX];
  unsigned int FREQ = 0;
  unsigned char position_table = 0;
  unsigned short asservissement;
  //--------------Ajout de la méthode de senseor----------------------/
#ifndef AD9958  // A REVOIR DANS LE CAS DU AD9958 !
// for (i = 0; i < 5; i++) CFR1[i] = 0x00;  130711 jmfriedt : a quoi ca sert ? CFR1 a toujours ete a 0 !
//// init DDS
// for (SEC = 0; SEC < 10; SEC++)
// {
//   envoi_DDS (CFR1, 5); delay (50);}      130711 jmfriedt : a quoi ca sert ? CFR1 a toujours ete a 0 !
#endif
  position_table = 0;
  for (npic = 0; npic < NBPICS; npic++)
       {
        for (FREQ = fstart[npic]; FREQ < fstop[npic]; FREQ += fstep[npic])
 	  {
   	   // envoi_DDS (FTW0, 5); retrait jmfriedt 130710 en passant FREQ en arg de interroge
	    tab_tempo[position_table] = 0;
            for (mes=0;mes<NBMES;mes++)
	        tab_tempo[position_table] += interroge (puissance[npic],FREQ,OFFSET,3)&0xffff;
	    tab_tempo[position_table] /= NBMES;
	    position_table++;
	    if (tab_tempo[position_table] > SEUILMAX)
	       {asservissement = 0xFFF;
		while (asservissement > SEUILMAX)
		    {
		     puissance[npic] += ((CONSIGNE_PUISS - asservissement) >> 10);
		     if (puissance[npic] > 32765) puissance[npic] = 0;
		     if (puissance[npic] > PUISS_MAX) puissance[npic] = PUISS_MAX;
		     asservissement = interroge (puissance[npic],FREQ,OFFSET,3)&0xffff;
                    }
	       }
	  }	// fin de Freq
      }	// fin de npic      
}

int asservissement_sur_phase (unsigned int *fstart, unsigned char npic, unsigned short *puissance,
      unsigned int *fre, unsigned short *diff_de_phase, unsigned char *choix_emission)
{	// jmfriedt
  unsigned int FREQ = 0, tab_FM[2] = {0, 0x346D0}, s, nb_err = 0; 
  unsigned char i, SEC = 0; 
  unsigned short tab0[2 * NBMOY_MAX]; 
  int phase = 0; 
  int choix_tmp = 0; 

  FREQ = fstart[npic]; 
  if (choix_emission[npic] != 2) choix_emission[npic] = 1 - (choix_emission[npic]);	// mode 1 point
     else {choix_emission[npic] = 1; choix_tmp = 2;}
  //  for (*choix_emission = 0; *choix_emission < 2; (*choix_emission)++)
  {
    SEC = 0; while (SEC < NBMOY)
      {
       s = (tab_FM[choix_emission[npic]]) + FREQ - (0x1a360);	// jmfriedt (vire *2)
       // envoi_DDS (FTW0, 5);
       tab0[SEC] = interroge (puissance[npic],s,OFFSET,3)&0xffff;
	// Pour resoudre temporairement le probleme de l'apparaition du 64 en amplitude        
        if (tab0[SEC] < 100) {nb_err++;}
        SEC++;}
  }
  if (NBMOY > 1)
    {s = 0;
     for (i = 0; i < NBMOY; i++) s += tab0[i];
     diff_de_phase[(choix_emission[npic]) + npic * 2] = s / NBMOY;
     if (choix_tmp == 2) diff_de_phase[1 - (choix_emission[npic]) + npic * 2] = s / NBMOY;
    }
  else
    {diff_de_phase[choix_emission[npic] + npic * 2] = tab0[0]; 
     if (choix_tmp == 2) diff_de_phase[1 - (choix_emission [npic]) + npic * 2] = tab0[0];
    }
  phase = diff_de_phase[npic * 2 + 1] - diff_de_phase[npic * 2 + 0];	//resultat de la mesure de phase
  *fre = FREQ + phase * proportional;	// 744    // jmfriedt : ici on recherche l'intersection a 0
					//     proprement au lieu de faire un *4
					//  -> on cherche a equilibrer les reponses de part et 
					//     d'autre de la resonance
	//  hpic[npic + 2] = diff_de_phase[1];
	//  hpic[npic] = diff_de_phase[0];
  return (0);
}

