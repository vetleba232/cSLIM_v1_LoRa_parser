/**
* \file  main.c
*
* \brief LORAWAN Parser Application
*		
*
* Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries. 
*
* \asf_license_start
*
* \page License
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products. 
* It is your responsibility to comply with third party license terms applicable 
* to your use of third party software (including open source software) that 
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, 
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, 
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE 
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL 
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE 
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE 
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY 
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
* \asf_license_stop
*
*/
 
/****************************** INCLUDES **************************************/
#include "system_assert.h"
#include "sw_timer.h"
#include "system_low_power.h"
#include "radio_interface.h"
#include "radio_driver_hal.h"
#include "lorawan.h"
#include "sys.h"
#include "sio2host.h"
#include "parser.h"
#include "parser_tsp.h"
#include "parser_system.h"
#include "system_init.h"
#include "aes_engine.h"
#ifdef CONF_PMM_ENABLE
 #include "sleep_timer.h"
#endif /* CONF_PMM_ENABLE */
#if (ENABLE_PDS == 1)	
  #include "pds_interface.h"
#endif
#include "sal.h"
#include "edbg_eui.h"


/************************** macro definition ***********************************/

/************************** Global variables ***********************************/

/****************************** PROTOTYPES *************************************/
SYSTEM_TaskStatus_t APP_TaskHandler(void);
#if (_DEBUG_ == 1)
static void assertHandler(SystemAssertLevel_t level, uint16_t code);
#endif

/****************************** FUNCTIONS *************************************/
static void print_reset_causes(void)
{
	
	enum system_reset_cause rcause = system_get_reset_cause();
	printf("\r\nLast reset cause: ");
	if(rcause & (1 << 6)) {
		printf("System Reset Request\r\n");
	}
	if(rcause & (1 << 5)) {
		printf("Watchdog Reset\r\n");
	}
	if(rcause & (1 << 4)) {
		printf("External Reset\r\n");
	}
	if(rcause & (1 << 2)) {
		printf("Brown Out 33 Detector Reset\r\n");
	}
	if(rcause & (1 << 1)) {
		printf("Brown Out 12 Detector Reset\r\n");
	}
	if(rcause & (1 << 0)) {
		printf("Power-On Reset\r\n");
	}
}

#if (_DEBUG_ == 1)
static void assertHandler(SystemAssertLevel_t level, uint16_t code)
{
	printf("\r\n%04x\r\n", code);
	(void)level;
}
#endif /* #if (_DEBUG_ == 1) */

/**
 * \mainpage
 * \section preface Preface
 * This is the reference manual for the LORAWAN Parser Application of EU Band
 */

int main(void)
{
	system_init();
	delay_init();
	board_init();
	INTERRUPT_GlobalInterruptEnable();
	sio2host_init();
	print_reset_causes();
	
#if (_DEBUG_ == 1)
	SYSTEM_AssertSubscribe(assertHandler);
#endif

	/* Configure board button as external interrupt pin */
	configure_extint();	
	/* Register External Interrupt callback */
	configure_eic_callback();
	printf("LoRaWAN Stack UP\r\n");
	HAL_RadioInit();
	// Initialize AES only (crypto is on-demand)
	SAL_Init(false) ;
	// Initialize Timers
	SystemTimerInit();
#ifdef CONF_PMM_ENABLE
	SleepTimerInit();
#endif /* CONF_PMM_ENABLE */

#if (ENABLE_PDS == 1)	
 	PDS_Init();
#endif	
	Stack_Init();
	Parser_Init();
    Parser_SetConfiguredJoinParameters(0x01);
    Parser_GetSwVersion(aParserData);
    Parser_TxAddReply((char *)aParserData, (uint16_t)strlen((char *)aParserData));
	
	//struct port_config pin_conf;
	/*
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	*/
	
	/*
	//Set i2c pins as input and pullup
	port_pin_set_config(PIN_PA16, &pin_conf);
	port_pin_set_config(PIN_PA17, &pin_conf);
	*/
	/*
	//Set SPI MISO as input (disable)
	pin_conf.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(PIN_PB02, &pin_conf);
	*/
	
	/*
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	
	//This pin is uset for uart logging to nRF52840 from nRF9160. If USB is set up this can be received here and sent over USB
	port_pin_set_config(PIN_PA06, &pin_conf);
	*/
	
	/*
	//Set address pins as input (pins are currently not used)
	port_pin_set_config(AD1_PIN, &pin_conf);
	port_pin_set_config(AD2_PIN, &pin_conf);
	port_pin_set_config(AD3_PIN, &pin_conf);
	port_pin_set_config(AD4_PIN, &pin_conf);
	port_pin_set_config(AD5_PIN, &pin_conf);
	port_pin_set_config(AD6_PIN, &pin_conf);
	port_pin_set_config(AD7_PIN, &pin_conf);
	port_pin_set_config(AD8_PIN, &pin_conf);
	*/
	
	/*
	// Configure LED as outputs, turn them off
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(PIN_PB03, &pin_conf);
	port_pin_set_output_level(PIN_PB03, LED_0_ACTIVE);
	port_pin_set_output_level(LED_0_PIN, LED_0_INACTIVE);
	//port_pin_set_output_level(LED_1_PIN, LED_1_INACTIVE);
	*/
	
    while (1)
    {
		parser_serial_data_handler();
		SYSTEM_RunTasks();
    }
}

SYSTEM_TaskStatus_t APP_TaskHandler(void)
{
	Parser_Main();
	return SYSTEM_TASK_SUCCESS;
}

/**
 End of File
 */
