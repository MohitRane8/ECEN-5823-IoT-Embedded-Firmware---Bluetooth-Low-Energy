/*
 * gpio.c
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */
#include "gpio.h"
#include "em_gpio.h"
#include <string.h>


/**
 * TODO: define these.  See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
 * and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
 */
#define	LED0_port gpioPortF
#define LED0_pin 4
#define LED1_port gpioPortF
#define LED1_pin 5

void gpioInit()
{
//	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateStrong);
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);
//	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateStrong);
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

	/* PB0 passkey confirmation button configuration */
	GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInputPull, true);

	/* Configuring PB0 for falling edge and enabling its interrupt */
	GPIO_IntConfig(PB0_PORT, PB0_PIN, false, true, true);

	/* Enabling GPIO in NVIC */
//	NVIC_EnableIRQ(GPIO_ODD_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}

//void GPIO_ODD_IRQHandler(void)
//{
//	uint32_t reason = GPIO_IntGet();
//
//	/* Clearing all interrupts */
//	GPIO_IntClear(0x00000000);
//
//	if(reason & 0x40)
//		gecko_external_signal(PB0_FLAG);
//
//	/* Disabling all interrupts */
//	GPIO_IntDisable(0x00000000);
//}

void GPIO_EVEN_IRQHandler(void)
{
	uint32_t reason = GPIO_IntGet();

	/* Clearing all interrupts */
//	GPIO_IntClear(0x00000000);
	GPIO_IntClear(reason);
//	GPIO->IFC = 0x00000000;

	if(reason & 0x40)
		gecko_external_signal(PB0_FLAG);

	/* Disabling all interrupts */
//	GPIO_IntDisable(0x00000000);
//	GPIO_IntDisable(reason);
//	GPIO->IEN = 0x00000000;
}

void gpioLed0SetOn()
{
	GPIO_PinOutSet(LED0_port,LED0_pin);
}
void gpioLed0SetOff()
{
	GPIO_PinOutClear(LED0_port,LED0_pin);
}
void gpioLed1SetOn()
{
	GPIO_PinOutSet(LED1_port,LED1_pin);
}
void gpioLed1SetOff()
{
	GPIO_PinOutClear(LED1_port,LED1_pin);
}

void gpioEnableDisplay()
{
	/* Enabling temperature sensor and display pin */
	GPIO_PinOutSet(LCD_PORT_DISP_SEL, LCD_PIN_DISP_SEL);
}

void gpioSetDisplayExtcomin(bool high)
{
	/* Setting EXTCOMIN pin based on boolean value - to toggle */
	if (high == true)
		GPIO_PinOutSet(LCD_PORT_EXTCOMIN, LCD_PIN_EXTCOMIN);

	else
		GPIO_PinOutClear(LCD_PORT_EXTCOMIN, LCD_PIN_EXTCOMIN);
}
