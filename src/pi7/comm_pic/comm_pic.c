/**
 * Modulo: Comunicacao MODBUS (simplificada)
 * Usa a Serial0 para comunicar-se
 * [jo:230927] usa UART0 e UART1 para comunicação
 */

/*
 * FreeRTOS includes
 */
//#include "FreeRTOS.h"
//#include "queue.h"
#include <stdbool.h>
#include <stdio.h>

// Drivers for UART, LED and Console(debug)
//#include <cr_section_macros.h>
//#include <NXP/crp.h>
//#include "LPC17xx.h"
//#include "type.h"
#include "drivers/uart/uart.h"
//#include "hardware/uart.h"

// Header files for PI7
#include "comm_pic.h"

void pic_init(void){

  // TODO: implementar
  
} // pic_init

void pic_sendToPIC(uint8_t portNum, pic_Data data) {
  uint8_t out[32];

  // Implementação de teste, envia setpoint para console e para UARTs
	//printf("X=%5.1f Y=%5.1f Z=%5.1f\n", data.setPoint1, data.setPoint2, data.setPoint3);
  sprintf((char*)out, "X=%5.1f Y=%5.1f Z=%5.1f\n", data.setPoint1, data.setPoint2, data.setPoint3);
  //puts((char*)out); // envia para console
  UARTSendNullTerminated(portNum, out);  // envia também para UART 0 ou 1
  //UARTSend(portNum, out, 23); // [jo:231004] alternativa linha acima sem NULL no final

  // TODO: implementar

} // pic_sendToPIC

extern uint8_t pic_receiveCharFromPIC(uint8_t portNum) {
  return UARTGetChar(portNum, false);
} // pic_receiveFromPIC
