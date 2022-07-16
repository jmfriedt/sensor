
/*  PWM     */


#define PWMBASE 	(*(volatile unsigned long *) 	0XFFFFFC00)
#define PWMCON 		(*(volatile unsigned long *) 	0XFFFFFC00)
#define PWMSTA	 	(*(volatile unsigned long *) 	0XFFFFFC04)
#define PWMDAT0 	(*(volatile unsigned long *) 	0XFFFFFC08)
#define PWMDAT1 	(*(volatile unsigned long *) 	0XFFFFFC0C)
#define PWMCFG 		(*(volatile unsigned long *) 	0XFFFFFC10)

#define PWMCH0 		(*(volatile unsigned long *) 	0XFFFFFC14)
#define PWMCH1 		(*(volatile unsigned long *) 	0XFFFFFC18)
#define PWMCH2 		(*(volatile unsigned long *) 	0XFFFFFC1C)

#define PWMEN 		(*(volatile unsigned long *) 	0XFFFFFC20)
#define PWMDAT2 	(*(volatile unsigned long *) 	0XFFFFFC24)


