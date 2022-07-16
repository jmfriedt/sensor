/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "init.h"

#define NB_CHARS 10    // taille du tampon pour la communication RS232

unsigned char global_tab[NB_CHARS];
volatile char init_status=0;

#include "config_modbus.h"

extern USHORT   usRegInputStart;
extern USHORT   usRegInputBuf[REG_INPUT_NREGS];
extern USHORT   usRegHoldingStart;
extern USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];

volatile int tim1=0;

int main( void )
{ int i;
  eMBErrorCode    eStatus;
  init_CPU();

        if( ( eStatus = eMBInit( MB_RTU, 0x01, 0, 38400, MB_PAR_EVEN ) ) != MB_ENOERR ) {}
        else {if( ( eStatus = eMBEnable(  ) ) != MB_ENOERR ) {}
        else
           { UCHAR tab[8]={0xff,0x06,0x00,0x01,0x55,0xA1,0x00,0x00}; //{0x01,0x03,0x0,0x2,0x0,0x1,};
	     UCHAR lire[8]={0xff,0x03,0x00,0x01,0x00,0x03,0x00,0x00};
	// tableau {Numero d'esclave automatique / 03 pour lire 06 pour ecrire /
	// Numero du registre haut / Numero du registre bas /
	// donnee a envoyer dans le cas ecriture sinon nombre d'octet /crc qui est calcule' plus loin}

             init_status=1;
	//   tim1=0;
             for (i=0;i<10;i++) {usRegInputBuf[i]=i;usRegHoldingBuf[i]=i;}
	     for(;;){   
                 ( void )eMBPoll(  );
	         if(tim1>10) {tim1=0;clignote();
//	           eMBRTUSend(0x01 , &tab[1], 5 );
//	           eMBRTUSend(0x01 , &lire[1], 5 );
// 		 eMBRegInputCB(tab, 16,2);
// 		( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
	    }
           }
	  }}
}

void
write_str (char *pString,char* chaine,int* chaine_index)
{
  while (*pString != 0x00)
    jmf_putchar (*pString++);
}
