extern int convertit_temperature;
extern int safran_temperature;
extern int bandes;
extern unsigned char deux_points;
extern unsigned char mode_balayage_continu;

extern int LBT;	        
extern unsigned char agc; 
extern unsigned char trame_minimale;

extern int ATTENTE;
extern int HATTENTE;
extern int NBMOY; 
extern int NBMES; 
extern unsigned int NBPICS;
extern int NBSTEPS;
extern int ANTENNES;
extern unsigned int FSTART;
extern unsigned int FSTOP;
extern unsigned int Fintsta[ANTENNES_MAX*NBPICS_MAX];
extern unsigned int Fintsto[ANTENNES_MAX*NBPICS_MAX];
extern int SEUILMIN;
extern int SEUILMAX;
extern int SEUILVAR;
extern const unsigned char pulse_shape[31];
extern int EMET; 
extern int NY;
extern int A0[ANTENNES_MAX*NBPICS_MAX];      
extern int A1[ANTENNES_MAX*NBPICS_MAX]; 
extern int A2[ANTENNES_MAX*NBPICS_MAX];   
extern int contrainte0;
extern int RS_BAUDRATE;
extern int RS_PARITY;
#ifdef AD9958
extern int puiss9958[36];
#endif
