#include <stdio.h> 
#include <stdlib.h> 
#include <math.h> 

unsigned int jm_sqrt(unsigned int A)
{unsigned short x,xp,n=0;
 xp=A>>1;           // initialisation : A/2
 do {
   x=xp;
   xp=(x+A/x)>>1;   // x(n+1)=1/2*(x(n)+A/x(n))
   n++;
 } while (abs(xp-x)>1);
// printf("n=%d:\t",n);
 return(xp);
}

int main(int argc,char **argv)
{unsigned int A=100000,sA,k;
 float fA0=-250.4;      // 0.1 pres
 float fA1=160000.66;   // % 0.01 pres
 float fA2=-0.17212345; // % 1e-4 pres
 float fdf=500000.;
 float fsA;
 unsigned int A0=(unsigned int)(fA0*10.);
 unsigned int A1=(unsigned int)(fA1*10000.);
 unsigned int A2=(unsigned int)(fA2*100000.);
 unsigned int df=(unsigned int)(fdf/10.);

 if (argc>1) A=atoi(argv[1]);
 sA=jm_sqrt(A);
 fsA=sqrt((float)A);
 printf("sqrt(%d)=%d %d\r\n",A,sA,fsA-sA);
 printf("T=%f=%d\r\n",fA0+sqrt(fA1+fA2*fdf),A0+jm_sqrt(A1+A2*df)/10);
}
