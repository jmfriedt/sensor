#undef debug    	// affiche les valeurs des variables temporaires pour deverminer
#undef debug2  		// affiche les valeurs des variables temporaires pour deverminer

#define affiche_rs232   // affiche sur le port serie
#undef compte_tour      // exploitation capteur mgnetique sur ADC2/CMP0(79)
#define SetH            // Sample & Hold pour ameliorer sensibilite
#define OFFSET 0xA00    // CFF si mux -- 800..FFF 
#define auto_offset     // calcul automatique de l'offset (ne marche pas)
#define rtc             // datation des trames au lieu de temperature ADuC
#define RTC0             // datation sous forme de hh:mm:ss
#undef LMD              // appui sur 'a' pour envoyer trames (asservit continu)
                        //     LMD => def manuel et undef rtc & 
		 	//     mode_balayage_continu =0 & bandes =1
#undef dephaseur 	// balayage de phase avec dephaseur sur DAC3
#undef manuel		// definition manuelle des bornes, ou distribution 
                        //   equidistante entre FSTART et FSTOP sur NBPICS
#undef LCD              // afficheur Nokia LCD  * A REVOIR SI FONCTIONNEL *


// mode de stockage des parametres en memoire non volatile
#undef at25              // exploitation d'une memoire externe flash AT25DF021 
#define eeprom           // exploitation de l'EEPROM interne a l'ADuC7026  
#undef f400   // generation de DDS=400+33 MHz au lieu de melange 200+33 (SAWHOT)
#undef v2450  // F1/F2 sur P2 au lieu de P4 sur elec 2450 MHz

#define RS_BAUDRATE 0x22 // 0x0b=115200, 0x17=57600, 38400-0x22

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
#define NBMOY_MAX  17   // nbre de moyenne(s) : 1..16
#define NBPICS_MAX  8   // nbre de resonateur(s) par antenne : 1..12
#define NBSTEPS_MAX 68  // nbre de pas de freq par resonance : 32..128
#define ANTENNES_MAX 1	// nbre d'antenne(s) : 1..4

/**************************** CAS DES BORNES MANUELLES ************************/

// Seuils min et max de detect. pics (<SEUILMIN=pas de pic, >SEUILMAX=sature)

#define NBRATES  2     // on se permet NBMOY*NBRATE essais pour trouver NBMOY 
                       //    bonnes mesures, >=1

// puissance max emise (31=+10 dBm, 1 unite=1 dB)
#define PUISS_MAX  31  // 16 Apres passage chez emitech, 20 pour ISM, 31 sinon

#define ADF4111        // nouvelle PLL, apres 11/2012, sur interrogateur classique
#undef AD9958         // necessite #defined f400 je pense

#define NB_CHARS 30    // taille du tampon pour la communication RS232


//----------------------------------------------------------------------------/
// Declation des fonctions		
//----------------------------------------------------------------------------/
void delay (int length);  // delai fixe qqsoit optimisation du code (assembleur)
unsigned short temperature(void);  // lit la temperature du CPU (ADC16)
int my_getchar (void);             // lit un char sur RS232
void write_str(char *);            // affiche une chaine de chars
void writeDECi(int);               // affiche la val d'un entier en decimal
void writeDECheure(unsigned char); // affiche la val d'un entier en decimal precede' de 1 0 max
void writeDEC0i(int);              // affiche un entier en decimal precede' de 0
void writeDECs(unsigned short);    // affiche la val d'un mot en decimal
void writeDECc(unsigned char);     // affiche la val d'un octet en decimal
void jmf_putchar(int );            // affiche un caractere sur RS232      
void writeHEXc(unsigned char);     // affiche la val d'un octet en hexa
void writeHEXs(unsigned short);    // affiche la val d'un mot en hexa
void writeHEXi(unsigned int);      // affiche la val d'un entier en hexa
void ADCpoweron(int time);         // initialisation de l'ADC
void envoi_PLL(unsigned char *entree);       // configuration de la PLL
void envoi_DDS(unsigned char *entree,int n); // configuration du DDS
void puiss(unsigned char puissance);         // configuration de l'attenuateur
unsigned short interroge(unsigned char puissance); // commute switch et mesure P
void corrige_offset(unsigned int*); // NE MARCHE PAS
int parabole(unsigned short *entree,unsigned short i,int fstep); // fit parabole
void bete_max(unsigned short *entree,int n,int *moyenne); // val et pos du max
// LE coeur du programme : balaie la bande de FSTART a FSTOP & analyse result
int balayage_primaire0(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned char *puissance,unsigned int *variance,char *balayage_primaire,unsigned char Nb_resonateur, int antennes,int *phases);
// appelle balayage_primaire0(), choisit si on reste en 3 points ou peigne fixe
int balaie_ism(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned char *puissance,unsigned int *variance,char *balayage_primaire,unsigned char Nb_resonateur, int antenne,int *phases);
void oscilloscope(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,unsigned char *puissance);
void IRQ_Handler (void);  // gestion des interruptions
void communication(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep, unsigned char *puissance,unsigned int *offset,int *nbpic_total,int *proportional); //lit cmds AT
void aff_osc(int freq,int antenne,unsigned char p); // trame affichee par Bode
void change_Fsto(int);   // remplit le tableau Fintsto (cf communication())
void change_Fsta(int);   // remplit le tableau Fintsta (cf communication())
int lit_param(int);      // lectures des parametres dans memoire non volatile
int sauve_param(void);   // ecriture des parametres dans memoire non volatile
void main_deux_points(void);
int ma_classe(const void*,const void*);
int balayage_bande0 (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep, int *hpic, unsigned int *parab, unsigned char *puissance, unsigned int *variance, char *balayage_primaire, unsigned char Nb_resonateur, int antenne, int *phases);
void writeDECTi (int d);
void remont_CS_PLL(void);
void sortie_led(int );
void sortie_antenne(int );
void init_CPU(void);
void set_serial_speed(int );
void set_DAC0(int );
void set_DAC1(int ); 
void set_DAC2(int );
void set_SPI_speedDiv(int);
void sortie_courant(int );
void sortie_courant_init();
#ifdef periodique
void power_down(void);
#endif

