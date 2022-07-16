#include"ad8801.h"

#include <stm32/gpio.h>
#include <stm32/usart.h>
#include <stm32/rcc.h>
#include <cmsis/stm32.h>
#include <stm32/flash.h>
#include <stm32/misc.h>
#include <stm32/adc.h>
#include <cmsis/stm32.h>
#include <stdlib.h>
#include <string.h>


#define AD8801_CLR    GPIO_ResetBits(GPIOB,GPIO_Pin_12) // CS du AD8801 sur PB12
#define AD8801_SET    GPIO_SetBits  (GPIOB,GPIO_Pin_12)

// commande, argument(s), nombre d'octets ecrits, nombre d'octets lus
void ad8801_set(unsigned char chan, unsigned char val)
{
	AD8801_CLR; //  gpio_clear(GPIOB,GPIO12);
	delay(5);//5);

	spi_put2(chan);
	spi_put2(val);

	delay(100); //100);
	AD8801_SET; //  gpio_set(GPIOB, GPIO12);
}

/*
void ad8801_init() {
	uint16_t tmpreg;
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_SPI2EN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	gpio_set_mode(GPIOB, GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_FLOAT, GPIO_SPI2_MISO);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_MOSI);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_SPI2_SCK);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	gpio_set(GPIOB, GPIO12);

	tmpreg = SPI_CR1(SPI2) & ((uint16_t)0x3040);
	tmpreg |= (uint16_t)((uint32_t) 0x0000 // bidirfullduplex
									| 0x104 // master 
									| SPI_CR1_DFF_8BIT
									| SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE
									| SPI_CR1_CPHA_CLK_TRANSITION_1
									| SPI_CR1_SSM
									| SPI_CR1_BAUDRATE_FPCLK_DIV_4
									| SPI_CR1_MSBFIRST);
	// Write to SPIx CR1 
	SPI_CR1(SPI2) = tmpreg;
	SPI_CR1(SPI2) &= ~SPI_CR1_CPOL; 
	SPI_CR1(SPI2) &= ~SPI_CR1_CPHA;

	// Activate the SPI mode (Reset I2SMOD bit in I2SCFGR register) 
	SPI_I2SCFGR(SPI2) &= ((uint16_t)0xF7FF);//SPI_Mode_Select;
	// Write to SPIx CRCPOLY 
	SPI_CRCPR(SPI2) = 7;
	spi_enable(SPI2);
	SPI_CR1(SPI2) |= ((uint16_t)0x0040);//CR1_SPE_Set

}
*/

