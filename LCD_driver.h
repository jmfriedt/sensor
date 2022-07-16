//******************************************************
//
//				Controller Definition
//
//******************************************************
// #define	PHILLIPS	   
#define	EPSON
//NOTE: If you know which driver you are using, you can
//increase the refresh rate of the LCD by commenting
//out the "#define" of the processor you don't need.
//(You can also use trial and error to determine the
//processor you are using)

//******************************************************
//
//			General Function Definitions
//
//******************************************************
#define LCD_SCK		0x00080000		
#define LCD_DIO		0x00020000		 
#define LCD_RES 		0x00010000		
#define LCD_CS		0x00040000		
 
//NOTE: These are the definitions for the pins we used
//while testing the LCD with the ARM LPC2148.  These
//definitions may need to be changed according to the
//processor and pins in your design.

//******************************************************
//
//			General Function Definitions
//
//******************************************************
void LCD_init(void);
void LCD_command(unsigned char data);
void LCD_data(unsigned char data);
void pset(unsigned char color, unsigned char x, unsigned char y);
void clearScreen(void);
void clearScreen1(int x1, int x2, int y1 ,int y2) ;
 void LCDSetPixel(int y, int x, int color);


// fonction tetris:






//******************************************************
//
//					LCD Dimensions
//
//******************************************************
#define ENDPAGE     132
#define ENDCOL      132

//******************************************************
//
//			EPSON Controller Definitions
//
//******************************************************
#define DISON 0xAF 	// Display on
#define DISOFF 0xAE	 // Display off
#define DISNOR 0xA6 	// Normal display
#define DISINV 0xA7 	// Inverse display
#define COMSCN 0xBB 	// Common scan direction
#define DISCTL 0xCA 	// Display control
#define SLPIN 0x95 	// Sleep in
#define SLPOUT 0x94 	// Sleep out
#define PASET 0x75 	// Page address set
#define CASET 0x15 	// Column address set
#define DATCTL 0xBC 	// Data scan direction, etc.
#define RGBSET8 0xCE 	// 256-color position set
#define RAMWR 0x5C 	// Writing to memory
#define RAMRD 0x5D 	// Reading from memory
#define PTLIN 0xA8 	// Partial display in
#define PTLOUT 0xA9 	// Partial display out
#define RMWIN 0xE0 	// Read and modify write
#define RMWOUT 0xEE 	// End
#define ASCSET 0xAA 	// Area scroll set
#define SCSTART 0xAB 	// Scroll start set
#define OSCON 0xD1 	// Internal oscillation on
#define OSCOFF 0xD2 	// Internal oscillation off
#define PWRCTR 0x20 	// Power control
#define VOLCTR 0x81 	// Electronic volume control
#define VOLUP 0xD6 	// Increment electronic control by 1
#define VOLDOWN 0xD7 	// Decrement electronic control by 1
#define TMPGRD 0x82 	// Temperature gradient set
#define EPCTIN 0xCD 	// Control EEPROM
#define EPCOUT 0xCC 	// Cancel EEPROM control
#define EPMWR 0xFC 	// Write into EEPROM
#define EPMRD 0xFD 	// Read from EEPROM
#define EPSRRD1 0x7C 	// Read register 1
#define EPSRRD2 0x7D 	// Read register 2
#define NOP 0x25 	// NOP instruction


// 12-bit color definitions
#define WHITE 0xFFF
#define BLACK 0x000
#define RED 0xF00
#define GREEN 0x0F0
#define BLUE 0x00F
#define CYAN 0x0FF
#define MAGENTA 0xF0F
#define YELLOW 0xFF0
#define BROWN 0xB22
#define ORANGE 0xFA0
#define PINK 0xF6A

// Font size

#define MEDIUM 0

// ligne

//******************************************************
//
//			PHILLIPS Controller Definitions
//
//******************************************************
//LCD Commands
#define	NOPP		0x00
#define	BSTRON		0x03
#define SLEEPIN     0x10
#define	SLEEPOUT	0x11
#define	NORON		0x13
#define	INVOFF		0x20
#define INVON      	0x21
#define	SETCON		0x25
#define DISPOFF     0x28
#define DISPON      0x29
#define CASETP      0x2A
#define PASETP      0x2B
#define RAMWRP      0x2C
#define RGBSET	    0x2D
#define	MADCTL		0x36
#define	COLMOD		0x3A
#define DISCTR      0xB9
#define	EC			0xC0
