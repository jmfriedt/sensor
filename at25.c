#include<ADuC7026.h>
#include<stdio.h>         // pour NULL
#include"at25.h"

void delay (int length);  // dans DDS.c

// BIEN PENSER A UNLOCK A LA 1ERE UTILISATION UNE NOUVELLE FLASH
// diode  GND X4       2o SO
// SMA        o         o
//            o AT25DF  o              ref 1715438 chez farnell
// DB9        o  021    o
//         CK o6       1o CS2#
//         SI o5       8o Vcc

// commande, argument(s), nombre d'octets ecrits, nombre d'octets lus
void at25_envoi(unsigned char *cmd, int p,unsigned char *entree, int n,int m)
{int k=0;__attribute__((unused)) unsigned char c;
 GP1DAT &= ~0x00800000;   // CS2# P1.7 = 0
 delay(5);
 if (p>0)
   for (k=0;k<p;k++)
     {SPITX = cmd[k];	  // transmit command or any dummy data
      do {} while ((SPISTA & 0x01) == 0x01) ; // wait for data rcv status bit
      c=SPIRX;
     }

 if (n>0)
   for (k=0;k<n;k++)
     {SPITX = entree[k];	  // transmit command or any dummy data
      do {} while ((SPISTA & 0x01) == 0x01) ; // wait for data rcv status bit
      c=SPIRX;
     }

 SPITX = entree[k]; // dummy read
 do {} while ((SPISTA & 0x01) == 0x01) ; 
 c=SPIRX;

 if (m>0) 
  {for (k=0;k<m;k++)
     {SPITX=0xFF;
      do {} while ((SPISTA & 0x08) == 0x00) ; // wait for data recv status bit
      entree[k]=SPIRX;
     }
  }
 delay(100);
 GP1DAT |= 0x00800000; 
}

void at25_id(unsigned char *entree)
{unsigned char memoire;
 memoire=0x9f; at25_envoi(&memoire,1,entree,0,5); // read ID
}

void at25_we() // write enable (0x06), necessaire avant toute ecriture
{unsigned char memoire;
 memoire=0x06;          
 at25_envoi(&memoire,1,NULL,0,0); 
}

void at25_erase(int addr)  // erase block
{unsigned char memoire[4];
 at25_we();
 memoire[0]=0x20; 
 memoire[1]=(addr&0x00ff0000)>>16;
 memoire[2]=(addr&0x0000ff00)>>8;
 memoire[3]=(addr&0x000000ff);
 at25_envoi(memoire,4,NULL,0,0);
	
 do {memoire[0]=0x05;          // read status register (doit renvoyer 16)
     at25_envoi(memoire,1,memoire,0,2);}
 while ((memoire[1]&0x01)!=0);
}

void at25_read(int addr,unsigned char* memoire, int n)  // read block
{unsigned char cmd[4];
 cmd[0]=0x03; 
 cmd[1]=(addr&0x00ff0000)>>16;
 cmd[2]=(addr&0x0000ff00)>>8;
 cmd[3]=(addr&0x000000ff);
 at25_envoi(cmd,4,memoire,0,n);
}

void at25_unlock(int addr) 
{unsigned char memoire[4];
 at25_we();
 memoire[0]=0x39; 
 memoire[1]=(addr&0x00ff0000)>>16;
 memoire[2]=(addr&0x0000ff00)>>8;
 memoire[3]=(addr&0x000000ff);
 at25_envoi(memoire,4,NULL,0,0);
}

/*
// read sector protection status
memoire[0]=0x3c; memoire[1]=0x00; memoire[2]=0x00; memoire[3]=0x00;
at25_envoi(memoire,4,3);
for (k=0;k<3;k++) writeHEXs((unsigned short)memoire[k]);
*/

void at25_write(int addr,unsigned char* data,int n) 
{unsigned char cmd[4];
 at25_we();
 cmd[0]=0x02; // 3. write block (0x02)
 cmd[1]=(addr&0x00ff0000)>>16;
 cmd[2]=(addr&0x0000ff00)>>8;
 cmd[3]=(addr&0x000000ff);
 at25_envoi(cmd,4,data,n,0);
	
 do {cmd[0]=0x05;          // read status register (doit renvoyer 16)
     at25_envoi(cmd,1,cmd,0,2);}
 while ((cmd[1]&0x01)!=0);
}

