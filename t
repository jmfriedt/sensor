
-------- begin (mode: ROM_RUN) --------

Assembling (ARM-only): startup.S
arm-none-eabi-gcc -c -mcpu=arm7tdmi  -I. -x assembler-with-cpp -DROM_RUN -Wa,-adhlns=startup.lst startup.S -o startup.o

Compiling C (ARM-only): irq.c
arm-none-eabi-gcc -c -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=irq.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations irq.c -o irq.o 

Compiling C: DDS.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=DDS.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations DDS.c -o DDS.o 
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:78:6: warning: redundant redeclaration of 'delay' [-Wredundant-decls]
   78 | void delay (int length);  // delai fixe qqsoit optimisation du code (assembleur)
      |      ^~~~~
In file included from DDS.c:31:
DDSinc.h:78:6: note: previous declaration of 'delay' was here
   78 | void delay (int length);  // delai fixe qqsoit optimisation du code (assembleur)
      |      ^~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:79:16: warning: redundant redeclaration of 'temperature' [-Wredundant-decls]
   79 | unsigned short temperature(void);  // lit la temperature du CPU (ADC16)
      |                ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:79:16: note: previous declaration of 'temperature' was here
   79 | unsigned short temperature(void);  // lit la temperature du CPU (ADC16)
      |                ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:80:5: warning: redundant redeclaration of 'my_getchar' [-Wredundant-decls]
   80 | int my_getchar (void);             // lit un char sur RS232
      |     ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:80:5: note: previous declaration of 'my_getchar' was here
   80 | int my_getchar (void);             // lit un char sur RS232
      |     ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:81:6: warning: redundant redeclaration of 'write_str' [-Wredundant-decls]
   81 | void write_str(char *,char*,int*,int);            // affiche une chaine de chars
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:81:6: note: previous declaration of 'write_str' was here
   81 | void write_str(char *,char*,int*,int);            // affiche une chaine de chars
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:82:6: warning: redundant redeclaration of 'writeDECi' [-Wredundant-decls]
   82 | void writeDECi(int,char*,int*,int);     // affiche la val d'un entier en decimal
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:82:6: note: previous declaration of 'writeDECi' was here
   82 | void writeDECi(int,char*,int*,int);     // affiche la val d'un entier en decimal
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:83:6: warning: redundant redeclaration of 'writeDECheure' [-Wredundant-decls]
   83 | void writeDECheure(unsigned char,char*,int*,int); // affiche la val d'un entier en decimal precede' de 1 0 max
      |      ^~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:83:6: note: previous declaration of 'writeDECheure' was here
   83 | void writeDECheure(unsigned char,char*,int*,int); // affiche la val d'un entier en decimal precede' de 1 0 max
      |      ^~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:84:6: warning: redundant redeclaration of 'writeDEC0i' [-Wredundant-decls]
   84 | void writeDEC0i(int,char*,int*,int);              // affiche un entier en decimal precede' de 0
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:84:6: note: previous declaration of 'writeDEC0i' was here
   84 | void writeDEC0i(int,char*,int*,int);              // affiche un entier en decimal precede' de 0
      |      ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:85:6: warning: redundant redeclaration of 'writeDECs' [-Wredundant-decls]
   85 | void writeDECs(unsigned short,char*,int*,int);    // affiche la val d'un mot en decimal
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:85:6: note: previous declaration of 'writeDECs' was here
   85 | void writeDECs(unsigned short,char*,int*,int);    // affiche la val d'un mot en decimal
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:86:6: warning: redundant redeclaration of 'writeDECc' [-Wredundant-decls]
   86 | void writeDECc(unsigned char,char*,int*,int);     // affiche la val d'un octet en decimal
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:86:6: note: previous declaration of 'writeDECc' was here
   86 | void writeDECc(unsigned char,char*,int*,int);     // affiche la val d'un octet en decimal
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:87:6: warning: redundant redeclaration of 'jmf_putchar' [-Wredundant-decls]
   87 | void jmf_putchar(int,char*, int*,int);        // affiche un caractere sur RS232
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:87:6: note: previous declaration of 'jmf_putchar' was here
   87 | void jmf_putchar(int,char*, int*,int);        // affiche un caractere sur RS232
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:88:6: warning: redundant redeclaration of 'writeHEXc' [-Wredundant-decls]
   88 | void writeHEXc(unsigned char,char*,int*,int);     // affiche la val d'un octet en hexa
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:88:6: note: previous declaration of 'writeHEXc' was here
   88 | void writeHEXc(unsigned char,char*,int*,int);     // affiche la val d'un octet en hexa
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:89:6: warning: redundant redeclaration of 'writeHEXs' [-Wredundant-decls]
   89 | void writeHEXs(unsigned short,char*,int*,int);    // affiche la val d'un mot en hexa
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:89:6: note: previous declaration of 'writeHEXs' was here
   89 | void writeHEXs(unsigned short,char*,int*,int);    // affiche la val d'un mot en hexa
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:90:6: warning: redundant redeclaration of 'writeHEXi' [-Wredundant-decls]
   90 | void writeHEXi(unsigned int,char*,int*,int);      // affiche la val d'un entier en hexa
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:90:6: note: previous declaration of 'writeHEXi' was here
   90 | void writeHEXi(unsigned int,char*,int*,int);      // affiche la val d'un entier en hexa
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:91:6: warning: redundant redeclaration of 'ADCpoweron' [-Wredundant-decls]
   91 | void ADCpoweron(int time);         // initialisation de l'ADC
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:91:6: note: previous declaration of 'ADCpoweron' was here
   91 | void ADCpoweron(int time);         // initialisation de l'ADC
      |      ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:92:6: warning: redundant redeclaration of 'envoi_PLL' [-Wredundant-decls]
   92 | void envoi_PLL(unsigned char *entree);       // configuration de la PLL
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:92:6: note: previous declaration of 'envoi_PLL' was here
   92 | void envoi_PLL(unsigned char *entree);       // configuration de la PLL
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:93:6: warning: redundant redeclaration of 'envoi_DDS' [-Wredundant-decls]
   93 | void envoi_DDS(unsigned char *entree,int n); // configuration du DDS
      |      ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:93:6: note: previous declaration of 'envoi_DDS' was here
   93 | void envoi_DDS(unsigned char *entree,int n); // configuration du DDS
      |      ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:94:6: warning: redundant redeclaration of 'puiss' [-Wredundant-decls]
   94 | void puiss(unsigned short puissance,unsigned char chan);         // configuration de l'attenuateur
      |      ^~~~~
In file included from DDS.c:31:
DDSinc.h:94:6: note: previous declaration of 'puiss' was here
   94 | void puiss(unsigned short puissance,unsigned char chan);         // configuration de l'attenuateur
      |      ^~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:95:16: warning: redundant redeclaration of 'interroge' [-Wredundant-decls]
   95 | unsigned short interroge(unsigned short puissance,unsigned int freq,unsigned int offset,unsigned char chan); // commute switch et mesure P
      |                ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:95:16: note: previous declaration of 'interroge' was here
   95 | unsigned short interroge(unsigned short puissance,unsigned int freq,unsigned int offset,unsigned char chan); // commute switch et mesure P
      |                ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:96:6: warning: redundant redeclaration of 'corrige_offset' [-Wredundant-decls]
   96 | void corrige_offset(unsigned int*); // NE MARCHE PAS
      |      ^~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:96:6: note: previous declaration of 'corrige_offset' was here
   96 | void corrige_offset(unsigned int*); // NE MARCHE PAS
      |      ^~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:97:5: warning: redundant redeclaration of 'parabole' [-Wredundant-decls]
   97 | int parabole(unsigned short *entree,unsigned short i,int fstep); // fit parabole
      |     ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:97:5: note: previous declaration of 'parabole' was here
   97 | int parabole(unsigned short *entree,unsigned short i,int fstep); // fit parabole
      |     ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:98:6: warning: redundant redeclaration of 'bete_max' [-Wredundant-decls]
   98 | void bete_max(unsigned short *entree,int n,int *moyenne); // val et pos du max
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:98:6: note: previous declaration of 'bete_max' was here
   98 | void bete_max(unsigned short *entree,int n,int *moyenne); // val et pos du max
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:100:5: warning: redundant redeclaration of 'balayage_primaire0' [-Wredundant-decls]
  100 | int balayage_primaire0(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antennes,int *phases);
      |     ^~~~~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:100:5: note: previous declaration of 'balayage_primaire0' was here
  100 | int balayage_primaire0(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antennes,int *phases);
      |     ^~~~~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:102:5: warning: redundant redeclaration of 'balaie_ism' [-Wredundant-decls]
  102 | int balaie_ism(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne,int *phases);
      |     ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:102:5: note: previous declaration of 'balaie_ism' was here
  102 | int balaie_ism(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,int *hpic,unsigned int *parab,unsigned short *puissance,unsigned int *variance,char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne,int *phases);
      |     ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:103:6: warning: redundant redeclaration of 'oscilloscope' [-Wredundant-decls]
  103 | void oscilloscope(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,unsigned short *puissance,unsigned int *offset,int *phases);
      |      ^~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:103:6: note: previous declaration of 'oscilloscope' was here
  103 | void oscilloscope(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep,unsigned short *puissance,unsigned int *offset,int *phases);
      |      ^~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:104:6: warning: redundant redeclaration of 'IRQ_Handler' [-Wredundant-decls]
  104 | void IRQ_Handler (void);  // gestion des interruptions
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:104:6: note: previous declaration of 'IRQ_Handler' was here
  104 | void IRQ_Handler (void);  // gestion des interruptions
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:105:6: warning: redundant redeclaration of 'communication' [-Wredundant-decls]
  105 | void communication(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep, unsigned short *puissance,unsigned int *offset,int *nbpic_total,char *schneider,int *phases); //lit cmds AT
      |      ^~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:105:6: note: previous declaration of 'communication' was here
  105 | void communication(unsigned int *fstart,unsigned int *fstop,unsigned int *fstep, unsigned short *puissance,unsigned int *offset,int *nbpic_total,char *schneider,int *phases); //lit cmds AT
      |      ^~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:106:6: warning: redundant redeclaration of 'aff_osc' [-Wredundant-decls]
  106 | void aff_osc(int ,int ,unsigned short,unsigned int,char *,int*); // trame affichee par Bode
      |      ^~~~~~~
In file included from DDS.c:31:
DDSinc.h:106:6: note: previous declaration of 'aff_osc' was here
  106 | void aff_osc(int ,int ,unsigned short,unsigned int,char *,int*); // trame affichee par Bode
      |      ^~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:107:6: warning: redundant redeclaration of 'change_Fsto' [-Wredundant-decls]
  107 | void change_Fsto(int);   // remplit le tableau Fintsto (cf communication())
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:107:6: note: previous declaration of 'change_Fsto' was here
  107 | void change_Fsto(int);   // remplit le tableau Fintsto (cf communication())
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:108:6: warning: redundant redeclaration of 'change_Fsta' [-Wredundant-decls]
  108 | void change_Fsta(int);   // remplit le tableau Fintsta (cf communication())
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:108:6: note: previous declaration of 'change_Fsta' was here
  108 | void change_Fsta(int);   // remplit le tableau Fintsta (cf communication())
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:109:5: warning: redundant redeclaration of 'lit_param' [-Wredundant-decls]
  109 | int lit_param(void);         // lectures des parametres dans memoire non volatile
      |     ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:109:5: note: previous declaration of 'lit_param' was here
  109 | int lit_param(void);         // lectures des parametres dans memoire non volatile
      |     ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:110:5: warning: redundant redeclaration of 'sauve_param' [-Wredundant-decls]
  110 | int sauve_param(void);   // ecriture des parametres dans memoire non volatile
      |     ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:110:5: note: previous declaration of 'sauve_param' was here
  110 | int sauve_param(void);   // ecriture des parametres dans memoire non volatile
      |     ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:111:6: warning: redundant redeclaration of 'main_deux_points' [-Wredundant-decls]
  111 | void main_deux_points(void);
      |      ^~~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:111:6: note: previous declaration of 'main_deux_points' was here
  111 | void main_deux_points(void);
      |      ^~~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:112:5: warning: redundant redeclaration of 'ma_classe' [-Wredundant-decls]
  112 | int ma_classe(const void*,const void*);
      |     ^~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:112:5: note: previous declaration of 'ma_classe' was here
  112 | int ma_classe(const void*,const void*);
      |     ^~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:113:5: warning: redundant redeclaration of 'balayage_bande0' [-Wredundant-decls]
  113 | int balayage_bande0 (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep, int *hpic, unsigned int *parab, unsigned short *puissance, unsigned int *variance, char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne, int *phases);
      |     ^~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:113:5: note: previous declaration of 'balayage_bande0' was here
  113 | int balayage_bande0 (unsigned int *fstart, unsigned int *fstop, unsigned int *fstep, int *hpic, unsigned int *parab, unsigned short *puissance, unsigned int *variance, char *balayage_primaire, unsigned int *offset, unsigned char Nb_resonateur, int antenne, int *phases);
      |     ^~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:114:6: warning: redundant redeclaration of 'writeDECTi' [-Wredundant-decls]
  114 | void writeDECTi (int d,char*,int*,int);
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:114:6: note: previous declaration of 'writeDECTi' was here
  114 | void writeDECTi (int d,char*,int*,int);
      |      ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:115:6: warning: redundant redeclaration of 'remont_CS_PLL' [-Wredundant-decls]
  115 | void remont_CS_PLL(void);
      |      ^~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:115:6: note: previous declaration of 'remont_CS_PLL' was here
  115 | void remont_CS_PLL(void);
      |      ^~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:116:6: warning: redundant redeclaration of 'sortie_led' [-Wredundant-decls]
  116 | void sortie_led(int );
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:116:6: note: previous declaration of 'sortie_led' was here
  116 | void sortie_led(int );
      |      ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:117:6: warning: redundant redeclaration of 'sortie_antenne' [-Wredundant-decls]
  117 | void sortie_antenne(int );
      |      ^~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:117:6: note: previous declaration of 'sortie_antenne' was here
  117 | void sortie_antenne(int );
      |      ^~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:118:6: warning: redundant redeclaration of 'init_CPU' [-Wredundant-decls]
  118 | void init_CPU(void);
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:118:6: note: previous declaration of 'init_CPU' was here
  118 | void init_CPU(void);
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:119:6: warning: redundant redeclaration of 'set_serial_speed' [-Wredundant-decls]
  119 | void set_serial_speed(int ,int);
      |      ^~~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:119:6: note: previous declaration of 'set_serial_speed' was here
  119 | void set_serial_speed(int ,int);
      |      ^~~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:120:6: warning: redundant redeclaration of 'set_DAC0' [-Wredundant-decls]
  120 | void set_DAC0(int );
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:120:6: note: previous declaration of 'set_DAC0' was here
  120 | void set_DAC0(int );
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:121:6: warning: redundant redeclaration of 'set_DAC1' [-Wredundant-decls]
  121 | void set_DAC1(int );
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:121:6: note: previous declaration of 'set_DAC1' was here
  121 | void set_DAC1(int );
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:122:6: warning: redundant redeclaration of 'set_DAC2' [-Wredundant-decls]
  122 | void set_DAC2(int );
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:122:6: note: previous declaration of 'set_DAC2' was here
  122 | void set_DAC2(int );
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:123:6: warning: redundant redeclaration of 'set_SPI_speedDiv' [-Wredundant-decls]
  123 | void set_SPI_speedDiv(int);
      |      ^~~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:123:6: note: previous declaration of 'set_SPI_speedDiv' was here
  123 | void set_SPI_speedDiv(int);
      |      ^~~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:124:6: warning: redundant redeclaration of 'sortie_courant' [-Wredundant-decls]
  124 | void sortie_courant(int );
      |      ^~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:124:6: note: previous declaration of 'sortie_courant' was here
  124 | void sortie_courant(int );
      |      ^~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:125:6: warning: redundant redeclaration of 'sortie_courant_init' [-Wredundant-decls]
  125 | void sortie_courant_init(void);
      |      ^~~~~~~~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:125:6: note: previous declaration of 'sortie_courant_init' was here
  125 | void sortie_courant_init(void);
      |      ^~~~~~~~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:126:6: warning: redundant redeclaration of 'kick_watchdog' [-Wredundant-decls]
  126 | void kick_watchdog(void);
      |      ^~~~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:126:6: note: previous declaration of 'kick_watchdog' was here
  126 | void kick_watchdog(void);
      |      ^~~~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:127:6: warning: redundant redeclaration of 'init_PLL' [-Wredundant-decls]
  127 | void init_PLL(void);
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:127:6: note: previous declaration of 'init_PLL' was here
  127 | void init_PLL(void);
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:128:6: warning: redundant redeclaration of 'init_DDS' [-Wredundant-decls]
  128 | void init_DDS(void);
      |      ^~~~~~~~
In file included from DDS.c:31:
DDSinc.h:128:6: note: previous declaration of 'init_DDS' was here
  128 | void init_DDS(void);
      |      ^~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:129:6: warning: redundant redeclaration of 'dephase' [-Wredundant-decls]
  129 | void dephase(unsigned short phase,unsigned char chan);
      |      ^~~~~~~
In file included from DDS.c:31:
DDSinc.h:129:6: note: previous declaration of 'dephase' was here
  129 | void dephase(unsigned short phase,unsigned char chan);
      |      ^~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:130:6: warning: redundant redeclaration of 'read_switch' [-Wredundant-decls]
  130 | char read_switch(void);
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:130:6: note: previous declaration of 'read_switch' was here
  130 | char read_switch(void);
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:131:6: warning: redundant redeclaration of 'filtre_hpf' [-Wredundant-decls]
  131 | void filtre_hpf(short *,int );
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:131:6: note: previous declaration of 'filtre_hpf' was here
  131 | void filtre_hpf(short *,int );
      |      ^~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:135:6: warning: redundant redeclaration of 'safran_init' [-Wredundant-decls]
  135 | void safran_init(void);
      |      ^~~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:135:6: note: previous declaration of 'safran_init' was here
  135 | void safran_init(void);
      |      ^~~~~~~~~~~
In file included from config_modbus.h:41,
                 from DDS.c:49:
DDSinc.h:136:6: warning: redundant redeclaration of 'safran_dac' [-Wredundant-decls]
  136 | void safran_dac(char ,unsigned short );
      |      ^~~~~~~~~~
In file included from DDS.c:31:
DDSinc.h:136:6: note: previous declaration of 'safran_dac' was here
  136 | void safran_dac(char ,unsigned short );
      |      ^~~~~~~~~~
DDS.c:116:16: warning: no previous prototype for 'jmfhtons' [-Wmissing-prototypes]
  116 | unsigned short jmfhtons(unsigned short i)
      |                ^~~~~~~~
DDS.c:119:14: warning: no previous prototype for 'jmfhtonl' [-Wmissing-prototypes]
  119 | unsigned int jmfhtonl(unsigned int i)
      |              ^~~~~~~~
DDS.c:364:1: warning: no previous prototype for 'writeHEXptri' [-Wmissing-prototypes]
  364 | writeHEXptri (int *ptr,char *chaine,int* chaine_index,int port)
      | ^~~~~~~~~~~~
DDS.c: In function 'sauve_param':
DDS.c:479:16: warning: unused variable 'kk' [-Wunused-variable]
  479 |   unsigned int kk;
      |                ^~
DDS.c: In function 'lit_param':
DDS.c:549:16: warning: unused variable 'kk' [-Wunused-variable]
  549 |   unsigned int kk; // ma_version;
      |                ^~
DDS.c: In function 'balayage_primaire0':
DDS.c:1086:25: warning: variable 'tmp3' set but not used [-Wunused-but-set-variable]
 1086 |   int tmp = 1, tmp2 = 1,tmp3=0,mes;
      |                         ^~~~
DDS.c: In function 'main':
DDS.c:2697:32: warning: unused variable 'safran_dac2' [-Wunused-variable]
 2697 |   unsigned short safran_dac1=0,safran_dac2=0;
      |                                ^~~~~~~~~~~
DDS.c:2697:18: warning: unused variable 'safran_dac1' [-Wunused-variable]
 2697 |   unsigned short safran_dac1=0,safran_dac2=0;
      |                  ^~~~~~~~~~~
DDS.c: In function 'balayage_primaire0':
DDS.c:1191:48: warning: 'parabtmp' may be used uninitialized in this function [-Wmaybe-uninitialized]
 1191 |        parab[npic] += ((parabtmp & 0x1fffffff) >> 2); // ICI SOMME=DEPASSEMENT
      |                       ~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~

Compiling C: at25.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=at25.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations at25.c -o at25.o 

Compiling C: eeprom.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=eeprom.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations eeprom.c -o eeprom.o 

Compiling C: DDSvar.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=DDSvar.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations DDSvar.c -o DDSvar.o 

Compiling C: syscalls.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=syscalls.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations syscalls.c -o syscalls.o 

Compiling C: init_aduc.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=init_aduc.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations init_aduc.c -o init_aduc.o 
init_aduc.c:687:6: warning: function declaration isn't a prototype [-Wstrict-prototypes]
  687 | void clignote() {
      |      ^~~~~~~~

Compiling C: racine/racine.c
arm-none-eabi-gcc -c  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=racine/racine.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  -Wnested-externs  -std=gnu99 -Wmissing-prototypes  -Wstrict-prototypes -Wmissing-declarations racine/racine.c -o racine/racine.o 

Linking: DDS.elf
arm-none-eabi-gcc  -mcpu=arm7tdmi  -I. -DROM_RUN -Daduc -I. -I./freemodbus-v1.5.0senseor/modbus/rtu/  -I./freemodbus-v1.5.0senseor/modbus/ascii/  -I./freemodbus-v1.5.0senseor/modbus/include/ -I./freemodbus-v1.5.0senseor/demo/ARM -Os  -Wall -Wcast-align -Wimplicit  -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wunused -Wa,-adhlns=startup.lst  -I./include -Wcast-qual -fno-omit-frame-pointer  startup.o  irq.o DDS.o at25.o eeprom.o DDSvar.o syscalls.o init_aduc.o racine/racine.o     --output DDS.elf -nostartfiles -Wl,-Map=DDS.map,--cref  -lm      -TADuC7026-ROM.ld
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0x0): multiple definition of `DABORT'; irq.o:(.bss+0x10): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0x4): multiple definition of `PABORT'; irq.o:(.bss+0xc): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0x8): multiple definition of `UNDEF'; irq.o:(.bss+0x8): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0xc): multiple definition of `FIQ'; irq.o:(.bss+0x0): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0x10): multiple definition of `SWI'; irq.o:(.bss+0x4): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: at25.o:(.bss+0x14): multiple definition of `IRQ'; irq.o:(.bss+0x14): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0x0): multiple definition of `DABORT'; irq.o:(.bss+0x10): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0x4): multiple definition of `PABORT'; irq.o:(.bss+0xc): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0x8): multiple definition of `UNDEF'; irq.o:(.bss+0x8): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0xc): multiple definition of `FIQ'; irq.o:(.bss+0x0): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0x10): multiple definition of `SWI'; irq.o:(.bss+0x4): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: eeprom.o:(.bss+0x14): multiple definition of `IRQ'; irq.o:(.bss+0x14): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0x4): multiple definition of `DABORT'; irq.o:(.bss+0x10): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0x8): multiple definition of `PABORT'; irq.o:(.bss+0xc): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0xc): multiple definition of `UNDEF'; irq.o:(.bss+0x8): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0x10): multiple definition of `FIQ'; irq.o:(.bss+0x0): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0x14): multiple definition of `SWI'; irq.o:(.bss+0x4): first defined here
/usr/lib/gcc/arm-none-eabi/10.3.1/../../../arm-none-eabi/bin/ld: init_aduc.o:(.bss+0x0): multiple definition of `IRQ'; irq.o:(.bss+0x14): first defined here
collect2: error: ld returned 1 exit status
make: *** [makefile.aduc:272: DDS.elf] Error 1
