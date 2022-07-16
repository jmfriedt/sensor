#include "DDSinc.h"

//----------------------------------------------------------------------------//
int convertit_temperature=0; // affiche la temperature en degC, exploitant A0..A2
// exploitation de bandes de frequences predefinies (1) ou recherche
//   de NBPIPCS dans la bande fstart[0]..fstop[NBPICS-1] (0), modif par AT=
int bandes=1;  // 0=pas de bande predefinie, ou 1=exploitation des bandes
               // -> NBPICS*NBSTEPS mesures

// choix du mode de mesure (2points ou fixe/3points), modifiable par AT!
unsigned char deux_points=0; // 0=fixe/3points, 1=2 points

// choix du mode de detection peigne fixe/3 points, modifiable par AT3
unsigned char mode_balayage_continu=0; // 0=3 points, 1=peigne fixe

//-----------------------------------------------------------------------------/
int LBT=0;	        // interroge que si la bande est silencieuse, 0/1 (ATL)
unsigned char agc=1;    // activer (1) controle de gain automatique, 0/1 (ATZ)
unsigned char trame_minimale=0; // virer les messages inutiles si trame_minimale==1

int ATTENTE=0; 	        // attente entre deux emissions	(0..65536)   
                        // 2000 = 1.1 ms entre deux emissions, 2800=1.55 ms
int HATTENTE=1;         // entre fin d'emission et conversion (0..65536)

//----------------------------------------------------------------------------//
//      CHOIX DU TYPE DE DETECTION:
//----------------------------------------------------------------------------//
// 2 types possibles: 
//              -> Détection de tous les resonateur en meme temps par la 
//                   fonction balaie_ism #define detec_tous
//              -> Sinon detection des resonateur un par un (permet d'avoir 
//                   plus de resonateurs)   

/**************************** CAS DES BORNES AUTOMATIQUES *********************/
#ifndef manuel    // cas des bornes equidistantes calculees automatiquement
int NBMOY=(2);    // DOIT etre inferieur ou egal a NBMOY_MAX
int NBPICS=(2);   // DOIT etre inferieur ou egal a NBPICS_MAX
int NBSTEPS=(64); // NBSTEPS tq pas de freq=largeur mi-hauteur/3=f0/Q/3 
                  //         Min 26 steps avec Ampl:500 pour 36 db
int ANTENNES=(1); // DOIT etre inferieur ou egal a ANTENNES_MAX

#ifndef f400      // cas de l'oscillateur du DDS a 200 MHz (habituel)

//unsigned int FSTART =(0x2a2d0e00); // 432.95 jerome 130218
//unsigned int FSTOP  =(0x2bf7cf00); // 434.35 jerome 130218
//    unsigned int FSTART =(0x27ced900); // 431.1 silmach
//    unsigned int FSTOP  =(0x28f5c300); // 432.7 silmach  nbpics=2  milieu = 
//unsigned int FSTART =(0x270a3d00); // LMD 32.57
//unsigned int FSTOP  =(0x29999a00); // LMD 34.57
//unsigned int FSTART =(0x29b08A00); // LMD 32.57
//unsigned int FSTOP  =(0x2c3fe600); // LMD 34.57
//unsigned int FSTART =(0x26666700); // 430.0
//unsigned int FSTOP  =(0x3ffff000); // 450.00
//unsigned int FSTART  =(0x2f5c3000); // 437.0  // beanair 121123
//unsigned int FSTOP   =(0x347AE200); // 441.0  // beanais 121123
// unsigned int FSTOP   =(0x370a3d00); // 443.0
//unsigned int FSTART =(0x27ae1500); // 431.0
//unsigned int FSTART  =(0x28f5c300); // 432.00 // BMCI .. 435

//unsigned int FSTOP =(0x2E147B00); // 436 // damien LMA : 2 moy, 1 pic
//unsigned int FSTART=(0x2CCCCD00); // 435

unsigned int FSTART=(0x2A4DD300);  // 433.05     SEAS10
unsigned int FSTOP  =(0x2ccc0000); // 435.00 MHz SEAS10
//unsigned int FSTOP =(0x2b851f00); // 434.000 Siemens couple avec debut a 433

//unsigned int FSTART=(0x26667000); // 430.00 MHz SAWHOT : bande = 0, NBSTEPS_MAX=808, NBSTEPS=800
//unsigned int FSTOP =(0x39900000); // 443.00 MHz SAWHOT

//unsigned int FSTART=(0x29AD4300);  // 432.56     siemens1
//unsigned int FSTOP  =(0x2c3c9f00); // 434.56 MHz siemens1
//unsigned int FSTART=(0x28D4FE00);  // 431.90     siemens2
//unsigned int FSTOP  =(0x2CEA4B00); // 435.09 MHz siemens2
//unsigned int FSTART=(0x29d7dc00);  // 432.69     shiso
//unsigned int FSTOP  =(0x2c673800); // 434.69 MHz shiso

//unsigned int FSTART=(0x29fbe800); // 432.80 irlande
//unsigned int FSTOP =(0x2c91d100); // 434.82 MHz
//unsigned int FSTART =(0x2Ac08300); // 433.400 Schrader 12/10/2011
//unsigned int FSTOP  =(0x2c2f8400); // 434.520 Schrader 12/10/2011
//unsigned int FSTART =(0x2a8c1500); // 433.400 Schrader 12/03/2012
//unsigned int FSTOP  =(0x2c01a400); // 434.520 Schrader 12/03/2012
//unsigned int FSTOP   =(0x2d70a400); // 435.50 MHz
//unsigned int FSTART =(0x2872b000);   // 431.6
//unsigned int FSTOP   =(0x2b020c00);  // 433.60 MHz
//unsigned int FSTART =(0x2a9fb300); // 433.
//unsigned int FSTOP  =(0x370a3d00); // 443.00 MHz 
//unsigned int FSTOP  =(0x3999a000); // 445.00 MHz 
//unsigned int FSTOP   =(0x30a3d700); // 438.00 MHz 
//unsigned int FSTOP   =(0x2f5c2900); // 437.00 MHz 
//unsigned int FSTOP  =(0x31eb8500); // 439.00 MHz 
//unsigned int FSTOP  =(0x3f5c2900); // 439.00 MHz 
#else             // cas de l'oscillateur du DDS a 400 MHz (schrader SAWHOT) 
unsigned int FSTART =(0x2A4DD400/2); // SEAS10
unsigned int FSTOP  =(0x2c96d000/2); // SEAS10
//unsigned int FSTART =(0x27ae1500/2); // 431.0 silmach
//unsigned int FSTOP  =(0x28f5c300/2); // 432.7 silmach  nbpics=2
#endif
// dans le cas #undef manual, on remplira manuellement ces tableaux dans main()
unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]; // ne pas toucher
unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]; // ne pas toucher

/**************************** CAS DES BORNES MANUELLES ************************/
#else             // cas #ifdef manuel : definition manuelle des bornes de freq
int NBMOY=1;      // DOIT etre inferieur ou egal a NBMOY_MAX
int NBPICS=(3);   // DOIT etre inferieur ou egal a NBPICS_MAX
int NBSTEPS=(32); // NBSTEPS tq pas de freq=largeur mi-hauteur/3=f0/Q/3  
int ANTENNES=(1);   // inferieur ou egal a ANTENNES_MAX (Nombre d'antenne balayer)

#ifndef f400      // cas de l'oscillateur du DDS a 200 MHz (habituel)
// jay 121129 plastic
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2A7EF9DB,0x2BA5E353,0x2CCCCCCC,0x2E147AE1,0x2F5C28F5,0x30A3D70A,0x31EB851E,0x33333333,0x347AE147,0x35C28F5C,0x370A3D70,0x3851EB85}; // 433,2 - 434,1 - 435,0 - 436,0 - 437,0 - 438,0 - 439,0 - 440,0 - 441,0 - 442,0 - 443,0 - 444,0
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2BA5E353,0x2CCCCCCC,0x2E147AE1,0x2F5C28F5,0x30A3D70A,0x31EB851E,0x33333333,0x347AE147,0x35C28F5C,0x370A3D70,0x3851EB85,0x39999999}; // 434,1 - 435,0 - 436,0 - 437,0 - 438,0 - 439,0 - 440,0 - 441,0 - 442,0 - 443,0 - 444,0 - 445,0

// Fintsta contient autant de frequences de debut de plage que ANTENNES*NBPICS
// Fintsto contient autant de frequences de fin de plage que ANTENNES*NBPICS

// unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2ac08300,0x2b22d100,0x2bac7100}; // shrader
// unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2b22d100,0x2bac7100,0x2c2f8400}; // schrader

// Sergei/SAWHOT : 128 points
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x26666700,0x2e147b00};
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2e147b00,0x30a3d700};
                                             // 

// Jerome 432.9-433.6=SSE/E017 433.6-434.6=SSE/E015 ; 437.0-437.8=TSME153 437.8-439.0=TSME153
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2a1cac00,0x2b020c00,0x2f5c2900,0x30624e00};
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2b020c00,0x2c49ba00,0x30624e00,0x31eb8500}; 

// Alstom Madrid
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2a3d7100,0x2b851800,0x2a3d7100,0x2b851800,0x2a3d7100,0x2b851800,0x2a3d7100,0x2bc6a800};
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2b851800,0x2ccccd00,0x2b851800,0x2ccccd00,0x2b851800,0x2ccccd00,0x2bc6a800,0x2ccccd00};  // 33-34.2 et 34.2-35
// int ANTENNES=(4);   // inferieur ou egal a ANTENNES_MAX (Nombre d'antenne balayer)

// Rory 121118, 4 pics, 1 antenne, 16 points
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2a3d7100,0x2a786c00,0x2b18fc00,0x2ba92a00};  // 33.653  37.0 38.0
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2a786c00,0x2a95eA00,0x2ba92a00,0x2bf14100};  // 34.083  37.5 38.5

// Jerome Maisonnet Sandvik 121109, 3 pics, 1 antenne, 32 points
unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x2ad0e600,0x2d70a400,0x2eb85200};  // 33.45  35.5 36.5
unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2bd70a00,0x2eb85200,0x30000000};  // 34.25  36.5 37.5

// Ruche, 4 pics, 1 antenne, 64 points
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x28f5c300,0x29fbe800,0x2ae14800,0x2be76400}; 
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x29fbe800,0x2ae14800,0x2be5e400,0x2d0e5600}; 

// Siemens couple, 4 pics, 1 antenne
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x29AD4300,0x2af4f100,0x2f175900,0x305f0700};  // 432.56-33.56-34.56 36.79   siemens1
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2af4f100,0x2c3c9f00,0x305f0700,0x31a6b500};  // 434.56 38.79   siemens1
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x28D4FE00,0x2adfa400,0x2f1aA000,0x30624e00};  // 431.90         siemens2
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2adfa400,0x2CEA4B00,0x30624e00,0x31a9fC00};  // 435.09         siemens2

// BMCI 2 pics, 2 antennes
//unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x270a3d70,0x289374c0,0x28f5c290,0x2ae147b0};
//unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x289374c0,0x2a7ef9e0,0x2a3d70a0,0x2c28f5c0};  // 33-34.2 et 34.2-35

// Torquesens
//unsigned int Fintsta[NBPICS_MAX]={0x2a3d7000,0x2b851f00,0x2ccccd00,0x2e148000,0x33333300,0x347ae200}; // 
//unsigned int Fintsto[NBPICS_MAX]={0x2b851f00,0x2ccccd00,0x2e148000,0x2f5c2900,0x347ae200,0x35c29000}; // 

// MFPM : NBPICS=10, NBSTEPS=32, NBMOY=8, ANTENNES=2
// unsigned int Fintsta[NBPICS_MAX]={0x2ad0e600,0x2bb64500,0x2d0e5600,0x2e560400,0x2f8d5000,0x30e56000,0x322d0f00,0x3374bd00,0x34bc6a00,0x35f3b600}; // MFPM
// unsigned int Fintsto[NBPICS_MAX]={0x2b53f700,0x2c083100,0x2da1cb00,0x2eb85200,0x3020c500,0x3147ae00,0x32c08300,0x33d70a00,0x354fdf00,0x36666700}; // MFPM
#else             // cas de l'oscillateur du DDS a 400 MHz (schrader SAWHOT)
unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX]={0x28f5c300/2,0x2b852000/2}; // LMD 432 & 33.7
unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX]={0x2b852000/2,0x2cac0800/2}; // snecma - 34.9
#endif

//unsigned int Fintsta[NBPICS_MAX]={0x2a4dd000,0x2b6ae000,0x2cdd3000,0x2dfa4000,0x2f6c8000,0x3089a000,0x31fbe000,0x3318f000,0x348b4000,0x35a85000,0x371aa000,0x3837b000}; // CMR
//                                 433.05     433.92      435.05     435.92     437.05     437.92     439.05     439.92     441.05
//unsigned int Fintsto[NBPICS_MAX]={0x2b6ae000,0x2c87f000,0x2dfa4000,0x2f175000,0x3089a000,0x31a6b500,0x3318f000,0x34361000,0x35a85000,0x36c56000,0x3837b000,0x3954c000}; // CMR

//volatile unsigned int Fintsta[NBPICS_MAX]={0x29990000,0x2b440000}; // hygroX
//				             432.5     433.80  
//volatile unsigned int Fintsto[NBPICS_MAX]={0x2b440000,0x2ccc0000}; // hygroX
//					     433.80    435.00
//volatile unsigned int Fintsta[NBPICS_MAX]={0x2a7f0000,0x2b6ae000}; // default
//				             433.2     433.92 
//volatile unsigned int Fintsto[NBPICS_MAX]={0x2b6ae000,0x2c56d000}; // default
//					     433.92    434.64
#endif

// Seuils min et max de detect. pics (<SEUILMIN=pas de pic, >SEUILMAX=sature)
#ifdef LMD
int SEUILMIN=100;  // dans bete_max, on teste que les voisins soient >SEUILMIN
#else
int SEUILMIN=400;  // dans bete_max, on teste que les voisins soient >SEUILMIN
#endif
int SEUILMAX=3980; // cas de la saturation, < 4095
int SEUILVAR=100000;// cas convertit_temperature : test sur la variance

// unsigned char pulse_shape[31]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,7,14,17,18,20}; // 5 pas -> puiss 20
// unsigned char pulse_shape[31]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,7,12,14,15,16}; // 5 pas -> puiss 16
const unsigned char pulse_shape[31]={1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,7,14,17,18}; // 5 pas -> puiss 18
//unsigned char pulse_shape[31]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,7,19,25,28,31}; // 5      pas -> puiss 31
// unsigned char pulse_shape[31]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,31,11,1,7,12,15,18,20,22,23,25,26,27,28,29,30,31}; // 15
int EMET=45;      // duree d'emission=45 soit 30 us (ATE)
int NY=PUISS_MAX; // 12; // Pour activer la mise en forme de crenaux
                  // si < PUISS_MAX, alors on a PUISS_MAX-NY marches 
		  //    (pulse_shape) dans le crenau, sinon impulsion rectangle
		  
//#ifdef convertit_temperature // fournit la temperature en 1/10 K
// # 538
//int A0=-2402;       // en 1/10 K, ie A0 normal *10   : -207.8      * 10
//int A1=2826272766;  // A1 normal *1E4                : 172639.394  * 1e2
//int A2=-20839;      // A2 normal *1E5 applique' a l  a diff(frequence)/1000
// # 549           //                               : -0.19077286 * 1e5 mais frequence/1000

// siemens1
int A0[ANTENNES_MAX*NBPICS_MAX]={-3469};      // -2515;      
int A1[ANTENNES_MAX*NBPICS_MAX]={36942995};   // 3004886615;
int A2[ANTENNES_MAX*NBPICS_MAX]={-27737};     // -21548;   
// siemens2
//int A0=-2548;
//int A1=29859000;
//int A2=-21787;

int contrainte0 = (0x2e147ae); // valeur minimale
