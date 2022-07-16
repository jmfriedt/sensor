// configuration pour les divers interrogateurs :
// interrogteur classique ancienne version (pre-nov 2012) : #undef f400, #undef ad9958, #undef ADF4111
// interrogteur classique nouvelle version (post-nov 2012): #undef f400, #undef ad9958, #define ADF4111
// interrogteur gilloux: #define f400, #define ad9958, #undef ADF4111  et adapter l'offset

// Ces deux constantes sont maintenant mises en place dans le Makefile
// #define f400   // generation de DDS=400+33 MHz au lieu de melange 200+33 (SAWHOT)
//#define AD9958         // necessite #defined f400 

#undef gilloux

#undef debug    	// affiche les valeurs des variables temporaires pour deverminer
#undef debug2  		// affiche les valeurs des variables temporaires pour deverminer

#undef arcelor		// lecture du switch pour couper emission RF (P1.2 sur ADuC a la masse pour couper)
#undef compte_tour      // exploitation capteur mgnetique sur ADC2/CMP0(79)
#define SetH            // Sample & Hold pour ameliorer sensibilite
#define CONSIGNE_PUISS 3000 // puissance du pic sur laqelle on cherche a asservir puissance emise

#ifndef gilloux
#define OFFSET 0xB00    // CFF si mux -- 800..FFF 
#else
#define OFFSET 0xB00    // CFF si mux -- 800..FFF 
#endif

#undef auto_offset     // calcul automatique de l'offset (ne marche pas)
#define rtc             // datation des trames au lieu de temperature ADuC
#undef RTC0             // datation sous forme de hh:mm:ss
#undef LMD              // appui sur 'a' pour envoyer trames (asservit continu)
                        //     LMD => def manuel et undef rtc & 
		 	//     mode_balayage_continu =0 & bandes =1
#undef dephaseur 	// balayage de phase avec dephaseur sur DAC3
//#undef manuel		// definition manuelle des bornes, ou distribution 
#undef manuel                        //   equidistante entre FSTART et FSTOP sur NBPICS
#undef LCD              // afficheur Nokia LCD  * A REVOIR SI FONCTIONNEL *
#undef hpf // FAUX NE PAS UTILISER laplacien passe haut pour eliminer composante continue & lineaire

// mode de stockage des parametres en memoire non volatile
#undef at25              // exploitation d'une memoire externe flash AT25DF021 
#define eeprom           // exploitation de l'EEPROM interne a l'ADuC7026  
#undef v2450  // F1/F2 sur P2 au lieu de P4 sur elec 2450 MHz

#define attend 5        // delai apres communication SPI et pour les CS# (5)

#undef periodique      // reveil periodique entre deux modes veille (ADuC7026 uniquement)

//----------------------------------------------------------------------------//
//      CHOIX DU TYPE DE DETECTION:
//----------------------------------------------------------------------------//
// 2 types possibles: 
//              -> Détection de tous les resonateur en meme temps par la 
//                   fonction balaie_ism #define detec_tous
//              -> Sinon detection des resonateur un par un (permet d'avoir 
//                   plus de resonateurs)   
#define detec_tous   // si détection de tous les pics en meme temps

#define wrc          // version de l'interrogateur pour mux antenne

// POUR declaration des tableaux: le produit de ces 4 termes doit rester ~ 10^4
#define NBMES_MAX   (1)  // nbre de mesure(s) : 1..16
#define NBMOY_MAX   (4)  // nbre de moyenne(s) : 1..16
#define NBPICS_MAX  (2)  // nbre de resonateur(s) par antenne : 1..12
#define NBSTEPS_MAX (868) // nbre de pas de freq par resonance : 32..128
#define ANTENNES_MAX (1) // nbre d'antenne(s) : 1..4

#define NBRATES  3     // on se permet NBMOY*NBRATE essais pour trouver NBMOY 
                       //    bonnes mesures, >=1

// puissance max emise (31=+10 dBm, 1 unite=1 dB)
#define PUISS_MAX  31  // 16 Apres passage chez emitech, 20 pour ISM, 31 sinon // 140220: passage de 5 a 10 bits

#define ADF4111        // nouvelle PLL, apres 11/2012, sur interrogateur classique
#define NB_CHARS 10    // taille du tampon pour la communication RS232

//----------------------------------------------------------------------------/
// Declation des fonctions		
//----------------------------------------------------------------------------/
void delay (int length);  // delai fixe qqsoit optimisation du code (assembleur)
unsigned short temperature(void);  // lit la temperature du CPU (ADC16)
int my_getchar (void);             // lit un char sur RS232
void write_str(char *,char*,int*,int);            // affiche une chaine de chars
void writeDECi(int,char*,int*,int);     // affiche la val d'un entier en decimal
void writeDECheure(unsigned char,char*,int*,int); // affiche la val d'un entier en decimal precede' de 1 0 max
void writeDEC0i(int,char*,int*,int);              // affiche un entier en decimal precede' de 0
void writeDECs(unsigned short,char*,int*,int);    // affiche la val d'un mot en decimal
void writeDECc(unsigned char,char*,int*,int);     // affiche la val d'un octet en decimal
void jmf_putchar(int,char*, int*,int);        // affiche un caractere sur RS232      
void writeHEXc(unsigned char,char*,int*,int);     // affiche la val d'un octet en hexa
void writeHEXs(unsigned short,char*,int*,int);    // affiche la val d'un mot en hexa
void writeHEXi(unsigned int,char*,int*,int);      // affiche la val d'un entier en hexa
void ADCpoweron(int time);         // initialisation de l'ADC
void envoi_PLL(unsigned char *entree);       // configuration de la PLL
void envoi_DDS(unsigned char *entree,int n); // configuration du DDS
void puiss(unsigned short puissance,unsigned char chan);         // configuration de l'attenuateur
unsigned short interroge(unsigned short puissance,unsigned int freq,unsigned int offset,unsigned char chan); // commute switch et mesure P
void corrige_offset(unsigned int*); // NE MARCHE PAS
int parabole(unsigned short *entree,unsigned short i,int fstep); // fit parabole
void bete_max(unsigned short *entree,int n,int *moyenne); // val et pos du max
// LE coeur du programme : balaie la bande de FSTART a FSTOP & analyse result
int balayage_primaire0(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antennes,int *phases);
// appelle balayage_primaire0(), choisit si on reste en 3 points ou peigne fixe
int balaie_ism(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne,int *phases);
void oscilloscope(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,unsigned short *puissance,unsigned int *offset,int *phases);
void IRQ_Handler (void);  // gestion des interruptions
void communication(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep, unsigned short *puissance,unsigned int *offset,int *nbpic_total,char *schneider,int *phases); //lit cmds AT
void aff_osc(int ,int ,unsigned short,unsigned int,char *,int*); // trame affichee par Bode
void change_Fsto(int);   // remplit le tableau Fintsto (cf communication())
void change_Fsta(int);   // remplit le tableau Fintsta (cf communication())
int lit_param(void);         // lectures des parametres dans memoire non volatile
int sauve_param(void);   // ecriture des parametres dans memoire non volatile
void main_deux_points(void);
int ma_classe(const void*,const void*);
int balayage_bande0 (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep, int *hpic, unsigned int *parab, unsigned short *puissance, unsigned int *variance, char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne, int *phases);
void writeDECTi (int d,char*,int*,int);
void remont_CS_PLL(void);
void sortie_led(int );
void sortie_antenne(int );
void init_CPU(void);
void set_serial_speed(int ,int);
void set_DAC0(int );
void set_DAC1(int ); 
void set_DAC2(int );
void set_SPI_speedDiv(int);
void sortie_courant(int );
void sortie_courant_init(void);
void kick_watchdog(void);
void init_PLL(void);
void init_DDS(void);
void dephase(unsigned short phase,unsigned char chan);
char read_switch(void);
void filtre_hpf(short *,int );
#ifdef periodique
void power_down(void);
#endif

//#define analog_phase // dephase analogique sur le DAC de la sortie oscillo pour Lionel
#undef schneider       // ajustement automatique de l'offset
