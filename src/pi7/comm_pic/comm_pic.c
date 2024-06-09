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

   uint8_t out[32];

  sprintf((char*)out, "%c%c%c%f%c\n", ':', '0', 'p', 0, ';');
  UARTSendNullTerminated(0, out);  // envia para UART 0 
  sprintf((char*)out, "%c%c%c%f%c\n", ':', '1', 'p', 0, ';');
  UARTSendNullTerminated(1, out);  // envia para UART 1
   // TODO: implementar
  
} // pic_init

void pic_sendToPIC(uint8_t portNum, pic_Data data) {
  uint8_t out[32];
  sprintf((char*)out, "%c%c%c%f%c\n", data.SOT, data.ADD, data.COM, data.VAL, data.EOT);
  UARTSendNullTerminated(portNum, out);  // envia também para UART 0 ou 1
  //UARTSend(portNum, out, 23); // [jo:231004] alternativa linha acima sem NULL no final
} // pic_sendToPIC

extern uint8_t pic_receiveCharFromPIC(uint8_t portNum) {
  return UARTGetChar(portNum, false);
} // pic_receiveFromPIC
