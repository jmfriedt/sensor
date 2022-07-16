// gcc -I/usr/local/include/modbus/ -c demo_DAT3024_sur_pc.c
// libtool --tag=CC --mode=link gcc -O2 -Wall -Werror -o demo_DAT3024_sur_pc demo_DAT3024_sur_pc.o /usr/local/lib/libmodbus.la

/*
1/ il ne faut PAS mettre le 4 en debut de registre, ca veut dire "lire"
2/ retrancher 1 aux numeros de registres car la doc commence en 1 alors que modbus commence en 0
   https://groups.google.com/forum/?fromgroups#!topic/libmodbus/oZ3d_n3ucCI
3/ changer l'endianness des mots quand on les lit sur PC
4/ la broche 3 du PC (TX) soit sur la broche du milieu du connecteur du DAT3024 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include "../../../config_modbus.h"

#define BASE 1001

int main(int argc, char *argv[])
{
  modbus_t *mb;
  uint16_t tab_reg[32],k,l,off=0x100;
  unsigned int *i;
  float *f;

  struct timeval response_timeout;
  mb = modbus_new_rtu("/dev/ttyS0", 19200, 'N', 8, 1);
  modbus_set_debug(mb, TRUE);

  response_timeout.tv_sec = 5;   // timeout=5 secondes
  response_timeout.tv_usec = 0;
  modbus_set_response_timeout(mb, &response_timeout);

  printf("set_slave: %d\n",modbus_set_slave(mb,1)); //  MODBUS_BROADCAST_ADDRESS));
  printf("connect: %d\n",modbus_connect(mb));

while (1) {
//  for (l=0;l<2;l++) {         // 2 mesures
    modbus_read_registers(mb, BASE-1, NBPICS_MAX*ANTENNES_MAX/2, tab_reg);
    printf("\n");
    for (k=0;k<NBPICS_MAX*ANTENNES_MAX/2;k++) printf("%d:%04x ",k,htons(tab_reg[k]));
    printf("\n-> ");
    i=(int*)tab_reg; *i=htonl(*i); 
    f=(float*)i;
    printf("%x %f",*i, *f);          // affiche la T en float
    printf("\n");
//    usleep(100000);
//  }

*f=123.4;
printf("%08x\n",i);
 return(1); 

  modbus_read_registers(mb, 101-1, 4, tab_reg);
  printf("\n");
  for (k=0;k<4;k++) printf("%d:%x ",k,htons(tab_reg[k]));
  i=(unsigned int*)tab_reg;
  printf("\nconversion int: %x %x\n",htonl(i[0]),htonl(i[1]));
  printf("\n");
  usleep(10000);
  
  modbus_write_register(mb,101-1,htons(0x55aa)); // coefs cal
  modbus_write_register(mb,102-1,htons(0x2233)); // coefs cal
  modbus_read_registers(mb, 101-1, 8, tab_reg);
  printf("\n");
  for (k=0;k<8;k++) printf("%d:%x ",k,htons(tab_reg[k]));
  printf("-> htonl=%x\n",htonl(i[0]));
  usleep(10000);

tab_reg[0]=htons(0x1122);
tab_reg[1]=htons(0x3344);
tab_reg[2]=htons(0x5566);
  printf("config: %d\n",modbus_write_registers(mb,1016-1,3,tab_reg)); 
  modbus_read_registers(mb, 1016-1, 3, tab_reg); // offset
  printf("\n");
  for (k=0;k<3;k++) printf("%d:%04x ",k,htons(tab_reg[k]));
  printf("\n");

  off=0xABC;
  modbus_write_register(mb,REG_HOLDING_START+NBPICS_MAX*ANTENNES_MAX*(REG_PAR_RESONANCE+1)-1,htons(off)); // offset
  modbus_read_registers(mb,REG_HOLDING_START+NBPICS_MAX*ANTENNES_MAX*(REG_PAR_RESONANCE+1)-1, 3, tab_reg); // offset
  printf("\n");
  for (k=0;k<3;k++) printf("%d:%04x ",k,htons(tab_reg[k]));
  printf("\n");

// ATTENTION : ICI ON NE PEUT CHANGER QUE 1 PARAMETRE, APRES LA LIAISON EST PERDUE
  modbus_write_register(mb,REG_CONFIG+1-1,htons(57600)); // rs_baudrate
//  modbus_write_register(mb,REG_CONFIG-1,htons(2)); // slave

exit(1);
  sleep(1);
}
// la seule difference entre ffff et 0000 ici est la gamme : 0-10 mA ou 0-20 mA,
// mais dans tous les cas la sortie en tension ET en courant sont toutes deux actives
  //printf("config: %d\n",modbus_write_register(mb,14-1,(0x1234))); // 1234 pour tester ADuC
/*
  printf("config: %d\n",modbus_write_register(mb,14-1,(0x0000))); // 1=V, 0=I
  usleep(10000);
  modbus_read_registers(mb, 14-1, 1, tab_reg);
  printf("%x\n",(tab_reg[0]));
  printf("config: %d\n",modbus_write_register(mb,12-1,(0xCCCC))); // immediate
  usleep(10000);
  sleep(1);
*/

/*
  while (1)
     for (k=0;k<10000;k+=100) {
       printf("V%d: %d ",k,modbus_write_register(mb,15-1,k));usleep(100);
       printf("V%d: %d ",k,modbus_write_register(mb,16-1,k));usleep(100);
       printf("V%d: %d ",k,modbus_write_register(mb,17-1,k));usleep(100);
       printf("V%d: %d ",k,modbus_write_register(mb,18-1,k));usleep(100);
      }
*/
  modbus_close(mb);
  modbus_free(mb);

  return 0;
}
