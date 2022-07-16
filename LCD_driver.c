//********************************************************************
//
//			Color LCD (128x128) Basic Library
//             (Written for ARM LPC2148)
//
//    					3-9-2008
//			Copyright Spark Fun Electronics� 2008
//						Ryan Owens
//					ryan at sparkfun.com
//
//		See LCD_driver.h for I/O pin and register definitions
//			*Delays assume a 16MHz external clock
//				(should be adjusted accordingly)
//
//
//		Reference: Jim Lynch's "Nokia 6100 LCD Display Driver"
//(http://www.sparkfun.com/tutorial/Nokia%206100%20LCD%20Display%20Driver.pdf)
//	
//
//
//
// Les fonctions, Couleur, entr�s et sortie sont d�clar�es dans driver.h
// Le format de la courleur est : RRRR_GGGG_BBBB
// transmit sur 3 trames.
//
// Les tableaux de caract�re et d'icon se trouve � la fin du driver
//
//
//
//
//
//********************************************************************
#include "LCD_driver.h"
#include<ADuC7026.h>
#include <stdio.h>
#include <string.h>	  

void nop()
{

}

//Usage: LCD_command(DISPON);
//Inputs: char data - The command character to be sent to the LCD
//Outputs: None
//Description: This function sends a command signal and an 8 bit command
//				character to the LCD in a 9-bit package

// la communication utilis� est une communication manuelle 
// car 9 bit
void LCD_command(unsigned char data)
{char j;
	
//	IODIR0 |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);		//Assign LCD pins as Outputs
	
//	IOCLR0 = LCD_CS;      	// Enable Chip (Bring CS signal LOW)
//  IOCLR0 = LCD_DIO;     	// Prepare Data Line (Bring DIO signal LOW)
	GP2DAT &= ~LCD_CS;
	GP2DAT &= ~LCD_DIO;

//    IOCLR0 = LCD_SCK;     // send clock pulse (Bring clock signal LOW)
//    IOSET0 = LCD_SCK;		// (Bring clock signal HIGH)
	GP2DAT &= ~LCD_SCK;
	GP2DAT |= LCD_SCK;

    for (j = 0; j < 8; j++)
    {
        if ((data & 0x80) == 0x80) GP2DAT |= LCD_DIO; // IOSET0 = LCD_DIO;	//Set data line HIGH
        else GP2DAT &= ~LCD_DIO; // IOCLR0 = LCD_DIO;							//Set data line LOW

        //IOCLR0 = LCD_SCK;   // send clock pulse (Bring clock signal LOW)
        //IOSET0 = LCD_SCK;	// (Bring clock signal HIGH)
	    GP2DAT &= ~LCD_SCK;
	    GP2DAT |= LCD_SCK;

        data <<= 1;
    }

//  IOSET0 = LCD_CS;    		// Disable Chip (Bring CS signal HIGH)
    GP2DAT |= LCD_CS;
}

//Usage: LCD_data(0x03);
//Inputs: char data - The data character to be sent to the LCD
//Outputs: None
//Description: This function sends a data signal and an 8 bit data character
//				to the LCD in a 9-bit package
void LCD_data(unsigned char data)
{
    char delay= 1,i,j;

//	IODIR0 |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);		//Assign LCD pins as Outputs
	
//	IOCLR0 = LCD_SCK;      	
//	IOCLR0 = LCD_CS;      	// Enable Chip (Bring CS signal LOW)    
	GP2DAT &= ~LCD_SCK;
	GP2DAT &= ~LCD_CS;

	#ifdef	PHILLIPS
		for (i = 0; i < 40; i++) nop(); // asm volatile ("nop");	//Only need this delay for the Phillips Controller
	#endif
//	IOSET0 = LCD_DIO;    	// Prepare Data Line (Bring DIO signal LOW)
	GP2DAT |=	LCD_DIO;
//  IOCLR0 = LCD_SCK;    	// send clock pulse (Bring clock signal LOW)
	GP2DAT &= ~LCD_SCK;
	#ifdef	PHILLIPS
		for (i = 0; i < 40; i++) nop() ; // asm volatile ("nop");	//Only need this delay for the Phillips Controller
	#endif
    // IOSET0 = LCD_SCK;		// (Bring clock signal HIGH)
	GP2DAT |=	LCD_SCK;
	for (j = 0; j < 8; j++)
    {
        if ((data & 0x80) == 0x80) GP2DAT |= LCD_DIO;   // IOSET0 = LCD_DIO;	//Set data line HIGH
        else GP2DAT &=~ LCD_DIO;   // IOCLR0 = LCD_DIO;	//Set data line LOW
//        IOCLR0 = LCD_SCK;   							// send clock pulse (Bring clock signal LOW)
		  GP2DAT &=	~LCD_SCK;
		#ifdef	PHILLIPS
			for (i = 0; i < delay; i++) nop(); // asm volatile ("nop");	//Only need this delay for the Phillips Controller
		#endif
//        IOSET0 = LCD_SCK;								// (Bring clock signal HIGH)
		  GP2DAT |=	LCD_SCK;
		#ifdef	PHILLIPS
			for (i = 0; i < delay; i++) nop() ; // asm volatile ("nop");	//Only need this delay for the Phillips Controller
		#endif
        data <<= 1;
    }
//    IOSET0 = LCD_CS;     		// Disable Chip (Bring CS signal HIGH)
    GP2DAT |= LCD_CS;

}

//Usage: LCD_init();
//Inputs: None
//Outputs: None
//Description: This function will initialize the LCD, regardless of whether the
//				LCD contains the EPSON or PHILLIPS driver
void LCD_init(void)
{
    int j;
	
    // reset display
	GP2DAT=0x0F000000; // P1.4 & P1.6 (CK & DAT)
	 // P4.2 et P4.4	(RESET & CS)
	// IODIR0 |= (LCD_DIO | LCD_SCK | LCD_CS | LCD_RES);		//Assign LCD pins as Outputs	
    // IOCLR0 = (LCD_SCK | LCD_DIO);							//output_low (SPI_CLK); //output_low (SPI_DO);
    
	// IOSET0 = LCD_CS;				//output_high (LCD_CS);
	GP2DAT |= LCD_CS;

    for (j = 0; j < 16; j++);
//    IOCLR0 = LCD_RES;				//output_low (LCD_RESET);
    GP2DAT &= ~LCD_RES;
    for (j = 0; j < 300000; j++);
//    IOSET0 = LCD_RES;				//output_high (LCD_RESET);
    GP2DAT |= LCD_RES;
//    IOSET0 = (LCD_SCK | LCD_DIO);
    GP2DAT|= (LCD_SCK | LCD_DIO);
    for (j = 0; j < 300000; j++);	//delay_ms(100);
	
    LCD_command(DISCTL);  	// display control(EPSON)
    LCD_data(0);   	// 12 = 1100 - CL dividing ratio [don't divide] switching period 8H (default)
    LCD_data(32);
    LCD_data(0);
	
    LCD_command(COMSCN);  	// common scanning direction(EPSON)
    LCD_data(1);
    
    LCD_command(OSCON);  	// internal oscialltor ON(EPSON)
    
    LCD_command(SLPOUT);  	// sleep out(EPSON)

    LCD_command(PWRCTR); 	// power ctrl(EPSON)
    LCD_data(0x0F);    //everything on, no external reference resistors

    LCD_command(DISINV);  	// invert display mode(EPSON)

    LCD_command(DATCTL);  	// data control(EPSON)
    LCD_data(0x01);	//correct for normal sin7	
    LCD_data(0x00);   	// normal RGB arrangement
    LCD_data(0x02);   	// 12-bit grayscale	

    LCD_command(VOLCTR);  	// electronic volume, this is the contrast/brightness(EPSON)
    LCD_data(33);   	// volume (contrast) setting - fine tuning, original
    LCD_data(0x03);   	// internal resistor ratio - coarse adjustment
	//LCD_command(SETCON);	//Set Contrast(PHILLIPS)
	
		
	  
    LCD_command(NOP);  	// nop(EPSON)

    LCD_command(DISON);   	// display on(EPSON)
}	  






//-----------------------------------------------------------------------------------------------------------


// focntion pour �ffacer l'�cran
void clearScreen(void) {
	long i; // loop counter
	// Row address set (command 0x2B)
	LCD_command(PASET);
	LCD_data(0);
	LCD_data(131);
	// Column address set (command 0x2A)
	LCD_command(CASET);
	LCD_data(0);
	LCD_data(131);
	// set the display memory to BLACK
	LCD_command(RAMWR);
	for(i = 0; i < ((131 * 131) / 2); i++) {
	LCD_data((WHITE >> 4) & 0xFF);
	LCD_data(((WHITE & 0xF) << 4) | ((WHITE >> 8) & 0xF));
	LCD_data(WHITE & 0xFF);
	}
}

// focntion pour �ffacer une partie de l'�cran	 
/*void clearScreen1(int x1,int x2,int y1,int y2) {	   // Y puis X
	long i; // loop counter
	// Row address set (command 0x2B)
	int color= WHITE;
	LCD_command(PASET);
	LCD_data(x1);
	LCD_data(x2);
	// Column address set (command 0x2A)
	LCD_command(CASET);
	LCD_data(y1);
	LCD_data(y2);
	// set the display memory to BLACK
	LCD_command(RAMWR);
	
	for(i = 0; i < ((x2-x1)*(y2-y1)); i++) {
	LCD_data((color >> 4) & 0xFF);
	LCD_data(((color & 0xF) << 4) | ((WHITE >> 8) & 0xF));
	LCD_data(color & 0xFF);
	}
}*/

// fonction prim�re : 
// cette fonction va �tre appel�e par toute les autres car elle permet 
// d'�crire un pixel.
// ATTENTION: du a un inverssion de Y et X 
// le X et le Y sont invers� pour toute les autres fonctions.

 void LCDSetPixel(int y, int x, int color) 
 {
	// Draw a pixel at the specified coordinates and color
	// x     =   column address [0..129]
	// y     =   row address [0..129]
	// color =   16-bit color value: 0000_RRRR_GGGG_BBBB
	//   x+=2;               // visible display starts in column 2
	    LCD_command(CASET);      // set column address
	    LCD_data(x);         // start
	    LCD_data(x);         // end
	
	    LCD_command(PASET);      // set page (row) address
	    LCD_data(y);         // start
	    LCD_data(y);         // end
	
	    LCD_command(RAMWR);
	    
		LCD_data((color >> 4) & 0xFF);
		LCD_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
		LCD_data(color & 0xFF);
} 

// Fonction d'affichage de courbe
// (Y0,X0,Y1,X1)
void LCDSetLine(int x0, int y0, int x1, int y1, int color) {
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	if (dy < 0) { dy = -dy; stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx; stepx = -1; } else { stepx = 1; }
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx
	LCDSetPixel(x0, y0, color);
	if (dx > dy) {
		int fraction = dy - (dx >> 1); // same as 2*dy - dx
		while (x0 != x1) {
		if (fraction >= 0) {
		y0 += stepy;
		fraction -= dx; // same as fraction -= 2*dx
							}
			x0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			LCDSetPixel(x0, y0, color);
						}
				} else {
					int fraction = dx - (dy >> 1);
					while (y0 != y1) {
					if (fraction >= 0) {
						x0 += stepx;
						fraction -= dy;
										}
					y0 += stepy;
					fraction += dx;
					LCDSetPixel(x0, y0, color);
									}
					}
}




 /* affichage d'un caractere, utilise les tableaus en bas avec 3 tailles possibles, 
 	LARGE	 8x6
	MEDIUM	 8x8
	SMALL	 16x8

	exemple dessin d'un A 8x6


				 /  *   /
				 /*  *  /		Les espaces sont des 0;
				 /*   * /
				 /*   * /
				 /***** /
				 /*   * /
				 /*   * /
				 /      /

   code : 0x20,0x50,0x88,0x88,0xF8,0x88,0x88,0x00,

   on donne le code la couleur pour chaque pixel:
   code de 12 Bits RRRRGGGGBBBB
   			LCD_data((Word0 >> 4) & 0xFF);
			LCD_data(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
			LCD_data(Word1 & 0xFF);
   */

void LCDPutChar(char c, int x, int y, int size, int fColor, int bColor) {
	
	
	extern const unsigned char FONT8x8[97][8] ;				// Modification de la police 
	

	int i,j;
	unsigned int nCols;
	unsigned int nRows;
	unsigned int nBytes;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned int Word0;
	unsigned int Word1;
	unsigned char *pFont;
	unsigned char *pChar;
	unsigned char *FontTable[] = { (unsigned char *)FONT8x8,		// Modification de la police 
	};

	// get pointer to the beginning of the selected font table
	pFont = (unsigned char *)FontTable[size];

	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);

	// get pointer to the last byte of the desired character
	pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;

	// Row address set (command 0x2B)
	LCD_command(PASET);
	LCD_data(x);
	LCD_data(x + nRows - 1);

	// Column address set (command 0x2A)
	LCD_command(CASET);
	LCD_data(y);
	LCD_data(y + nCols - 1);

	// WRITE MEMORY
	LCD_command(RAMWR);

	// loop on each row, working backwards from the bottom to the top
		for (i = nRows - 1; i >= 0; i--) {
		// copy pixel row from font table and then decrement row
		PixelRow = *pChar--;
		// loop on each pixel in the row (left to right)
		// Note: we do two pixels each loop
		Mask = 0x80;
			for (j = 0; j < nCols; j += 2) {
			// if pixel bit set, use foreground color; else use the background color
			// now get the pixel color for two successive pixels
			if ((PixelRow & Mask) == 0)
			Word0 = bColor;
			else
			Word0 = fColor;
			Mask = Mask >> 1;
			if ((PixelRow & Mask) == 0)
			Word1 = bColor;
			else
			Word1 = fColor;
			Mask = Mask >> 1;
			// use this information to output three data bytes
			LCD_data((Word0 >> 4) & 0xFF);
			LCD_data(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
			LCD_data(Word1 & 0xFF);
		}
	}
	// terminate the Write Memory command
	LCD_command(NOP);
}

 
 /* fonction icon 
 elle utilise les icones T et P dessin�e dans les tableaus
 pour en rajouter il s'agit de dessin de 28 pour les colones X	0xFFFFFFF
 et de 32 pour les lignes	*/



/* affichage de chaine de caract�re, 
 affiche d'un carat�re puis incr�mentation pour point� sur le suivant*/

void LCDPutStr(char *pString, int x, int y, int Size, int fColor, int bColor) {
	// loop until null-terminator is seen
	while (*pString != 0x00) {
	// draw the character
		LCDPutChar(*pString++, x, y, Size, fColor, bColor);
	// advance the y position
		
	y = y + 8;
	// bail out if y exceeds 131
	if (y > 131) break;
	}
}



//********************************************************************
//
//	 TAbleau de donn�e:
//
//********************************************************************
/*

const unsigned char FONT8x16[97][16] = {
	0x08,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // columns, rows, num_bytes_per_char
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // space 0x20
	0x00,0x00,0x18,0x3C,0x3C,0x3C,0x18,0x18,0x18,0x00,0x18,0x18,0x00,0x00,0x00,0x00, // !
	0x00,0x63,0x63,0x63,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // "
	0x00,0x00,0x00,0x36,0x36,0x7F,0x36,0x36,0x36,0x7F,0x36,0x36,0x00,0x00,0x00,0x00, // #
	0x0C,0x0C,0x3E,0x63,0x61,0x60,0x3E,0x03,0x03,0x43,0x63,0x3E,0x0C,0x0C,0x00,0x00, // $
	0x00,0x00,0x00,0x00,0x00,0x61,0x63,0x06,0x0C,0x18,0x33,0x63,0x00,0x00,0x00,0x00, // %
	0x00,0x00,0x00,0x1C,0x36,0x36,0x1C,0x3B,0x6E,0x66,0x66,0x3B,0x00,0x00,0x00,0x00, // &
	0x00,0x30,0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // '
	0x00,0x00,0x0C,0x18,0x18,0x30,0x30,0x30,0x30,0x18,0x18,0x0C,0x00,0x00,0x00,0x00, // (
	0x00,0x00,0x18,0x0C,0x0C,0x06,0x06,0x06,0x06,0x0C,0x0C,0x18,0x00,0x00,0x00,0x00, // )
	0x00,0x00,0x00,0x00,0x42,0x66,0x3C,0xFF,0x3C,0x66,0x42,0x00,0x00,0x00,0x00,0x00, // *
	0x00,0x00,0x00,0x00,0x18,0x18,0x18,0xFF,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00, // +
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00, // ,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // -
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00, // .
	0x00,0x00,0x01,0x03,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00, // / (forward slash)
	0x00,0x00,0x3E,0x63,0x63,0x63,0x6B,0x6B,0x63,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // 0 0x30
	0x00,0x00,0x0C,0x1C,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3F,0x00,0x00,0x00,0x00, // 1
	0x00,0x00,0x3E,0x63,0x03,0x06,0x0C,0x18,0x30,0x61,0x63,0x7F,0x00,0x00,0x00,0x00, // 2
	0x00,0x00,0x3E,0x63,0x03,0x03,0x1E,0x03,0x03,0x03,0x63,0x3E,0x00,0x00,0x00,0x00, // 3
	0x00,0x00,0x06,0x0E,0x1E,0x36,0x66,0x66,0x7F,0x06,0x06,0x0F,0x00,0x00,0x00,0x00, // 4
	0x00,0x00,0x7F,0x60,0x60,0x60,0x7E,0x03,0x03,0x63,0x73,0x3E,0x00,0x00,0x00,0x00, // 5
	0x00,0x00,0x1C,0x30,0x60,0x60,0x7E,0x63,0x63,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // 6
	0x00,0x00,0x7F,0x63,0x03,0x06,0x06,0x0C,0x0C,0x18,0x18,0x18,0x00,0x00,0x00,0x00, // 7
	0x00,0x00,0x3E,0x63,0x63,0x63,0x3E,0x63,0x63,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // 8
	0x00,0x00,0x3E,0x63,0x63,0x63,0x63,0x3F,0x03,0x03,0x06,0x3C,0x00,0x00,0x00,0x00, // 9
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00, // :
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,0x00, // ;
	0x00,0x00,0x00,0x06,0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x06,0x00,0x00,0x00,0x00, // <
	0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00, // =
	0x00,0x00,0x00,0x60,0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x60,0x00,0x00,0x00,0x00, // >
	0x00,0x00,0x3E,0x63,0x63,0x06,0x0C,0x0C,0x0C,0x00,0x0C,0x0C,0x00,0x00,0x00,0x00, // ?
	0x00,0x00,0x3E,0x63,0x63,0x6F,0x6B,0x6B,0x6E,0x60,0x60,0x3E,0x00,0x00,0x00,0x00, // @ 0x40
	0x00,0x00,0x08,0x1C,0x36,0x63,0x63,0x63,0x7F,0x63,0x63,0x63,0x00,0x00,0x00,0x00, // A
	0x00,0x00,0x7E,0x33,0x33,0x33,0x3E,0x33,0x33,0x33,0x33,0x7E,0x00,0x00,0x00,0x00, // B
	0x00,0x00,0x1E,0x33,0x61,0x60,0x60,0x60,0x60,0x61,0x33,0x1E,0x00,0x00,0x00,0x00, // C
	0x00,0x00,0x7C,0x36,0x33,0x33,0x33,0x33,0x33,0x33,0x36,0x7C,0x00,0x00,0x00,0x00, // D
	0x00,0x00,0x7F,0x33,0x31,0x34,0x3C,0x34,0x30,0x31,0x33,0x7F,0x00,0x00,0x00,0x00, // E
	0x00,0x00,0x7F,0x33,0x31,0x34,0x3C,0x34,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00, // F
	0x00,0x00,0x1E,0x33,0x61,0x60,0x60,0x6F,0x63,0x63,0x37,0x1D,0x00,0x00,0x00,0x00, // G
	0x00,0x00,0x63,0x63,0x63,0x63,0x7F,0x63,0x63,0x63,0x63,0x63,0x00,0x00,0x00,0x00, // H
	0x00,0x00,0x3C,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00, // I
	0x00,0x00,0x0F,0x06,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,0x00,0x00,0x00, // J
	0x00,0x00,0x73,0x33,0x36,0x36,0x3C,0x36,0x36,0x33,0x33,0x73,0x00,0x00,0x00,0x00, // K
	0x00,0x00,0x78,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0x33,0x7F,0x00,0x00,0x00,0x00, // L
	0x00,0x00,0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x63,0x63,0x63,0x00,0x00,0x00,0x00, // M
	0x00,0x00,0x63,0x63,0x73,0x7B,0x7F,0x6F,0x67,0x63,0x63,0x63,0x00,0x00,0x00,0x00, // N
	0x00,0x00,0x1C,0x36,0x63,0x63,0x63,0x63,0x63,0x63,0x36,0x1C,0x00,0x00,0x00,0x00, // O
	0x00,0x00,0x7E,0x33,0x33,0x33,0x3E,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00, // P 0x50
	0x00,0x00,0x3E,0x63,0x63,0x63,0x63,0x63,0x63,0x6B,0x6F,0x3E,0x06,0x07,0x00,0x00, // Q
	0x00,0x00,0x7E,0x33,0x33,0x33,0x3E,0x36,0x36,0x33,0x33,0x73,0x00,0x00,0x00,0x00, // R
	0x00,0x00,0x3E,0x63,0x63,0x30,0x1C,0x06,0x03,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // S
	0x00,0x00,0xFF,0xDB,0x99,0x18,0x18,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00, // T
	0x00,0x00,0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // U
	0x00,0x00,0x63,0x63,0x63,0x63,0x63,0x63,0x63,0x36,0x1C,0x08,0x00,0x00,0x00,0x00, // V
	0x00,0x00,0x63,0x63,0x63,0x63,0x63,0x6B,0x6B,0x7F,0x36,0x36,0x00,0x00,0x00,0x00, // W
	0x00,0x00,0xC3,0xC3,0x66,0x3C,0x18,0x18,0x3C,0x66,0xC3,0xC3,0x00,0x00,0x00,0x00, // X
	0x00,0x00,0xC3,0xC3,0xC3,0x66,0x3C,0x18,0x18,0x18,0x18,0x3C,0x00,0x00,0x00,0x00, // Y
	0x00,0x00,0x7F,0x63,0x43,0x06,0x0C,0x18,0x30,0x61,0x63,0x7F,0x00,0x00,0x00,0x00, // Z
	0x00,0x00,0x3C,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3C,0x00,0x00,0x00,0x00, // [
	0x00,0x00,0x80,0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x07,0x03,0x01,0x00,0x00,0x00,0x00, // \ (back slash)
	0x00,0x00,0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00,0x00,0x00,0x00, // ]
	0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00, // _
	0x18,0x18,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ` 0x60
	0x00,0x00,0x00,0x00,0x00,0x3C,0x46,0x06,0x3E,0x66,0x66,0x3B,0x00,0x00,0x00,0x00, // a
	0x00,0x00,0x70,0x30,0x30,0x3C,0x36,0x33,0x33,0x33,0x33,0x6E,0x00,0x00,0x00,0x00, // b
	0x00,0x00,0x00,0x00,0x00,0x3E,0x63,0x60,0x60,0x60,0x63,0x3E,0x00,0x00,0x00,0x00, // c
	0x00,0x00,0x0E,0x06,0x06,0x1E,0x36,0x66,0x66,0x66,0x66,0x3B,0x00,0x00,0x00,0x00, // d
	0x00,0x00,0x00,0x00,0x00,0x3E,0x63,0x63,0x7E,0x60,0x63,0x3E,0x00,0x00,0x00,0x00, // e
	0x00,0x00,0x1C,0x36,0x32,0x30,0x7C,0x30,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00, // f
	0x00,0x00,0x00,0x00,0x00,0x3B,0x66,0x66,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00,0x00, // g
	0x00,0x00,0x70,0x30,0x30,0x36,0x3B,0x33,0x33,0x33,0x33,0x73,0x00,0x00,0x00,0x00, // h
	0x00,0x00,0x0C,0x0C,0x00,0x1C,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00, // i
	0x00,0x00,0x06,0x06,0x00,0x0E,0x06,0x06,0x06,0x06,0x06,0x66,0x66,0x3C,0x00,0x00, // j
	0x00,0x00,0x70,0x30,0x30,0x33,0x33,0x36,0x3C,0x36,0x33,0x73,0x00,0x00,0x00,0x00, // k
	0x00,0x00,0x1C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x1E,0x00,0x00,0x00,0x00, // l
	0x00,0x00,0x00,0x00,0x00,0x6E,0x7F,0x6B,0x6B,0x6B,0x6B,0x6B,0x00,0x00,0x00,0x00, // m
	0x00,0x00,0x00,0x00,0x00,0x6E,0x33,0x33,0x33,0x33,0x33,0x33,0x00,0x00,0x00,0x00, // n
	0x00,0x00,0x00,0x00,0x00,0x3E,0x63,0x63,0x63,0x63,0x63,0x3E,0x00,0x00,0x00,0x00, // o
	0x00,0x00,0x00,0x00,0x00,0x6E,0x33,0x33,0x33,0x33,0x3E,0x30,0x30,0x78,0x00,0x00, // p 0x70
	0x00,0x00,0x00,0x00,0x00,0x3B,0x66,0x66,0x66,0x66,0x3E,0x06,0x06,0x0F,0x00,0x00, // q
	0x00,0x00,0x00,0x00,0x00,0x6E,0x3B,0x33,0x30,0x30,0x30,0x78,0x00,0x00,0x00,0x00, // r
	0x00,0x00,0x00,0x00,0x00,0x3E,0x63,0x38,0x0E,0x03,0x63,0x3E,0x00,0x00,0x00,0x00, // s
	0x00,0x00,0x08,0x18,0x18,0x7E,0x18,0x18,0x18,0x18,0x1B,0x0E,0x00,0x00,0x00,0x00, // t
	0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x3B,0x00,0x00,0x00,0x00, // u
	0x00,0x00,0x00,0x00,0x00,0x63,0x63,0x36,0x36,0x1C,0x1C,0x08,0x00,0x00,0x00,0x00, // v
	0x00,0x00,0x00,0x00,0x00,0x63,0x63,0x63,0x6B,0x6B,0x7F,0x36,0x00,0x00,0x00,0x00, // w
	0x00,0x00,0x00,0x00,0x00,0x63,0x36,0x1C,0x1C,0x1C,0x36,0x63,0x00,0x00,0x00,0x00, // x
	0x00,0x00,0x00,0x00,0x00,0x63,0x63,0x63,0x63,0x63,0x3F,0x03,0x06,0x3C,0x00,0x00, // y
	0x00,0x00,0x00,0x00,0x00,0x7F,0x66,0x0C,0x18,0x30,0x63,0x7F,0x00,0x00,0x00,0x00, // z
	0x00,0x00,0x0E,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x18,0x0E,0x00,0x00,0x00,0x00, // {
	0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x18,0x00,0x00,0x00, // |
	0x00,0x00,0x70,0x18,0x18,0x18,0x0E,0x18,0x18,0x18,0x18,0x70,0x00,0x00,0x00,0x00, // }
	0x00,0x00,0x3B,0x6E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ~
	0x00,0x70,0xD8,0xD8,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // DEL
*/
const unsigned char FONT8x8[97][8] = {
	0x08,0x08,0x08,0x00,0x00,0x00,0x00,0x00, // columns, rows, num_bytes_per_char
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // space 0x20
	0x30,0x78,0x78,0x30,0x30,0x00,0x30,0x00, // !
	0x6C,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00, // "
	0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00, // #
	0x18,0x3E,0x60,0x3C,0x06,0x7C,0x18,0x00, // $
	0x00,0x63,0x66,0x0C,0x18,0x33,0x63,0x00, // %
	0x1C,0x36,0x1C,0x3B,0x6E,0x66,0x3B,0x00, // &
	0x30,0x30,0x60,0x00,0x00,0x00,0x00,0x00, // '
	0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00, // (
	0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00, // )
	0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00, // *
	0x00,0x30,0x30,0xFC,0x30,0x30,0x00,0x00, // +
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x30, // ,
	0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00, // -
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00, // .
	0x03,0x06,0x0C,0x18,0x30,0x60,0x40,0x00, // / (forward slash)
	0x3E,0x63,0x63,0x6B,0x63,0x63,0x3E,0x00, // 0 0x30
	0x18,0x38,0x58,0x18,0x18,0x18,0x7E,0x00, // 1
	0x3C,0x66,0x06,0x1C,0x30,0x66,0x7E,0x00, // 2
	0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00, // 3
	0x0E,0x1E,0x36,0x66,0x7F,0x06,0x0F,0x00, // 4
	0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00, // 5
	0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00, // 6
	0x7E,0x66,0x06,0x0C,0x18,0x18,0x18,0x00, // 7
	0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00, // 8
	0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00, // 9
	0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00, // :
	0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x30, // ;
	0x0C,0x18,0x30,0x60,0x30,0x18,0x0C,0x00, // <
	0x00,0x00,0x7E,0x00,0x00,0x7E,0x00,0x00, // =
	0x30,0x18,0x0C,0x06,0x0C,0x18,0x30,0x00, // >
	0x3C,0x66,0x06,0x0C,0x18,0x00,0x18,0x00, // ?
	0x3E,0x63,0x6F,0x69,0x6F,0x60,0x3E,0x00, // @ 0x40
	0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00, // A
	0x7E,0x33,0x33,0x3E,0x33,0x33,0x7E,0x00, // B
	0x1E,0x33,0x60,0x60,0x60,0x33,0x1E,0x00, // C
	0x7C,0x36,0x33,0x33,0x33,0x36,0x7C,0x00, // D
	0x7F,0x31,0x34,0x3C,0x34,0x31,0x7F,0x00, // E
	0x7F,0x31,0x34,0x3C,0x34,0x30,0x78,0x00, // F
	0x1E,0x33,0x60,0x60,0x67,0x33,0x1F,0x00, // G
	0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00, // H
	0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00, // I
	0x0F,0x06,0x06,0x06,0x66,0x66,0x3C,0x00, // J
	0x73,0x33,0x36,0x3C,0x36,0x33,0x73,0x00, // K
	0x78,0x30,0x30,0x30,0x31,0x33,0x7F,0x00, // L
	0x63,0x77,0x7F,0x7F,0x6B,0x63,0x63,0x00, // M
	0x63,0x73,0x7B,0x6F,0x67,0x63,0x63,0x00, // N
	0x3E,0x63,0x63,0x63,0x63,0x63,0x3E,0x00, // O
	0x7E,0x33,0x33,0x3E,0x30,0x30,0x78,0x00, // P 0x50
	0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00, // Q
	0x7E,0x33,0x33,0x3E,0x36,0x33,0x73,0x00, // R
	0x3C,0x66,0x30,0x18,0x0C,0x66,0x3C,0x00, // S
	0x7E,0x5A,0x18,0x18,0x18,0x18,0x3C,0x00, // T
	0x66,0x66,0x66,0x66,0x66,0x66,0x7E,0x00, // U
	0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00, // V
	0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00, // W
	0x63,0x63,0x36,0x1C,0x1C,0x36,0x63,0x00, // X
	0x66,0x66,0x66,0x3C,0x18,0x18,0x3C,0x00, // Y
	0x7F,0x63,0x46,0x0C,0x19,0x33,0x7F,0x00, // Z
	0x3C,0x30,0x30,0x30,0x30,0x30,0x3C,0x00, // [
	0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x00, // \ (back slash)
	0x3C,0x0C,0x0C,0x0C,0x0C,0x0C,0x3C,0x00, // ]
	0x08,0x1C,0x36,0x63,0x00,0x00,0x00,0x00, // ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF, // _
	0x18,0x18,0x0C,0x00,0x00,0x00,0x00,0x00, // ` 0x60
	0x00,0x00,0x3C,0x06,0x3E,0x66,0x3B,0x00, // a
	0x70,0x30,0x3E,0x33,0x33,0x33,0x6E,0x00, // b
	0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00, // c
	0x0E,0x06,0x3E,0x66,0x66,0x66,0x3B,0x00, // d
	0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00, // e
	0x1C,0x36,0x30,0x78,0x30,0x30,0x78,0x00, // f
	0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x7C, // g
	0x70,0x30,0x36,0x3B,0x33,0x33,0x73,0x00, // h
	0x18,0x00,0x38,0x18,0x18,0x18,0x3C,0x00, // i
	0x06,0x00,0x06,0x06,0x06,0x66,0x66,0x3C, // j
	0x70,0x30,0x33,0x36,0x3C,0x36,0x73,0x00, // k
	0x38,0x18,0x18,0x18,0x18,0x18,0x3C,0x00, // l
	0x00,0x00,0x66,0x7F,0x7F,0x6B,0x63,0x00, // m
	0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00, // n
	0x00,0x00,0x3C,0x66,0x66,0x66,0x3C,0x00, // o
	0x00,0x00,0x6E,0x33,0x33,0x3E,0x30,0x78, // p 0x70
	0x00,0x00,0x3B,0x66,0x66,0x3E,0x06,0x0F, // q
	0x00,0x00,0x6E,0x3B,0x33,0x30,0x78,0x00, // r
	0x00,0x00,0x3E,0x60,0x3C,0x06,0x7C,0x00, // s
	0x08,0x18,0x3E,0x18,0x18,0x1A,0x0C,0x00, // t
	0x00,0x00,0x66,0x66,0x66,0x66,0x3B,0x00, // u
	0x00,0x00,0x66,0x66,0x66,0x3C,0x18,0x00, // v
	0x00,0x00,0x63,0x6B,0x7F,0x7F,0x36,0x00, // w
	0x00,0x00,0x63,0x36,0x1C,0x36,0x63,0x00, // x
	0x00,0x00,0x66,0x66,0x66,0x3E,0x06,0x7C, // y
	0x00,0x00,0x7E,0x4C,0x18,0x32,0x7E,0x00, // z
	0x0E,0x18,0x18,0x70,0x18,0x18,0x0E,0x00, // {
	0x0C,0x0C,0x0C,0x00,0x0C,0x0C,0x0C,0x00, // |
	0x70,0x18,0x18,0x0E,0x18,0x18,0x70,0x00, // }
	0x3B,0x6E,0x00,0x00,0x00,0x00,0x00,0x00, // ~
	0x1C,0x36,0x36,0x1C,0x00,0x00,0x00,0x00}; // DEL	  


