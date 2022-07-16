#include <stdio.h>
#include <stdlib.h>
#include "rs232.h"

#define DEBUG printf
// #define DEBUG nothing

int main(int argc,char **argv)
{int fd,cnt=0;FILE *f;
 unsigned char cmd;
    fd=init_rs232(); 
    while (1) {
//       for (cnt=0;cnt<3;cnt++) 
            {read(fd,&cmd,1);
             DEBUG("(%x) ",(cmd&0xff));
            }
	    printf("\n");fflush(stdout);
	   }
}
