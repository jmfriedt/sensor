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

int main(int argc, char *argv[])
{
  modbus_t *mb;
  uint16_t tab_reg[32],k,l=0;

  struct timeval response_timeout;
  mb = modbus_new_rtu("/dev/ttyUSB0", 38400, 'N', 8, 1);
  modbus_set_debug(mb, TRUE);

  response_timeout.tv_sec = 1;
  response_timeout.tv_usec = 0;
  modbus_set_response_timeout(mb, &response_timeout);

  printf("set_slave: %d\n",modbus_set_slave(mb, 1));
  printf("connect: %d\n",modbus_connect(mb));

  modbus_read_registers(mb, 1001-1, 8, tab_reg);    // lecture registre resultats
  for (k=0;k<8;k++) printf("%d:%x ",k,(tab_reg[k]));
  printf("\n");
  modbus_write_register(mb,1004-1,(0x66bb+l));      // ecriture registre resultats
  modbus_read_registers(mb, 1001-1, 8, tab_reg);    // lecture registre resultats
  for (k=0;k<8;k++) printf("%d:%x ",k,(tab_reg[k]));
  printf("\n");
 
  modbus_read_registers(mb, 100-1, 12, tab_reg);    // lecture A0 qui depasse sur A1 puis A2
  for (k=0;k<12;k++) printf("%d:%x ",k,(tab_reg[k]));
  printf("\n");
  modbus_write_register(mb,104-1,(0x66bb+l));       // ecriture coef cal
  l++;
  modbus_read_registers(mb, 104-1, 8, tab_reg);     // lecture sur A1 qui depasse sur A2
  for (k=0;k<8;k++) printf("%d:%x ",k,(tab_reg[k]));
  printf("\n");
  modbus_read_registers(mb, 105-1, 4, tab_reg);     // lecture au milieu de A1
  for (k=0;k<4;k++) printf("%d:%x ",k,(tab_reg[k]));
  printf("\n");
/*
Opening /dev/ttyUSB0 at 38400 bauds (N, 8, 1)
connect: 0
[01][03][03][E8][00][08][C4][7C]
Waiting for a confirmation...
<01><03><10><00><65><00><66><00><67><66><BB><00><69><00><6A><00><6B><00><6C><18><C9>
0:65 1:66 2:67 3:66bb 4:69 5:6a 6:6b 7:6c 
[01][06][03][EB][66][BB][92][69]
Waiting for a confirmation...
<01><06><03><EB><66><BB><92><69>
[01][03][03][E8][00][08][C4][7C]
Waiting for a confirmation...
<01><03><10><00><65><00><66><00><67><66><BB><00><69><00><6A><00><6B><00><6C><18><C9>
0:65 1:66 2:67 3:66bb 4:69 5:6a 6:6b 7:6c 
[01][03][00][63][00][0C][B5][D1]
Waiting for a confirmation...
<01><03><18><11><22><33><44><55><66><77><88><66><BB><BB><CC><DD><EE><FF><00><DD><00><CC><11><EE><22><AA><33><E7><7C>
0:1122 1:3344 2:5566 3:7788 4:66bb 5:bbcc 6:ddee 7:ff00 8:dd00 9:cc11 10:ee22 11:aa33 
[01][06][00][67][66][BB][53][C6]
Waiting for a confirmation...
<01><06><00><67><66><BB><53><C6>
[01][03][00][67][00][08][F5][D3]
Waiting for a confirmation...
<01><03><10><66><BB><BB><CC><DD><EE><FF><00><DD><00><CC><11><EE><22><AA><33><EC><FE>
0:66bb 1:bbcc 2:ddee 3:ff00 4:dd00 5:cc11 6:ee22 7:aa33 
[01][03][00][68][00][04][C5][D5]
Waiting for a confirmation...
<01><03><08><BB><CC><DD><EE><FF><00><DD><00><40><C2>
0:bbcc 1:ddee 2:ff00 3:dd00 
*/

  while (1) {
    modbus_read_registers(mb, 1000-1, 4, tab_reg);    // lecture registre resultats
    for (k=0;k<4;k++) printf("%d:%x ",k,(tab_reg[k]));
    printf("\n");
  }
  modbus_close(mb);
  modbus_free(mb);

  return 0;
}
