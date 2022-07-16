/*******************************************************************
** File        : XE1203driver.C                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by   : Miguel Luis & Gr�goire Guye                     **
**                                                                **
** Date        : 28-03-2003                                       **
**                                                                **
** Project     : API-1203                                         **
**                                                                **
********************************************************************
** Changes     : V 2.0 / MiL - 09-12-2003                         **
**                - RF frame format changed                       **
**                - Most functions modified to be more flexible   **
**                - Add BitJockey / Non BitJockey compatibility   **
**                                                                **
** Changes     : V 2.1 / MiL - 24-04-2004                         **
**               - No changes                                     **
**                                                                **
********************************************************************
** Description : XE1203 transceiver drivers                    	  **
************************x*******************************************/

/*******************************************************************
** Include files                                                  **
*******************************************************************/
#include <stm32/gpio.h>
#include <stm32/usart.h>
#include <stm32/rcc.h>
#include <cmsis/stm32.h>
#include <stm32/flash.h>
#include <stm32/misc.h>
#include <stm32/adc.h>
#include <stm32/tim.h>
#include <stm32/spi.h>
#include <stm32/dac.h>
#include <cmsis/stm32.h>

#include "stm32f10x_it.h"
#include "XE1203Driver.h"

//#include <stm32/spi.h>
//#include "Initialisation.h"
//#include <stm32/tim.h>
#define SPI
#define CORTEX

//#include <cmsis/stm32.h>

//extern volatile unsigned short freq_porteuse_Save;
/*******************************************************************
** Global variables                                               **
*******************************************************************/
volatile  _U16 ByteCounter = 0;       // RF frame byte counter
volatile  _U8 PreMode = RF_SW_SLEEP; // Previous chip operating mode
volatile _U8 EnableSyncByte = false; // Enables/disables the synchronization byte reception/transmission
volatile   _U8 SyncByte;              // RF synchronization byte counter
volatile   _U8 PatternSize = 1;       // Size of pattern detection
volatile   _U8 StartByte[4];          // Pattern detection values
//volatile _U8 CCR0ReachedWait;     	// Interruption from timer A according the waiting time programmed
//volatile _U8 RFBaudrateIntTx;     	//RFBaudrate interrupt for Tx 
//volatile _U8 RFFrameTimeoutIntRx; 	//RFFrameTimeout interrupt for Rx 
//volatile _U8 PatternOn, DataOn;     //Detection of the pattern and of datas

static  _U32 RFFrameTimeOut = RF_FRAME_TIMEOUT(4800); // Reception counter value (full frame timeout generation)
static  _U32 RFBaudrate = TX_BAUDRATE_GEN_4800;       // Transmission counter value (baudrate generation)

_U16 RegistersCfg[] = { // 1203 configuration registers values
    //CongigSwitch Register [0]
    DEF_CONFIG | RF_CONFIG_MODE1,
    //RTParam Configuration [0x00001]
    DEF_RTPARAM1 | RF_RT1_BIT_SYNC_ON| RF_RT1_BARKER_OFF | RF_RT1_RSSI_OFF | RF_RT1_RSSIR_LOW | RF_RT1_FEI_OFF | RF_RT1_BW_200 | RF_RT1_OSC_INT | RF_RT1_CLKOUT_OFF,
    DEF_RTPARAM2 | RF_RT2_STAIR_10 | RF_RT2_FILTER_ON | RF_RT2_MODUL_OFF  | RF_RT2_IQAMP_ON |  RF_RT2_SWITCH_PAD   | RF_RT2_PATTERN_ON | RF_RT2_BAND_433,
    // FSParam Configuration Register [0x00011-0x00101]
    DEF_FSPARAM1 | RF_FS1_FDEV_00,//RF_FS1_FDEV_00 saw
    //152340/(RF_FS2_BAUDRATE_4800(0x1F)+1) calcule de baudrate 
    DEF_FSPARAM2 | RF_FS2_CHANGE_OSR_OFF |RF_FS2_BAUDRATE_4800,
    DEF_FSPARAM3 | RF_FS3_NORMAL,// NON KONNEX

    //SWParam Configuration Register [0x00110-0x01011]
    DEF_SWPARAM1 | RF_SW_RECEIVER | RF_SW_POWER_0 | RF_SW_RMODE_MODE_A, //[00110]
    DEF_SWPARAM2 ,//[00111]	
    DEF_SWPARAM3 ,//[01000]
    DEF_SWPARAM4 | RF_SW_POWER_10 | RF_SW_RMODE_MODE_A |RF_SW_TRANSMITTER ,//RF_SW_STANDBY ,//RF_SW_SLEEP ,// ,//[01001]
    DEF_SWPARAM5,//[001010]
    DEF_SWPARAM6,//[001011] le suivant <REG_DATAOUT1 adress +2 donc  //[01110]
    DEF_ADPARAM1 | RF_AD1_P_SIZE_32| RF_AD1_P_TOL_0 | RF_AD1_CLK_FREQ_1_22_MHZ | RF_AD1_INVERT_OFF | RF_AD1_REG_BW_ON,//[01111]
    DEF_ADPARAM2 | RF_AD2_REG_FREQ_START_UP | RF_AD2_REG_COND_ON | RF_AD2_X_SEL_07_PF | RF_AD2_RES_X_OSC_3800 | RF_AD2_KONNEX_OFF,//[10000]
    DEF_ADPARAM3 | RF_AD3_CHG_THRES_OFF,//[10001]
    DEF_ADPARAM4 | RF_AD4_DATA_BIDIR_OFF,//[10001]
    //PATTERN register
    DEF_ADPARAM5,//[10010]
    DEF_PATTERN1 | 0xB9,//[10011]//
    DEF_PATTERN2 | 0xAC,//[10100]
    DEF_PATTERN3 | 0x03,//[10101]
    DEF_PATTERN4 | 0x04,//[10110]
};

/*void set_bit(unsigned char port, unsigned char bit)
{	port|=bit;}
void clear_bit(unsigned char port, unsigned char bit)
{	port&=~bit;}
unsigned char check_bit(unsigned char port, unsigned char bit);
{	return(port&=bit);}*/
/*******************************************************************
** Configuration functions                                        **
*******************************************************************/

/*******************************************************************
** InitRFChip : Initializes the RF Chip registers using           **
**            pre initialized global variable                     **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitRFChip (void){
    _U16 i;

    
    //Configuration of the SPI and control signal between the XE1203 and the uc
    SrtInit();

    // Initializes the registers of the XE1203
    for(i = 0; (i + 2) <= REG_PATTERN4 + 1; i++)
    {
        if(i < REG_DATAOUT1){
            WriteRegister(i, RegistersCfg[i]);
        }
        else{ //go over "read" status register of the XE1203
            WriteRegister(i + 2, RegistersCfg[i]);
        }
    }
    
    //Invert bytes of the pattern
    PatternSize = ((RegistersCfg[REG_ADPARAM1 - 2] >> 6) & 0x03) + 1;
    for(i = 0; i < PatternSize; i++)
    {
        StartByte[i] = /*InvertByte*/(RegistersCfg[REG_PATTERN1 - 2 + i]);
    }
    
    //Value to initiate the Timer regarding the baud rate
    if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_1200)
    {
        RFBaudrate = TX_BAUDRATE_GEN_1200;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(1200);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_2400)
    {
        RFBaudrate = TX_BAUDRATE_GEN_2400;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(2400);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_4800)
    {
        RFBaudrate = TX_BAUDRATE_GEN_4800;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(4800);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_9600)
    {
        RFBaudrate = TX_BAUDRATE_GEN_9600;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(9600);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_19200)
    {
        RFBaudrate = TX_BAUDRATE_GEN_19200;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(19200);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_38400)
    {
        RFBaudrate = TX_BAUDRATE_GEN_38400;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(38400);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_76800)
    {
        RFBaudrate = TX_BAUDRATE_GEN_76800;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(76800);
    }
    else if(RegistersCfg[REG_FSPARAM2] == RF_FS2_BAUDRATE_153600)
    {
        RFBaudrate = TX_BAUDRATE_GEN_153600;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(153600);
    }
    else 
    {
        RFBaudrate = TX_BAUDRATE_GEN_1200;
        RFFrameTimeOut = RF_FRAME_TIMEOUT(1200);
    }
}

/*******************************************************************
** WriteRegister : Writes the register value at the given address **
**                  on the XE1203 SPI sur cortex                  **
********************************************************************
** In  : address, value                                           **
** Out : -                                                        **
*******************************************************************/
#ifdef SPI
void WriteRegister(_U8 address, _U16 value){
    _U8 i;
    unsigned char addr=0;
   
     clear_bit(GPIOB, (EN));

   // P1OUT   = 0x01; 
    //SrtInit();
   
        addr=0x80+(address & 0x1F);
    // Write Address
    spi_put(addr);
    // Write value
    spi_put(value);
     spi_put(0xFF);
  //  spi_put(0xFF);
for(i=0;i<6;i++){};
 // for(i=0;i<16;i++){};
   // for(i=0;i<16;i++){};
    // for(i=0;i<16;i++){};
    // for(i=0;i<16;i++){};
    set_bit(GPIOB, EN);

/*comm_puts("addr: ");
    writeHEXi(addr);
 comm_puts(" "); 
       comm_puts("val: ");
    writeHEXi(value);
 comm_puts("\r\n ");    
    
  */ 
}
#endif


/*******************************************************************
** ReadRegister : Reads the register value at the given address on**
**                the XE1203                                      **
********************************************************************
** In  : address                                                  **
** Out : value                                                    **
*******************************************************************/
#ifdef SPI
_U16 ReadRegister(_U8 address){
    _U8 i;
    _U8 value = 0;
    
   set_bit(GPIOB, SI+SCK);
    
    unsigned char addr=0;
    clear_bit(GPIOB, EN);
    addr=0xA0+(address & 0x1F);

    // Write Address
    spi_put(addr);
   
    
    // Write value
    spi_put(0xFF);
    	 while((SPI2->SR & (1))==0);
	 value= SPI2->DR;
    
  //  spi_put(0xFF);
  // Attente sinon le EN est trop court
   for(i=0;i<4;i++){};
 //  for(i=0;i<16;i++){};
    //for(i=0;i<16;i++){};
   //  for(i=0;i<16;i++){};
    set_bit(GPIOB, EN);
    
    

    
 
    
    return value;
}
#endif



/*******************************************************************
** SetRFMode : Sets the XE1203 operating mode (Sleep, Receiver,   **
**           Transmitter)                                         **
********************************************************************
** In  : mode & XE1203 Numéro du module 1/0                       **
** Out : -                                                        **
*******************************************************************/
void SetRFMode(_U8 mode){
    _U8 chipConfig;

    if(mode != PreMode)
    {
        chipConfig = RegistersCfg[REG_CONFIG];
        if(!(chipConfig & RF_CONFIG_MASK))
        {//config1
            if((mode == RF_SW_TRANSMITTER) && (PreMode == RF_SW_SLEEP))
            {
                PreMode = RF_SW_TRANSMITTER;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_STANDBY);
                // wait TS_OS
                //wait(TS_OS);
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_TRANSMITTER);

                set_bit(ANT_SWITCH0, TX);
                clear_bit(ANT_SWITCH0, RX);

                // wait TS_TR
                //wait(TS_TR);
            }
            else if((mode == RF_SW_TRANSMITTER) && (PreMode == RF_SW_RECEIVER))
            {
                PreMode = RF_SW_TRANSMITTER;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_TRANSMITTER);

                set_bit(ANT_SWITCH0, TX);
                clear_bit(ANT_SWITCH0, RX);

                // //wait TS_TR
                //wait(TS_TR);
            }
            else if((mode == RF_SW_RECEIVER) && (PreMode == RF_SW_SLEEP))
            { 
                PreMode = RF_SW_RECEIVER;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_STANDBY);
                // //wait TS_OS
                //wait(TS_OS);
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_RECEIVER);

                set_bit(ANT_SWITCH0, RX);
                clear_bit(ANT_SWITCH0, TX);

                // //wait TS_RE
                //wait(TS_RE);
            }
            else if((mode == RF_SW_RECEIVER) && (PreMode == RF_SW_TRANSMITTER))
            {
                PreMode = RF_SW_RECEIVER;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_RECEIVER);

                set_bit(ANT_SWITCH0, RX);
                clear_bit(ANT_SWITCH0, TX);

                set_bit(ANT_SWITCH1, RX);
                clear_bit(ANT_SWITCH1, TX);		  

                // //wait TS_RE
                //wait(TS_RE);
            }
            else if(mode == RF_SW_SLEEP)
            {
                PreMode = RF_SW_SLEEP;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_SLEEP);

                clear_bit(ANT_SWITCH0, (RX+TX));

            }
            else
            {
                PreMode = RF_SW_SLEEP;
                WriteRegister(REG_SWPARAM1, (RegistersCfg[REG_SWPARAM1] & 0x3F) | RF_SW_SLEEP);

                clear_bit(ANT_SWITCH0, (RX+TX));

            }
        }
        else
        {//config 2
            if((mode == RF_SW_TRANSMITTER) && (PreMode == RF_SW_SLEEP))
            {
                PreMode = RF_SW_TRANSMITTER;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_STANDBY);
                // //wait TS_OS
                //wait(TS_OS);
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_TRANSMITTER);

                set_bit(ANT_SWITCH0, TX);
                clear_bit(ANT_SWITCH0, RX);

                // //wait TS_TR
                //wait(TS_TR);
            }
            else if((mode == RF_SW_TRANSMITTER) && (PreMode == RF_SW_RECEIVER))
            {
                PreMode = RF_SW_TRANSMITTER;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_TRANSMITTER);

		set_bit(ANT_SWITCH0, TX);
                clear_bit(ANT_SWITCH0, RX);
		  
                // //wait TS_TR
                //wait(TS_TR);
            }
            else if((mode == RF_SW_RECEIVER) && (PreMode == RF_SW_SLEEP))
            {
                PreMode = RF_SW_RECEIVER;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_STANDBY);
                // //wait TS_OS
                //wait(TS_OS);
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_RECEIVER);

                set_bit(ANT_SWITCH0, RX);
                clear_bit(ANT_SWITCH0, TX);

                // //wait TS_RE
                //wait(TS_RE);
            }
            else if((mode == RF_SW_RECEIVER) && (PreMode == RF_SW_TRANSMITTER))
            {
                PreMode = RF_SW_RECEIVER;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_RECEIVER);

                set_bit(ANT_SWITCH0, RX);
                clear_bit(ANT_SWITCH0, TX);

                // //wait TS_RE
                //Wait(TS_RE);
            }
            else if(mode == RF_SW_SLEEP)
            {
                PreMode = RF_SW_SLEEP;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_SLEEP);

                clear_bit(ANT_SWITCH0, (RX+TX));

            }
            else
            {
                PreMode = RF_SW_SLEEP;
                WriteRegister(REG_SWPARAM4, (RegistersCfg[REG_SWPARAM4] & 0x3F) | RF_SW_SLEEP);

                clear_bit(ANT_SWITCH0, (RX+TX));

            }
        }
    }
} // void SetRFMode(_U8 mode)

/*******************************************************************
** WaitSCK : Used as a temporisation for the spi                 **
** in other to have SCK < 1 MHz                                   **
**                                                                **
********************************************************************
** In  :                                                      **
** Out : -                                                        **
*******************************************************************/

void WaitSCK(void)
{
  _U8 i;
  //4-> 50 us msp
  //4-> 1MHz aduc
  for (i=0; i < 10 ; i++){}
}


/*******************************************************************
** Communication functions                                        **
*******************************************************************/
void envoie_porteuse( _U16 donnee_envoye)
{
	_U8 tronc_freq;	
	
	tronc_freq=(_U8)((donnee_envoye&0xFF00)>>8);
	//if(tronc_freq!=(_U8)((freq_porteuse_Save&0xFF00)>>8))
	//{
			WriteRegister(SWParam_Freq_1,tronc_freq );
			WriteRegister(SWParam_Freq_2,tronc_freq );
			//}
			//tronc_freq=(_U8)((donnee_envoye&0xFF00)>>8);
			//if(tronc_freq!=(_U8)((freq_porteuse_Save&0x00FF)))
			//{
			tronc_freq=(_U8)((donnee_envoye&0x00FF));
			WriteRegister(SWParam_Freq_1+1,tronc_freq );
			WriteRegister(SWParam_Freq_2+1,tronc_freq );
			//}
			//freq_porteuse_Save=donnee_envoye;

}


/*******************************************************************
** InvertByte : Inverts a byte. MSB -> LSB, LSB -> MSB            **
********************************************************************
** In  : b                                                        **
** Out : b                                                        **
*******************************************************************/
_U8 InvertByte(_U8 b){

    _U8 b_invert=0x00;
    int j;
    
    for (j=7; j>= 0; j=j-1)
    {
        b_invert|=(((b<<(7-j))&0x80)>>j);
    }
    return b_invert;
    
}// _U8 InvertByte(_U8 b)



