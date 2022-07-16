/*********************************************************************
 Authors       : David Rabus
 Date          : Fev. 2011
 File          : multi_voies.c
 Hardware      : Applicable to ADuC7026 rev H or I silicon.
*********************************************************************/

/*
Lecture / ecriture sur ROM : 
inclure le fichier flash.h
FEEMOD = 0x18 pour autoriser la lecture/ecriture/effacement sur ROM
FEEADR = adresse du bloc : 16 bits ex : FEEADR=0x6000;
FEECON=0x01 : single read : lit ce qu'il y a a l'adresse indiquee dans FEEADR
                et place le resultat dans FEEDAT
FEECON=0x02 : single write ecrit ce qu'il y a dans FEEDAT a l'adresse FEEADR
FEECON=0x03 : erase/write (erase sur 512 adresses)
FEESTA : status des actions effectuees
FEESTA=0x01 : la commande s'est effectuee correctement, mis a zero quand on 
                lit ce registre
FEESTA=0x02 : commande failed
*/

#include  <ADuC7026.h>
#include  "eeprom.h"

void eeprom_erase(int addr)
{__attribute__((unused)) volatile unsigned long tmp;
 FEEADR=0x6000+addr;
 FEEDAT=0xFFFF;
 FEECON=0x03;
 tmp=FEESTA;
}

void eeprom_write(int addr,unsigned char* data,int n) 
{__attribute__((unused)) volatile unsigned long tmp;
 int i;
 for (i=0;i<n;i++)
   {FEEADR = 0x6000+addr+2*i;
    FEEDAT = (short)data[i];
    FEECON = 0x02; // 0x02 = single write (DOIT etre a adresse paire, donc i*2)
    tmp = FEESTA;  // 0x03 = erase+write, plus lent & rm les vals precedentes
//    jmf_putchar('w');
//    writeHEXi(data[i]);jmf_putchar(' ');
//    writeHEXi(tmp&0x0f);jmf_putchar('\r');jmf_putchar('\n');
  }
}

void eeprom_read(int addr,unsigned char *memoire , int n)
{int i;
 __attribute__((unused)) volatile unsigned long tmp;
 for (i=0;i<n;i++)
   {FEEADR = 0x6000+addr+2*i;
    FEECON = 0x01;
    tmp = FEESTA;
    memoire[i] =(char)(FEEDAT&0xff);
//    jmf_putchar('r');
//    writeHEXi(memoire[i]);jmf_putchar(' ');
//    writeHEXi(tmp&0x0f);jmf_putchar('\r');jmf_putchar('\n');
   }
}

void eeprom_init()
{
    FEEMOD = 0x18;  //autorisation d'ecriture sur ROM + erase
}
