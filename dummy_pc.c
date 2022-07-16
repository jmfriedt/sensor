// attention au cas #define 400 qui decale la bande de freq
// a executer UNIQUEMENT dans une console, PAS dans un screen ou xterm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#include "DDSinc.h"

extern unsigned char global_tab[NB_CHARS];
extern volatile int heure,minute,seconde,tim0;
extern int global_index;

volatile int io_happened,pc_offset=0; // alarm_happened, ctrl_c_happened,

void handle_alarm(int xxx)
{ //  printf("RTC : %d %d %d\n",seconde,minute,heure);
  tim0+=10;
  seconde+=10;
  if (seconde>600){minute++;seconde=0;}
  if (minute>60) {heure++;minute=0;}
  alarm(1); 
}

void handle_io(int xxx)
{ // io_happened=1; 
  global_tab[global_index] = getchar ();
  if (global_index < (NB_CHARS - 1))
     global_index++;
}

extern unsigned char FTW0[5];

void envoi_PLL(unsigned char *entree) 
{//printf("appel envoi_PLL\n");
}

void envoi_DDS(unsigned char *entree,int n) 
{// printf("appel envoi_DDS\n");
}

void puiss(unsigned short puissance,__attribute__((unused)) unsigned char chan) {}

void corrige_offset(unsigned int* a) {}

#ifndef f400
#define f01 719407022.        // 33.5 MHz en format DDS, resonance 1
//#define f01 891205714.        // 41.5 MHz en format DDS, resonance 1
#define f02 740881858.        // 34.5 MHz en format DDS, resonance 2
#define df  900000.           // largeur de la resonance, en format DDS
#else
#define f01 (719407022./2)    // 33.5 MHz en format DDS, resonance 1
#define f02 (740881858./2)    // 34.5 MHz en format DDS, resonance 2
#define df  (900000./2)       // largeur de la resonance, en format DDS
#endif

#define bruit (RAND_MAX/100)  // 1000 = tres bruite', 100=ok

unsigned short interroge(unsigned short puissance,unsigned int freq,__attribute__((unused))unsigned int offset,__attribute__((unused))unsigned char chan) 
{//int freq;
 float reponse;
// freq=FTW0[1]*65536*256+FTW0[2]*65536+FTW0[3]*256+FTW0[4];
 reponse =exp(-(((float)freq-f01)/df)*(((float)freq-f01)/df))*3100.;
 reponse+=exp(-(((float)freq-f02)/df)*(((float)freq-f02)/df))*3100.;
 reponse+=(float)((rand()-RAND_MAX/2)/bruit); // ajout du bruit;
 // reponse+=(float)(2048-pc_offset);
 if (reponse<0.) reponse=0.;
 if (reponse>4095.) reponse=4095.;
 // printf("%d %f\n",freq,reponse);
 usleep(60);
 return((unsigned short)reponse);
}

void delay (int length) {}

unsigned short temperature(void) {return(0x123);}

void eeprom_erase(int a) {}

void eeprom_write(int a,unsigned char* b,int c) {}

void eeprom_read(int a,unsigned char* b, int c) {}

void eeprom_init(void) {}

void
jmf_putchar (int a,char *chaine, int* chaine_index,__attribute__((unused))int port)
{
  if (chaine != NULL)
    {chaine[*chaine_index]=a;
     *chaine_index=*chaine_index+1;
    }
  else
    {
     putchar(a); /* fprintf(stdout,"%c",a); */ fflush(stdout);
    }
}


void init_CPU(void)  // passe le clavier en non-bloquant et genere un signal quand 
{                    //   une touche est appuyee
 global_index=0;

// ctrl_c_happened=0;
  io_happened=0;
  signal(SIGALRM, handle_alarm);
// signal(SIGINT, handle_ctrl_c);
  struct termios buf;
  tcgetattr(0, &buf);
  buf.c_lflag &= ~(ECHO | ICANON);
  buf.c_cc[VMIN]=1;
  buf.c_cc[VTIME]=0;
  tcsetattr(0, TCSAFLUSH, &buf);
  signal(SIGIO, handle_io);
  int savedflags=fcntl(0, F_GETFL, 0);
  fcntl(0, F_SETFL, savedflags | O_ASYNC | O_NONBLOCK );
  fcntl(0, F_SETOWN, getpid());
  alarm(1);  // si on veut simuler le timeout de reader
}

void remont_CS_PLL() {}

void sortie_led(int led) {}

void sortie_antenne(int antenne) {}

void set_serial_speed(int a,int b) {}

void set_DAC0(int i) {}

void set_DAC1(int i) {}

void set_SPI_speedDiv(int i) {}

void set_DAC2(int i) 
{pc_offset=(i>>16);
// printf("pc_offset=%d\n",pc_offset);
}

void sortie_courant(int val)
{//printf("courant=%d\n",val);
}

void sortie_courant_init() {}

void init_PLL() {}
void init_DDS() {}
void kick_watchdog() {}
void dephase(unsigned short phase,unsigned char chan) {}

char read_switch() {return(1);}
