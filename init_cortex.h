//#include "stm32f10x.h"     // definition uint32_t, added 150116
#include <stdint.h>

/*
#include <libopencm3/cm3/common.h> // BEGIN_DECL,          added 150116
#include <libopencm3/stm32/f1/memorymap.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
*/

#define readerCS_SET    GPIO_SetBits  (GPIOA,GPIO_Pin_13)
#include <stm32/gpio.h>
#include <stm32/usart.h>
#include <stm32/rcc.h>
#include <stm32/flash.h>
#include <stm32/misc.h>
#include <stm32/adc.h>
#include <stm32/tim.h>
#include <stm32/spi.h>
#include <stm32/dac.h>

#define readerCS_CLR    GPIO_ResetBits(GPIOA,GPIO_Pin_13)
#define readerIOUP_SET  GPIO_SetBits  (GPIOA,GPIO_Pin_14)
#define readerIOUP_CLR  GPIO_ResetBits(GPIOA,GPIO_Pin_14)
#define readerRESET_SET GPIO_SetBits  (GPIOA,GPIO_Pin_15)
#define readerRESET_CLR GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define readerF1_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_10)
#define readerF1_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_10)
#define readerF2_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define readerF2_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_9)
#define readerFD_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_11)
#define readerFD_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_11)
#define readerallSET    GPIO_SetBits  (GPIOB,GPIO_Pin_11|GPIO_Pin_9|GPIO_Pin_10)
// HMC349 : lo donne RFC=RF2 et hi donne RFC=RF1

#define alim5p0V GPIO_SetBits(GPIOB,GPIO_Pin_7)	// ON if hi, OFF if lo
#define alim1p8V GPIO_SetBits(GPIOB,GPIO_Pin_8)	// ON if hi, OFF if lo
#define alimDDS  GPIO_ResetBits(GPIOB,GPIO_Pin_6)	// ON if lo, OFF if hi (DDS PWR_DWN)

#ifdef MODBUS 
#include "port.h"
#include "mbport.h"
#include "mb.h"

#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

void vMBPortSerialEnable(BOOL , BOOL );
BOOL xMBPortSerialInit( UCHAR , ULONG , UCHAR , eMBParity );
BOOL xMBPortSerialPutByte( CHAR );
BOOL xMBPortSerialGetByte( CHAR * );
void EnterCriticalSection( void );
void ExitCriticalSection( void );

#define MB_TIMER_TICKS          ( 10L )

BOOL xMBPortTimersInit( USHORT );
void vMBPortTimersEnable( void ) ;

void vMBPortTimersDisable( void );
eMBErrorCode eMBRegInputCB( UCHAR * , USHORT , USHORT );
eMBErrorCode eMBRegHoldingCB( UCHAR * , USHORT , USHORT , eMBRegisterMode);
eMBErrorCode eMBRegCoilsCB( UCHAR * , USHORT , USHORT , eMBRegisterMode );
eMBErrorCode eMBRegDiscreteCB( UCHAR * , USHORT , USHORT );
int my_getchar (void);
#endif

void Timer3_Configuration (void);
int spi_put1 (int );
int spi_put2 (int );
unsigned char getResponse (void);
/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration (void);
void NVIC_Configuration (void);
void Delay (uint32_t nCount);
void RTC_GetTime (uint32_t * THH, uint32_t * TMM, uint32_t * TSS);
void RTC_Init (void);
void initADC (void);
/* Private functions ---------------------------------------------------------*/

void initADC ();
uint32_t readADC12 ();
uint16_t readADC1 (uint8_t );
void TIM2_Init (void);
void dac2_init ();
void dac1_init ();
void init_CPU (void);

#ifdef stm32_usb
static int cdcacm_control_request(usbd_device *, struct usb_setup_data *, uint8_t **, uint16_t *, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req));
static void cdcacm_data_rx_cb(usbd_device *, uint8_t );
static void cdcacm_set_config(usbd_device *, uint16_t );
void usb_comm();
void go_usb();
#endif

void RCC_Configuration (void);
void Delay (uint32_t nCount);

#ifdef  DEBUG
void assert_failed (u8 * , u32 );
#endif 

void corrige_offset (unsigned int *);
void delay (int );
unsigned short temperature (void);
void eeprom_erase (int );
void eeprom_write (int ,unsigned char *,int );
void eeprom_read (int , unsigned char *b, int );
void eeprom_init (void);
void jmf_putchar (int ,char *, int*,int );
void remont_CS_PLL ();
void envoi_PLL (unsigned char *);
void clignote();
void sortie_led (int );
void sortie_antenne (int );
void set_serial_speed (int ,int);
void set_DAC0 (int );
void set_DAC1 (int );
void set_DAC2 (int );
void set_SPI_speedDiv (int );
void sortie_courant_init ();
void sortie_courant (int );
void kick_watchdog ();
void init_CPU_specifique();
