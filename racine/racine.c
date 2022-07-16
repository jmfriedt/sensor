#include <stdio.h>
#include <stdlib.h>
#include <racine/racine.h>
#include <config_modbus.h>

long jm_sqrt(long A)
{long x,xp; // ,n=0;
 {if (A==0) return(0);
  if (A==1) return(1); // pourquoi Newton ne marche pas en 1 ?
  xp=A>>1;             // initialisation : A/2
  do {
//writeDECi(xp,NULL,NULL,usart_port);
//jmf_putchar(' ',NULL,NULL,usart_port);
    x=xp;
    if (x!=0) xp=(x+A/x)>>1;   // x(n+1)=1/2*(x(n)+A/x(n))
    else break;
//   n++;
  } while ((x-xp)>1);
 }
// printf("n=%d:\t",n);
 return(xp);
}
