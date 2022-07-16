#include<ADuC7026.h>
int ther, testtherapie=0, noimagerie=0;
 
#define NB_CHARS 10
unsigned char global_tab[NB_CHARS];
int global_index=0,tim0=0;
int dtherapie=0,ntherapie=0;
int nbre_therapie=4;
int mask_therapie=0x1f;
int dure_therapie=30;
int attente=0, periode =0, periode2=0, resultat=0,test =0, temps=0;
int freqreel = 0;
int loop=0;
void delay (int length) {
	while (length >=0) length--;
}
// int putchar(int ch)  {               // Write character to Serial Port  
int putchar(int ch)  {                  // Write character to Serial Port  
  do {} while ((COMSTA0 & 0x020)!=0x20);
  COMTX = ch;
  return (1);
}

// int getchar (void)  {  // lecture non bloquante du char : 0 signifie rien 
void writeASC( short *ptr,int len)
{int i;
 unsigned char b;
 for (i=0;i<len;i++)
 {b=((ptr[i]&0xf000)>>12);
  if (b<10) putchar(b+48); else putchar(b+55);
  b=((ptr[i]&0x0f00)>>8);
  if (b<10) putchar(b+48); else putchar(b+55);
  b=((ptr[i]&0x00f0)>>4);
  if (b<10) putchar(b+48); else putchar(b+55);
  b=((ptr[i]&0x000f));
  if (b<10) putchar(b+48); else putchar(b+55);
//  jmf_putchar(32);
 }
}


// int write(unsigned char * ptr, int len) {
int jmf_write( char * ptr, int len) {
  int i;
  for (i = 0; i < len; i++) putchar (*ptr++);
  return len;
}


// void IRQ_Handler() __irq 
void IRQ_Handler(void) __irq 
{if (IRQSIG & UART_BIT)  {        // UART Interrupt      
   }
 if (IRQSIG & XIRQ2_BIT)  {    //IRQ EXT 0      
	jmf_write("XIRQ=",5);jmf_write("\r\n",2);
	periode = T1CAP;
	resultat = periode2 - periode;
	jmf_write("FREQ=",5);writeASC(&resultat,1);jmf_write("\r\n",2);
        periode2=periode;
		}
 }
 if (IRQSIG & GP_TIMER_BIT)  {    // Timer1 Interrupt      
    tim0=1;
    T1CLRI = 1;                   // Clear Timer 1 interrupt
  }

/* mesure de temperature */
/*
 if (IRQSIG & MONITOR_BIT)  {    //IRQ Comparator  
 	periode2=T1CAP;
 	resultat = periode2 - periode;
	delay(100);
	//temps=resultat / 41780000;
	//jmf_write("Periode=",8);writeASC(&periode,1);jmf_write("\r\n",2);
	//jmf_write("Periode2=",9);writeASC(&periode2,1);jmf_write("\r\n",2);
	jmf_write("TEMPS=",6);writeASC(&resultat,1);jmf_write("\r\n",2);
    IRQCLR = MONITOR_BIT; 

 }
*/
}


int main(void) {
	int k,etat=0xf0ff;
	unsigned char sec=0,dsec=0;

#ifdef spimux 
	GP1CON = 0x22020011;	// configure SPI on P1.4-P1.7, and uart tx & rx pins on P1.0 and P1.1 
	SPIDIV = 0x06;		// set SPI clock 40960000/(2x(1+SPIDIV)) 
	SPICON = 0x1043;	// ena SPI master in continuous transfer mode  
#else 
        GP1CON=0x00000011;    	// setup tx & rx pins on P1.0 and P1.1 
 //       GP1DAT=0x52000000;
#endif 
	GP3DAT = 0xff000000; 	// OE0 = P3.0 / OE1 = P3.1 / OE2 = P3.2 / OE3 = P3.3 / OE4 = P3.4 / OE5 = GND / OE6 = P3.5 / OE7 = P3.6 / OE8 = P3.7
	GP4DAT = 0x8F000000;	// P4.[2-4] en sortie, P4.[3,4] = 1 & P4.2 = 0
	GP0DAT = 0x00000000;
	GP2DAT = 0xff100000;	// P2.0-7 en sortie, initialises a 0; 

// Start setting up UART at 19200 bps
   	COMCON0 = 0x080;	// Setting DLAB
   	COMDIV0 = 0x88/4;         // 22; pour 57600 // Setting DIV0 and DIV1 to DL calculated 0x88=9600, 0x44=19200
   	COMDIV1 = 0x000;	// *** RESET DE LA CARTE ***
   	COMCON0 = 0x007;	// Clearing DLAB

	CMPCON = 0x4C0;		// Interrupt comparator
    	COMIEN0=1;
	T1CON=0x2D180;   // external xtal actif, pas de division  //    	IRQ = IRQ_Handler;   
	//IRQEN = UART_BIT | GP_TIMER_BIT |MONITOR_BIT;   // timer 0 & 2  RTOS_TIMER_BIT 
	while (1) 
       	  {
		  GP3DAT = 0xff000000; 
 		  periode = T1VAL;
		  IRQEN = UART_BIT | GP_TIMER_BIT | XIRQ2_BIT;   
		  delay(100);

	       }
 
}
