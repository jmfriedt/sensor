// programme de test de modbus

#include "modbus.h"
#include "DDSinc.h"
#include "version.h"
#include "modbus-private.h"
//#include <stdlib.h>
//#include <string.h>
//#include <racine/racine.h>
#include <time.h>

//#include<ADuC7026.h>

int nbpics[2];
unsigned char ACR[4] = { 0x06, 0x00, 0x13, 0xFF };
unsigned char FTW0[5] = { 0x04, 0x31, 0x00, 0x70, 0xA3 };

unsigned char global_tab[NB_CHARS];
volatile int global_index = 0, tim0 = 0, heure = 0, minute = 0, seconde = 0;
volatile int montim0 = 0;
volatile unsigned int mont1 = 0, mont2 = 0;
volatile unsigned int GLOBAL_TIMER1 = 0;
#ifdef periodique
volatile int reveil = 0;
#endif

void
write_str (char *pString)
{
  while (*pString != 0x00)
    jmf_putchar (*pString++);
}

void
writeDECs(unsigned short d)
{
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDECc(unsigned char d)
{
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else				// if (sup!=0) 
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDEC0i(int d)
{
  if (d < 0)
    {
      jmf_putchar ('-');
      d = -d;
    }
  if (d > 99999)
    d = 99999;
  if (d > 9999)
    {
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else
    jmf_putchar (48);
  if (d > 999)
    {
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else
    jmf_putchar (48);
  if (d > 99)
    {
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeDECi(int d)
{
  int sup = 0;
  if (d < 0)
    {
      jmf_putchar ('-');
      d = -d;
    }
  if (d > 999999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000000) + 48);
      d = d - (d / 1000000000) * 1000000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99999999)
    {
      sup = 1;
      jmf_putchar ((d / 100000000) + 48);
      d = d - (d / 100000000) * 100000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9999999)
    {
      sup = 1;
      jmf_putchar ((d / 10000000) + 48);
      d = d - (d / 10000000) * 10000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 999999)
    {
      sup = 1;
      jmf_putchar ((d / 1000000) + 48);
      d = d - (d / 1000000) * 1000000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99999)
    {
      sup = 1;
      jmf_putchar ((d / 100000) + 48);
      d = d - (d / 100000) * 100000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9999)
    {
      sup = 1;
      jmf_putchar ((d / 10000) + 48);
      d = d - (d / 10000) * 10000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 999)
    {
      sup = 1;
      jmf_putchar ((d / 1000) + 48);
      d = d - (d / 1000) * 1000;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 99)
    {
      sup = 1;
      jmf_putchar ((d / 100) + 48);
      d = d - (d / 100) * 100;
    }
  else if (sup != 0)
    jmf_putchar (48);
  if (d > 9)
    {
      jmf_putchar ((d / 10) + 48);
      d = d - (d / 10) * 10;
    }
  else if (sup != 0)
    jmf_putchar (48);
  jmf_putchar (d + 48);
}

void writeHEXi(unsigned int ptr)
{
  unsigned char b;
  {
    b = ((ptr & 0xf0000000) >> 28);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0f000000) >> 24);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00f00000) >> 20);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000f0000) >> 16);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0000f000) >> 12);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00000f00) >> 8);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000000f0) >> 4);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0000000f));
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    jmf_putchar (32);
  }
}

void writeHEXs(unsigned short ptr)
{
  unsigned char b;
  {
    b = ((ptr & 0xf000) >> 12);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x0f00) >> 8);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x00f0) >> 4);
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    b = ((ptr & 0x000f));
    if (b < 10)
      jmf_putchar (b + 48);
    else
      jmf_putchar (b + 55);
    jmf_putchar (32);
  }
}

void writeHEXc(unsigned char ptr)
{
  unsigned char b;
  b = ((ptr & 0x00f0) >> 4);
  if (b < 10)
    jmf_putchar (b + 48);
  else
    jmf_putchar (b + 55);
  b = ((ptr & 0x000f));
  if (b < 10)
    jmf_putchar (b + 48);
  else
    jmf_putchar (b + 55);
  jmf_putchar (32);
}

/*
int main(void)
{
  
  // main requetes modbus
  //int recu = 0;
  int k=0;
  modbus_t mb;
  unsigned short tab_reg[32];
  struct timeval response_timeout;
  
  init_CPU ();
  delay(0xffff);
  clignote();

  modbus_new_rtu(&mb, "hello", 38400, 'N', 8, 1);
  modbus_set_debug(&mb, FALSE); // TRUE
  response_timeout.tv_sec = 1;
  response_timeout.tv_usec = 0;
  modbus_set_response_timeout(&mb, &response_timeout);
  modbus_set_slave (&mb, 1);
  modbus_connect (&mb);
      
  modbus_read_registers(&mb, 2-1, 4, tab_reg);
  delay (0xffff);
  modbus_write_register(&mb,14-1,(0x0000));
    
  while (1) {
     for (k=0;k<20000;k+=100){
         modbus_write_register(&mb, 15 - 1, k);
     	 clignote();
	 delay (0xffff);delay (0xffff);
         }
      }
    if (tim0 > 99999)	tim0 = 0;
}
*/

//-------------------------------------------------------------------------------------------------------
// ARM esclave

int main(void)
{
  modbus_t ctx;
  int i;
  modbus_mapping_t *mb_mapping;
    
  init_CPU ();
  modbus_new_rtu(&ctx, "hello", 38400, 'N', 8, 1);
  modbus_set_debug(&ctx, FALSE);
  modbus_set_slave(&ctx, 1);
  modbus_connect(&ctx);
  mb_mapping = modbus_mapping_new(50, 50, 50, 50);
  if (mb_mapping == NULL) {
      write_str ("Failed to allocate the mapping");
      modbus_free(&ctx);
      while(1){}
    }
  for (i=0;i<10;i++) mb_mapping->tab_registers[i]=i;
  for (;;) {
      uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
      int rc;
      rc = modbus_receive(&ctx, query);
	
      if (rc != -1) {           // rc is the query size 
            modbus_reply(&ctx, query, rc, mb_mapping);clignote();
        } 
    }

  write_str ("Quit the loop");// modbus_strerror(errno));

  modbus_mapping_free(mb_mapping);
  // modbus_close(&ctx);
  // modbus_free(&ctx);
  return 0;
}
//-------------------------------------------------------------------------------------------------------
void
_kill () {}
void _getpid () {}
void _exit () {}
