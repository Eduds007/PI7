/**
 * Name        : main.c
 * Version     :
 * Description : main definition for FreeRTOS application
 */

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Raspberry PICO W includes
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
//#include <type.h>
#include "boards/pico_w.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"

// Drivers for UART and LED
#include "drivers/ledonboard/leds.h"
#include "drivers/uart/uart.h"

// Header files for PI7
#include "pi7/comm_pic/comm_pic.h"
#include "pi7/comm_pc/modbus.h"
#include "pi7/command_interpreter/command_interpreter.h"
#include "pi7/trj_control/trj_control.h"
#include "pi7/trj_program/trj_program.h"
#include "pi7/trj_state/trj_state.h"

// PI7 DEFINES
#define CONTROL_Q_SIZE 1 // queue sizes
#define PIC_Q_SIZE 2
#define DEV_Q_SIZE 20
#define UART_BAUD 115200

/**
 * Time constants for FreeRTOS delays
 */
const portTickType DELAY_1SEC = 1000 / portTICK_RATE_MS;
const portTickType DELAY_500MS = 500 / portTICK_RATE_MS;
const portTickType DELAY_200MS = 200 / portTICK_RATE_MS;

//void __error__(char *pcFilename, unsigned long ulLine) {
//}

/**
 * Communication queues for data transfer between components
 */
QueueHandle_t qControlCommands;
QueueHandle_t qCommPIC;
QueueHandle_t qCommDev; // para testes

#define USERTASK_STACK_SIZE configMINIMAL_STACK_SIZE

void taskController(void *pvParameters) {
  while(1) {
    
    printf("1"); // [jo:231005] teste

    com_executeCommunication(); //internally, it calls Controller to process events
    vTaskDelay(DELAY_200MS); // [jo:230929] TODO: por que não tem vTaskDelay() ? -> não, tem espera na fila
  } //task loop
} // taskController

/**
 * taskNCProcessing: processes NC Program. It receives commands from Controller
 * via queue qControlCommands (start/pause/resume/abort)
 * Runs every 200ms (may generate up to 5 new setpoints per second to interpolate trajectory)
 * Note the use of vTaskDelayUntil instead of vTaskDelay; this will cause system to run every 200ms.
 */
//portTickType lastWakeTime;
void taskNCProcessing(void *pvParameters) {
  static portTickType lastWakeTime;
  tcl_Data data;
  lastWakeTime = xTaskGetTickCount();
  while(1) {

    printf("2"); // [jo:231005] teste

    data.command = NO_CMD;
    xQueueReceive(qControlCommands, &data, 0); //do not wait for command
    if (data.command != NO_CMD) {
      tcl_processCommand(data);
    }
    tcl_generateSetpoint();
    vTaskDelayUntil(&lastWakeTime, DELAY_200MS);
  } //task loop
} // taskNCProcessing

/**
 * taskCommPIC: receive setpoints to send to PICs from queue qCommPIC
 * and send them following PIC protocol
 */
void taskCommPIC(void *pvParameters) {
	pic_Data setpoints;
	while(1) {
            
    //uart_putc_raw(uart0, '3'); // [jo:231004] teste
    UARTSend(0, (uint8_t*)"3", 1); // [jo:231004] teste
    UARTSend(1, (uint8_t*)"3", 1); // [jo:231004] teste
    printf("3"); // [jo:231005] teste

    xQueueReceive(qCommPIC, &setpoints, pdMS_TO_TICKS(250)); // portMAX_DELAY); // [jo:231004] 250 ms no meu teste
    pic_sendToPIC(0, setpoints);
    pic_sendToPIC(1, setpoints);
    //vTaskDelay(DELAY_200MS); // [jo:230928] eu coloquei, precisa?
  } //task loop
} // taskCommPIC

void taskBlinkLed(void *lpParameters) {
  char ch = NO_CHAR;  // [jo:231005] teste console DEV_MODE
	while(1) {
		led_invert();
		vTaskDelay(DELAY_1SEC);  
	} // task loop
} //taskBlinkLed

static void setupHardware(void) {
	// Put hardware configuration and initialisation in here
  /* SystemClockUpdate() updates the SystemFrequency variable */
	// Warning: If you do not initialize the hardware clock, the timings will be inaccurate
	//SystemClockUpdate(); // [jo:230927] TODO: achar equivalente no PICO W 
  //                        [jo:231005] UPDATE: acho que não precisa no PICO W

	// init onboard led
	led_init();

  // inicializa stdin, stdout e stderr no USB c/ default baud rate de 115200
  stdio_usb_init();

  // inicializa UARTs para comunicação com os PICs
  UARTInit(0, UART_BAUD); // UART0
  UARTInit(1, UART_BAUD); // UART1

	printf("Hardware setup completed.\n");
} // setupHardware

static void initComponents(void) {

  // communication between tasks
  qControlCommands = xQueueCreate(CONTROL_Q_SIZE, sizeof(tpr_Data));
  qCommPIC = xQueueCreate(PIC_Q_SIZE, sizeof(pic_Data));
  qCommDev = xQueueCreate(DEV_Q_SIZE, sizeof(char));

  // init components
  com_init(); // communication PC
  pic_init(); // communication PIC
  ctl_init(); // commant interpreter
  tcl_init(); // trajectory control
  tst_init(); // trajectory state
  tpr_init(); // trajectory program

} // initComponents

/**
 * Program entry point 
 */

// test data for ReadRegister: read reg 1 (CoordY)
uint8_t msgReadRegister[] = {0x3a, 0x00, 0x00, 0x30, 0x33, 0x00, 0x00, 0x30, 0x31, 0x33, 0x3c, 0x0d, 0x0a};
// test data for WriteRegister: write reg 0 (START NC PROGRAM)
uint8_t msgWriteRegister[] = {0x3a, 0x30, 0x31, 0x30, 0x36, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 0x0d, 0x0a};

int main(void) {
	int i;
	char ch;

  // Define task handles
  TaskHandle_t handleLed; // [jo:230929] no pico w o cyw43 precisa rodar sempre num único core

	//MB+ init Console(debug)
	printf("nao apague esta linha\n");

	// init hardware
	setupHardware();

	// init components
	initComponents(); // init Modbus

	/* 
	 * Start the tasks defined within this file/specific to this demo. 
	 */
	xTaskCreate( taskBlinkLed, "BlinkLed", USERTASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, &handleLed);
	xTaskCreate( taskController, "Controller", USERTASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( taskNCProcessing, "NCProcessing", USERTASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );
	xTaskCreate( taskCommPIC, "CommPIC", USERTASK_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL );

  vTaskCoreAffinitySet(handleLed, (1 << 0)); // executa BlinkLed num único core

	//*************** DEBUG FOR ReadRegister
	// insert ReadRegister msg on qCommDev for debug
	// for (i=0; i<sizeof(msgReadRegister); i++) {
	//   ch = msgReadRegister[i];
  // 	xQueueSend(qCommDev, &ch, portMAX_DELAY);
	// }
	
  // ************** DEBUG FOR WriteRegister
	// insert WriteRegister msg on qCommDev for debug
	// for (i=0; i<sizeof(msgWriteRegister); i++) {
	//   ch = msgWriteRegister[i];
  // 	xQueueSend(qCommDev, &ch, portMAX_DELAY);
	// }
	
	/* 
	 * Start the scheduler. 
	 */
	vTaskStartScheduler();

	/* 
	 * Will only get here if there was insufficient memory to create the idle task. 
	 */
	return 1;
} // main
