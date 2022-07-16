// ATTENTION : la fonction temperature() ne marche pas et desactive les ADCs ?!

/////////////////////////////////////////////
// TODO : fini USB

// INCLUSION USB : 
// line 238 of .../sat/arm-none-eabi/include/cmsis/stm32.h must be commented to prevent
// conflict on the definition of a boolean type
// compilation assumes that these constants are defined
// export CORTEX_BASE_DIR="${HOME}/sat/arm-none-eabi"
// export STM32_INC=${CORTEX_BASE_DIR}/include
// export STM32_LIB=${CORTEX_BASE_DIR}/lib
// export PATH=${HOME}/sat/bin:$PATH

// pour travailler avec le cortex : #define f400, #define AD9958 <- actif dans Makefile

#include <stdlib.h>
#include <string.h>

////////////// begin libsmt32 ///

#include "DDSinc.h"
#include "DDSvar.h"
#include "init_cortex.h"

extern volatile int tim0 , heure , minute , seconde ,global_index,tim0modbus,modif_AT,proportional;
extern unsigned char global_tab[NB_CHARS];

#ifdef MODBUS
#include "config_modbus.h"

#include "port.h"
#include "mbport.h"
#include "mb.h"

#include "mbconfig.h"
#include "mbframe.h"
#include "mbproto.h"
#include "mbfunc.h"

extern USHORT   usRegInputStart;
extern USHORT   usRegInputBuf[REG_INPUT_NREGS];
extern USHORT   usRegHoldingStart;
extern USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
extern UCHAR    ucMBAddress_tmp;
extern int      rs_tmp,rs_tmp2;

void EnterCriticalSection( void ) {};
void ExitCriticalSection( void ) {};

void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{if (xTxEnable==TRUE) xMBRTUTransmitFSM();
}

BOOL
xMBPortSerialInit( UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{return TRUE;}

BOOL
xMBPortSerialPutByte( CHAR ucByte)
{jmf_putchar (ucByte,NULL,NULL,modbus_port);
 return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{ 
//  while (!(USART1_SR & USART1_SR_RXNE));
//  return ((int)(USART1_DR & 0xFF));
  if (modbus_port==1) *pucByte=USART_ReceiveData(USART1);  // declenchement sur interruption
     else *pucByte=USART_ReceiveData(USART2);
  return TRUE;
}

/* ----------------------- Static variables ---------------------------------*/
static USHORT   usTimerOCRADelta;
static USHORT   usTimerOCRBDelta;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit( USHORT usTim1Timeout50us )
{tim0modbus=0;return TRUE;}

void vMBPortTimersEnable( void ) {tim0modbus=0;}

void vMBPortTimersDisable( void ) {tim0modbus=255;}

eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {  
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {  
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8);
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {  
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

eMBErrorCode // fonction appelee par la commande MODBUS 3 (Read Holding Registers) dans mbfuncholding.c
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{   
// le bordel sur l'adresse du registre ! la valeur usAddress est incrementee de 1 par rapport a la 
//   valeur recue par eMBPoll : quelquepart (je ne sais pas ou`) FreeModbus ajoute 1 entre l'@ recue
//   dans le paquet et l'appel a cette fonction
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex,antenne;
    int 	    *pointeur;

//
write_str("Modbus",NULL,NULL,usart_port);
writeDECc(MODBUS_SLAVE_ADD,NULL,NULL,usart_port);
write_str(" request @",NULL,NULL,usart_port);
writeDECi(usAddress,NULL,NULL,usart_port);
write_str(": ",NULL,NULL,usart_port);
//
   
    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {  
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {   
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {   
                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
                iRegIndex++;
                usNRegs--;
            }
            if (iRegIndex<=(NBPICS_MAX*ANTENNES_MAX )) 
               {proportional=PROPORTIONAL_INIT+2; // nombre de mesures a faire pour raffraichir le 
                                                  // contenu de la memoire -> init que si lit T
                               // quand on depasse la zone de stockage des parametres de mesure
                // jmf_putchar('.',NULL,NULL,usart_port);delay(0xff);
               }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++<<8;
                iRegIndex++;
                usNRegs--;
            } 
            if (iRegIndex>(NBPICS_MAX*ANTENNES_MAX*REG_PAR_RESONANCE)) modif_AT=1; 
        }
    }
// les registres modbus sont sur 16 bits, les registres de calibrage sur 32
// et on reserve ANTENNES_MAX*NBPICS_MAX/2 elements dans chaque coef de cal
    else if ((usAddress >= REG_COEF_START)&&((usAddress+usNRegs)<=(REG_COEF_START+3*ANTENNES_MAX*NBPICS_MAX)))
    {
        iRegIndex = ( int )( usAddress - REG_COEF_START );
        if (iRegIndex<ANTENNES_MAX*NBPICS_MAX) pointeur=A0;
          else if (iRegIndex<2*ANTENNES_MAX*NBPICS_MAX) {pointeur=A1;iRegIndex-=ANTENNES_MAX*NBPICS_MAX;}
            else {pointeur=A2;iRegIndex-=2*ANTENNES_MAX*NBPICS_MAX;}

        switch ( eMode )
        { 
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
               // A0 est sur 32 bits, la transaction vient de se faire sur 16 bits
                if ((iRegIndex%2)==0) 
                   {*pucRegBuffer++ = ( unsigned char )( (pointeur[iRegIndex/2] & 0xff0000) >> 16 );
                    *pucRegBuffer++ = ( unsigned char )( (pointeur[iRegIndex/2] & 0xFF000000)>>24 );
                   }
                else
                   {*pucRegBuffer++ = ( unsigned char )( (pointeur[iRegIndex/2] & 0xFF));
                    *pucRegBuffer++ = ( unsigned char )( (pointeur[iRegIndex/2] & 0xff00) >> 8 );
                   }
                iRegIndex++;
                usNRegs--;
                if (iRegIndex==(2*ANTENNES_MAX*NBPICS_MAX))
                   {
                    iRegIndex=0;
                    if (pointeur==A0) {pointeur=A1;} else pointeur=A2;
                   }
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                if ((iRegIndex%2)==0) 
                   {pointeur[iRegIndex/2]=((pointeur[iRegIndex/2]&0xff00ffff)|((int)*pucRegBuffer++ <<16));
                    pointeur[iRegIndex/2]=((pointeur[iRegIndex/2]&0x00ffffff)|((int)*pucRegBuffer++ <<24));
                   }
                else
                   {pointeur[iRegIndex/2]=((pointeur[iRegIndex/2]&0xffffff00)|((int)*pucRegBuffer++ ));
                    pointeur[iRegIndex/2]=((pointeur[iRegIndex/2]&0xffff00ff)|((int)*pucRegBuffer++ <<8 )); 
                   }
                iRegIndex++;
                if (iRegIndex==2*ANTENNES_MAX*NBPICS_MAX) // passe au tableau suivant
                   {iRegIndex=0;
                    if (pointeur==A0) pointeur=A1; else pointeur=A2;
                   }
                usNRegs--;
            }
        }
    }
    else if ((usAddress >= REG_CONFIG)&&((usAddress+usNRegs)<=(REG_CONFIG+3)))
    {
        iRegIndex = ( int )( usAddress - REG_CONFIG );
        switch ( eMode )
        { 
         case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
             if (iRegIndex==0) 
               {ucMBAddress_tmp = *pucRegBuffer++;ucMBAddress_tmp |= *pucRegBuffer++<<8;
//                write_str("\nslave=",NULL,NULL,usart_port); 
//                writeDECi(ucMBAddress_tmp,NULL,NULL,usart_port);
//                write_str("\r\n",NULL,NULL,usart_port);
               }
             if (iRegIndex==1) 
               {rs_tmp = *pucRegBuffer++;rs_tmp |= *pucRegBuffer++<<8;
//               write_str("\nRS232=",NULL,NULL,usart_port); 
//               writeDECi(rs_tmp,NULL,NULL,usart_port);
//               write_str("\r\n",NULL,NULL,usart_port);
               }
             if (iRegIndex==2) 
               {rs_tmp2 = *pucRegBuffer++;rs_tmp2 |= *pucRegBuffer++<<8;}
             iRegIndex++;
             usNRegs--;
            }
         break;
         case MB_REG_READ:
            while( usNRegs > 0 )
            {
             if (iRegIndex==0) 
               {*pucRegBuffer++ = ( unsigned char )( ucMBAddress_tmp & 0xFF );
                *pucRegBuffer++ = ( unsigned char )( ucMBAddress_tmp >> 8 );
               }
             if (iRegIndex==1) 
               {*pucRegBuffer++ = ( unsigned char )( rs_tmp & 0xFF );
                *pucRegBuffer++ = ( unsigned char )( rs_tmp >> 8 );
               }
             if (iRegIndex==2) 
               {*pucRegBuffer++ = ( unsigned char )( rs_tmp2 & 0xFF );
                *pucRegBuffer++ = ( unsigned char )( rs_tmp2 >> 8 );
               }
             iRegIndex++;
             usNRegs--;
            }
        }
    }
      else
       {
        eStatus = MB_ENOREG;
       }
    return eStatus;
}

eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{   
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    return MB_ENOREG;
}
#endif

void
Timer3_Configuration (void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Period = 7200 - 1;	//  745 => 0.01s
  TIM_TimeBaseStructure.TIM_Prescaler = 1000 - 1;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;	// TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit (TIM3, &TIM_TimeBaseStructure);
}

int
spi_put1 (int i)
{
  SPI1->DR = i;
  while ((SPI1->SR & (1 << 1)) == 0);	// Si TXE =1 le buff est libre
  while ((SPI1->SR & (1)) == 0);
  return (SPI1->DR);
}

int
spi_put2 (int i)
{
  SPI2->DR = i;
  while ((SPI2->SR & (1 << 1)) == 0);	// Si TXE =1 le buff est libre
  while ((SPI2->SR & (1)) == 0);
  return (SPI2->DR);
}


unsigned char
getResponse (void)
{
  unsigned char response;
  response = spi_put2 (0xFF);
  return response;
}

ErrorStatus HSEStartUpStatus;

void
initADC ()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  ADC_InitTypeDef ADC_InitStructure;

  // chan8
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	// ADC=PB0
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init (GPIOB, &GPIO_InitStructure);

  // Configure ADC1 
  ADC_DeInit (ADC1);
  ADC_DeInit (ADC2);
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_ADC2|RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 12 MHz on APB2 CK  // ajout 130919

  ADC_InitStructure.ADC_Mode = ADC_Mode_RegSimult; // ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init (ADC1, &ADC_InitStructure);

  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_Init (ADC2, &ADC_InitStructure);

  ADC_DMACmd (ADC1, ENABLE);  // DOIT etre enabled meme si on ne fait pas de
  ADC_DMACmd (ADC2, ENABLE);  // DMA RM0008 p.219

  ADC_ExternalTrigConvCmd (ADC1, ENABLE); // DOIT etre actif sur ADC12 pour recevoir
  ADC_ExternalTrigConvCmd (ADC2, ENABLE); // l'ordre de ADC1
 
  ADC_RegularChannelConfig (ADC1, ADC_Channel_8, 1, ADC_SampleTime_1Cycles5); 
  ADC_RegularChannelConfig (ADC2, ADC_Channel_9, 1, ADC_SampleTime_1Cycles5); 

  ADC_Cmd (ADC1, ENABLE);
  ADC_Cmd (ADC2, ENABLE);

  ADC_TempSensorVrefintCmd(ENABLE);

  ADC_ResetCalibration (ADC1);
  while (ADC_GetResetCalibrationStatus (ADC1));
  ADC_StartCalibration (ADC1);
  while (ADC_GetCalibrationStatus (ADC1));
  ADC_ResetCalibration (ADC2);
  while (ADC_GetResetCalibrationStatus (ADC2));
  ADC_StartCalibration (ADC2);
  while (ADC_GetCalibrationStatus (ADC2));
  ADC_Cmd (ADC1, ENABLE);
}

uint32_t
readADC12 (void)	// jmf convert ADC12: ADC1=PB0 et ADC2=PB1 en dur
{
//  ADC_Cmd (ADC1, ENABLE);
//  ADC_Cmd (ADC2, ENABLE);
//  ADC_SoftwareStartConvCmd (ADC2, ENABLE);
  ADC_SoftwareStartConvCmd (ADC1, ENABLE);
  while ((ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC) == RESET)) {} //  && (ADC_GetFlagStatus (ADC2, ADC_FLAG_EOC) == RESET));       // EOC is set @ end of cv
//   return (ADC_GetConversionValue (ADC1));

  return(ADC_GetDualModeConversionValue()); // renvoie un 32 bits, 
  // ADC2 in upper word and ADC1 in lower word, RM0008 p.223
}


uint16_t
readADC1 (__attribute__((unused))uint8_t channel)	// jmf convert ADC1
{
  // ADC_RegularChannelConfig (ADC1, channel, 1, ADC_SampleTime_1Cycles5); // ADC_SampleTime_13Cycles5);
  ADC_SoftwareStartConvCmd (ADC1, ENABLE);
  while (ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC) == RESET);	// EOC is set @ end of cv
  return (ADC_GetConversionValue (ADC1));
}

void
TIM2_Init (void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

  // PA.01 is TIM2_CH2

  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit (TIM2, &TIM_TimeBaseStructure);

  TIM_TIxExternalClockConfig (TIM2, TIM_TIxExternalCLK1Source_TI2, TIM_ICPolarity_Rising, 0);	//TI2FP2

  TIM_Cmd (TIM2, ENABLE);
}

void
dac2_init ()
{
  //Initialisation de la sortie PA5
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_DAC, ENABLE);
  DAC_InitTypeDef DAC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  //Pour PA5
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init (GPIOA, &GPIO_InitStructure);
  // Init DAC ( conf: logiciel)
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_Init (DAC_Channel_2, &DAC_InitStructure);
  //Validation du DAC
  DAC_Cmd (DAC_Channel_2, ENABLE);
}

void
dac1_init ()
{				//Initialisation de la sortie PA4
  RCC_APB1PeriphClockCmd (RCC_APB1Periph_DAC, ENABLE);
  DAC_InitTypeDef DAC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  //Pour PA4 (DAC1)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init (GPIOA, &GPIO_InitStructure);
  // Init DAC ( conf: logiciel)
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
  DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_Init (DAC_Channel_1, &DAC_InitStructure);
  //Validation du DAC
  DAC_Cmd (DAC_Channel_1, ENABLE);
}

void
init_CPU (void)
{
//  NVIC_InitTypeDef NVIC_InitStruct;
#ifdef DEBUG
  debug ();
#endif /*  */

  /* Configure the system clocks */
  RCC_Configuration ();

  /* NVIC Configuration */
  // NVIC_Configuration ();

  initADC ();

// USART configured 38400, N81, no hw flow ctrl, enable Tx & Rx
  RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA, ENABLE);
  // GPIO Config 
  GPIO_InitTypeDef gpstruct;
  // USART1_RX 
  gpstruct.GPIO_Pin = GPIO_Pin_10;
  gpstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init (GPIOA, &gpstruct);
  // USART1_TX 
  gpstruct.GPIO_Pin = GPIO_Pin_9;
  gpstruct.GPIO_Speed = GPIO_Speed_50MHz;
  gpstruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init (GPIOA, &gpstruct);
  // USART2_RX 
  gpstruct.GPIO_Pin = GPIO_Pin_3;
  gpstruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init (GPIOA, &gpstruct);
  // USART2_TX 
  gpstruct.GPIO_Pin = GPIO_Pin_2;
  gpstruct.GPIO_Speed = GPIO_Speed_50MHz;
  gpstruct.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init (GPIOA, &gpstruct);

  NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);

  set_serial_speed(RS_BAUDRATE,RS_PARITY);

  init_CPU_specifique();
#ifdef stm32_usb
// USB interrupt
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStruct.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
  
  go_usb();
#endif
}

/////////////  end libstm32 

#ifdef stm32_usb
static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x83,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x82,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"Black Sphere Technologies",
	"CDC-ACM Demo",
	"DEMO",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static int cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;

	switch (req->bRequest) {
	case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
		/*
		 * This Linux cdc_acm driver requires this to be implemented
		 * even though it's optional in the CDC spec, and we don't
		 * advertise it in the ACM functional descriptor.
		 */
		char local_buf[10];
		struct usb_cdc_notification *notif = (void *)local_buf;

		/* We echo signals back to host as notification. */
		notif->bmRequestType = 0xA1;
		notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
		notif->wValue = 0;
		notif->wIndex = 0;
		notif->wLength = 2;
		local_buf[8] = req->wValue & 3;
		local_buf[9] = 0;
		// usbd_ep_write_packet(0x83, buf, 10);
		return 1;
		}
	case USB_CDC_REQ_SET_LINE_CODING:
		if (*len < sizeof(struct usb_cdc_line_coding))
			return 0;
		return 1;
	}
	return 0;
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	(void)usbd_dev;

	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, 0x01, buf, 64);

	if (len) {
		usbd_ep_write_packet(usbd_dev, 0x82, buf, len);
		buf[len] = 0;
	}
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, 0x82, USB_ENDPOINT_ATTR_BULK, 64, NULL);
	usbd_ep_setup(usbd_dev, 0x83, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdcacm_control_request);
}

usbd_device *usbd_dev; // jmf : je le sors en variable globale pour appeler usb_comm
                       // de DDS.c

void usb_comm()
{
 char tu[14] = "hello usb  \r\n\0";
 usbd_poll(usbd_dev);
 usbd_ep_write_packet (usbd_dev, 0x82, tu, 14);
}

void go_usb()
{
//  int i,j=0;
//  char tr[14] = "hello usart\r\n\0";
//  char tu[14] = "hello usb  \r\n\0";

  rcc_peripheral_enable_clock (&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);

  usbd_dev = usbd_init(&stm32f103_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
  usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

// begin libstm32
#ifdef debug
  write_str ("USART initialized\r\n\0");  // display message at RESET
#endif

  while (1)
      usbd_poll(usbd_dev);
/*
  while (1)
    {
     i++;
// end libstm32
      usbd_poll(usbd_dev);
      if (i%0x4000==0 ) usbd_ep_write_packet (usbd_dev, 0x82, tu, 14);
      if (i==0x8000) {GPIO_SetBits (GPIOC, GPIO_Pin_1);	 // opencm3 -> stm32
                      if (j<10) jmf_putchar(j+'0'); else jmf_putchar(j+'A'-10);
                      j++;if (j==16) j=0;
                      jmf_putchar(' ');
                      writeHEXi(0x1234);
                     }
      if (i==0xffff) {
          i=0;
          write_str (tr);
          GPIO_ResetBits (GPIOC, GPIO_Pin_1);// opencm3 -> stm32
      }
    }
*/
}
#endif

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
*******************************************************************************/
void
RCC_Configuration (void)
{
  // ErrorStatus HSEStartUpStatus;
  /* RCC system reset(for debug purpose) */
  RCC_DeInit ();

  /* Enable HSE */
  RCC_HSEConfig (RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp ();
  if (HSEStartUpStatus == SUCCESS)
    {

      FLASH_PrefetchBufferCmd (FLASH_PrefetchBuffer_Enable); // Enable Prefetch Buffer 
      FLASH_SetLatency (FLASH_Latency_2); // Flash 2 wait state 
      RCC_HCLKConfig (RCC_SYSCLK_Div1);   // HCLK = SYSCLK 
      RCC_PCLK2Config (RCC_HCLK_Div1);    // PCLK2 = HCLK 
      RCC_PCLK1Config (RCC_HCLK_Div2);    // PCLK1 = HCLK/2
      RCC_PLLConfig (RCC_PLLSource_HSE_Div1, RCC_PLLMul_9); // PLLCLK = 8MHz * 9 = 72 MHz 
      // jmfriedt : horloge / 1 * 9 = 72
// taken from STM32_USB-FS-Device_Lib_V4.0.0/Projects/Virtual_COM_Port/src
      RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);    // Select USBCLK source
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE); // Enable the USB clock

      RCC_PLLCmd (ENABLE);                // Enable PLL 

      /* Wait till PLL is ready */
      while (RCC_GetFlagStatus (RCC_FLAG_PLLRDY) == RESET)
	{
	}

      /* Select PLL as system clock source */
      RCC_SYSCLKConfig (RCC_SYSCLKSource_PLLCLK);

      /* Wait till PLL is used as system clock source */
      while (RCC_GetSYSCLKSource () != 0x08)
	{
	}
    }
// ADC & USART
      RCC_APB2PeriphClockCmd (RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
			      RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO |
			      RCC_APB2Periph_GPIOC, ENABLE);
      RCC_APB1PeriphClockCmd (RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN | 
                              RCC_APB1Periph_USART2, ENABLE);
}

void
Delay (uint32_t nCount)
{
  for (; nCount != 0; nCount--);
}

#ifdef  DEBUG
void
assert_failed (u8 * file, u32 line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
    {
    }
}

#endif /*  */

void
corrige_offset (__attribute__((unused)) unsigned int *a)
{				/* ne marche pas ... inutile pour le moment */
}

void
delay (int length)
{
  while (length >= 0)
    length--;
}				// 1000=180 us ; 100=20 us ; 200=38 us ; 10=3.3 us

unsigned short
temperature (void)
{
/*
  ADC_RegularChannelConfig (ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5); 
  ADC_SoftwareStartConvCmd (ADC1, ENABLE);
  while (ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC) == RESET);	// EOC is set @ end of cv
  return (ADC_GetConversionValue (ADC1));
*/
  return(999);
}

// voir AN2594 de ST sur eeprom emulation using flash : ATTENTION au risque d'effacer programme !
#define FLASH_PAGE_SIZE    ((uint16_t)0x800)  // chaque page fait 2048 octets
#define BANK1_WRITE_START_ADDR  ((uint32_t)0x0800f800)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08010000)
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
volatile TestStatus MemoryProgramStatus = PASSED;

void eeprom_erase (int a)
{ volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
  uint32_t NbrOfPage;
  uint32_t EraseCounter=0x00;
  FLASH_Unlock();
  NbrOfPage = (BANK1_WRITE_END_ADDR - BANK1_WRITE_START_ADDR) / FLASH_PAGE_SIZE;
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  {
    FLASHStatus = FLASH_ErasePage(BANK1_WRITE_START_ADDR+a+(FLASH_PAGE_SIZE * EraseCounter));
  }
}

void eeprom_write (int a,unsigned char *b, int c)
{ volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
  uint32_t Address = BANK1_WRITE_START_ADDR+a;
  uint32_t *Data;
  unsigned int k=0;
  Data=(uint32_t*)b;
  FLASH_Unlock();
  Address = BANK1_WRITE_START_ADDR;
  while((k<(c/sizeof(uint32_t)))&&(Address<BANK1_WRITE_END_ADDR)&&(FLASHStatus == FLASH_COMPLETE))
    {FLASHStatus = FLASH_ProgramWord(Address, Data[k]); // FLASH_ProgramHalfWord(uint32_t Addr,uint16_t Data);
     Address+=4;
     k++;
    }
  FLASH_Lock();
}

void eeprom_read (int a, unsigned char *b, __attribute__((unused)) int c)
{
  uint32_t *Data;
  uint32_t Address = BANK1_WRITE_START_ADDR+a;
  unsigned int k=0;
  Data=(uint32_t*)b;
  while( (k<(c/sizeof(uint32_t))) &&(Address < BANK1_WRITE_END_ADDR) && (MemoryProgramStatus != FAILED))
  {
   Data[k]=(*(__IO uint32_t*) Address);
   Address += 4;
   k++;
  }
}

void eeprom_init (void)
{
}

void remont_CS_PLL ()
{
}

void envoi_PLL (__attribute__((unused)) unsigned char *entree)
{
}


void clignote() {
//  gpio_toggle(GPIOC, GPIO2|GPIO1);
GPIOC->ODR ^=  ( 1 << 1 ); // 2
// GPIOC->ODR ^=  ( 1 << 2 ); 
//  GPIO_SetBits(GPIOC, GPIO_Pin_2);
//  GPIO_SetBits(GPIOC, GPIO_Pin_1);
}

void
sortie_led (int led)
{
  if (led > 100)
    {
      GPIO_SetBits (GPIOC, GPIO_Pin_2);
      GPIO_ResetBits (GPIOC, GPIO_Pin_1);
    }
  else
    {
      GPIO_SetBits (GPIOC, GPIO_Pin_1);
      GPIO_ResetBits (GPIOC, GPIO_Pin_2);
    }
////    GPIO_ResetBits(GPIOC,GPIO_Pin_1);
////    else if (led > 0)
////         GPIO_ResetBits(GPIOC,GPIO_Pin_2);
}

void sortie_antenne (__attribute__((unused)) int antenne)
{				/* cas multiantennes ... inutile ici */
}

void deinit_serial (int a)
{if ((a&1)!=0) USART_DeInit (USART1); 
 if ((a&2)!=0) USART_DeInit (USART2); 
}

void set_serial_speed (int a,int b) // a=speed, b=parity {0,1,2}={None, Odd, Even}
{
 if ((a==115200) || (a==57600) || (a==19200) || (a==38400) ||
     (a==9600)||(a==4800))
 {
  USART_InitTypeDef UART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStruct;

  USART_StructInit (&UART_InitStructure);
  UART_InitStructure.USART_StopBits = USART_StopBits_1;
  if (b>0)
    {UART_InitStructure.USART_WordLength = USART_WordLength_9b;
     if (b==1) UART_InitStructure.USART_Parity = USART_Parity_Odd;
        else   UART_InitStructure.USART_Parity = USART_Parity_Even;
    }
  else
   {UART_InitStructure.USART_WordLength = USART_WordLength_8b;
    UART_InitStructure.USART_Parity = USART_Parity_No;
   }
  UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  UART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  UART_InitStructure.USART_BaudRate = a;
  USART_DeInit (USART1); USART_Init (USART1, &UART_InitStructure);
  USART_DeInit (USART2); USART_Init (USART2, &UART_InitStructure);

#ifdef MODBUS
  NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;        // USART1_IRQChannel;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init (&NVIC_InitStruct);
  USART_ITConfig (USART2, USART_IT_RXNE, ENABLE);
 // ATTENTION : ISR differents pour libopencm3 et libstm32 -> a avoir dans stm32f10x_it.c
#endif
  NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;        // USART1_IRQChannel;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init (&NVIC_InitStruct);
  USART_ITConfig (USART1, USART_IT_RXNE, ENABLE);
 // ATTENTION : ISR differents pour libopencm3 et libstm32 -> a avoir dans stm32f10x_it.c
  USART_Cmd (USART1, ENABLE);
#ifdef MODBUS
  USART_Cmd (USART2, ENABLE);
#endif
 }
}

void set_DAC0 (int i) // commenter si on commande le dephaseur analogique
{
#ifndef analog_phase
  DAC_SetChannel2Data (DAC_Align_12b_R, (unsigned short) (i >> 16));
  DAC_SoftwareTriggerCmd (DAC_Channel_2, ENABLE);
#endif
}

void set_DAC1 (__attribute__((unused)) int i)
{
}

void set_DAC2 (int i)
{
  DAC_SetChannel1Data (DAC_Align_12b_R, (unsigned short) (i >> 16));
  DAC_SoftwareTriggerCmd (DAC_Channel_1, ENABLE);
}

void
set_SPI_speedDiv (__attribute__((unused)) int i)
{
}

void
sortie_courant_init ()
{
}

void
sortie_courant (__attribute__((unused)) int val)
{
}

void
kick_watchdog ()
{
}

#ifdef periodique
void power_down() {PWR_EnterSTANDBYMode();}  // a necessite' d'activer RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN
#endif

char read_switch() {return(1);}
