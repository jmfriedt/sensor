// 1/ connecter USBA-RS232-RS485 a USB pour creer /dev/ttyUSB0
// 2/ conencter USBA-USBminiB a UBS pour creer /dev/ttyUSB1
// 3/ flasher le firmware par stm32flash -w DDS.bin /dev/ttyUSB1
// 4/ lancer minicom 38400 bauds, 8N1, /dev/ttyUSB1 -> rien ne s'affiche
// 5/ lancer freemodbus-v1.5.0senseor/demo/ARM/demo_DAT3024_sur_pc -> transactions modbus +

/*
RS232
002 000000000 04045 027 00000 000.0 000000000 04037 027 00000 00424 000
002 000000000 04045 023 00000 000.0 000000000 04023 023 00000 00426 000
002 000000000 04044 019 00000 000.0 434335920 03635 021 00077 00429 000
002 000000000 04042 015 00000 000.0 434336032 03481 019 00156 00430 000
002 433781648 02946 015 00006 228.1 434337168 02639 021 00121 00446 108
002 433781760 02946 015 00041 000.0 000000000 02630 023 99999 00447 108
002 433781824 02953 015 00009 228.3 434335904 02896 024 00002 00448 108
002 433781360 02957 015 00004 000.0 000000000 02733 025 99999 00449 108
002 433781024 03754 012 00002 227.9 434338080 03545 023 00197 00464 108
002 433780272 03517 010 00011 228.0 434336416 03530 021 00003 00465 108
002 433780880 03242 009 00008 227.9 434337712 03407 020 00091 00466 108
002 433780768 03127 009 00001 228.0 434337248 03318 019 00058 00467 108

Modbus (uniquement les trames recues, j'elimine les trames emises)
0:069c 1:ac24 2:09b0 3:001f 4:0004 5:08e1 6:0708 7:f058 8:0766 9:001f 10:0181 11:006f 
0:069c 1:ac24 2:09b0 3:0017 4:0004 5:08e1 6:0708 7:f058 8:0766 9:0017 10:0181 11:006f 
0:069c 1:ac24 2:09b0 3:000f 4:0004 5:08e1 6:0707 7:8c68 8:0d99 9:0013 10:009c 11:006f 
0:069c 1:b5e4 2:0b82 3:000f 4:0029 5:08e9 6:0707 7:c484 8:0a4f 9:0017 10:0079 11:006f 
0:069c 1:a2b8 2:0b8d 3:000f 4:0004 5:08eb 6:0707 7:85f4 8:0b50 9:0019 10:0002 11:006f 
0:069c 1:6cd0 2:0dbd 3:000a 4:000b 5:08e8 6:0707 7:9f84 8:0dca 9:0015 10:0003 11:006f 
  ^^^^^^^^^^^
01:0x069cxxxx~33778 kHz en prenant ((frequence>>4)*155)/1958)*16+25000000
2 :0x9b0=2480 = puissance recue
3 : 0x0f= 15 = puissance emise (j'ai change' volontairement l'attenuateur au cours de la mesure)
4 : 0x29= 41 = variance
5 :0x8e9=2281= temperature en 1/10 degres (n'est pas affiche' en RS232 a cause de variance sur pic2)
6: on recommence pour le second pic, 0x707xxxx=34.333 MHz
etc ...
*/

// PENSER A make clean && make en changeant cette config (.h n'est pas dans les dependances du Makefile)

#include "DDSinc.h"
 
#define REG_AT_START (REG_HOLDING_START+NBPICS_MAX*ANTENNES_MAX*(REG_PAR_RESONANCE+1))
#define CMDS_AT (5)          // nombre de registres requis pour AT

#define REG_INPUT_START 1001
#define REG_INPUT_NREGS 1
//#define REG_INPUT_NREGS (NBPICS_MAX*ANTENNES_MAX*8) 

// 2 registres/mesure + puissance emise + puissance recue + variance + phase + offset
// 0 = freq poids fort
// 1 = freq poids faible
// 2 = hpic
// 3 = puissance emission / attenuation reception
// 4 = variance
// 5 = phase
// 6 = offset
#define REG_PAR_RESONANCE 7
#define REG_HOLDING_START 1001  // registre 1001=adresse 1000
#define REG_HOLDING_NREGS (NBPICS_MAX*ANTENNES_MAX*(REG_PAR_RESONANCE+1)+CMDS_AT)	// REG_PAR_RESONANCE+1 pour temperature en debut de registre

// registres des coefs de cal
#define REG_COEF_START 101
#define REG_CONFIG     501

#ifdef MODBUS
#define modbus_port 2
#define usart_port  1
#else
#define modbus_port 2
#define usart_port  1
#endif

#define PROPORTIONAL_INIT 3 // nombre de mesures-1 realisees lors de la requete MODBUS
#define MODBUS_SLAVE_ADD 1  // 15 capteurs (B) = @2, 6 capteurs (A) = @1
