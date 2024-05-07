/**
 * Modulo: Comunicacao UART
 * 
 */

#ifndef __UART_H 
#define __UART_H

#include <stdint.h>
#include <stdbool.h>

// #define IER_RBR		0x01
// #define IER_THRE	0x02
// #define IER_RLS		0x04

// #define IIR_PEND	0x01
// #define IIR_RLS		0x03
// #define IIR_RDA		0x02
// #define IIR_CTI		0x06
// #define IIR_THRE	0x01

// #define LSR_RDR		0x01
// #define LSR_OE		0x02
// #define LSR_PE		0x04
// #define LSR_FE		0x08
// #define LSR_BI		0x10
// #define LSR_THRE	0x20
// #define LSR_TEMT	0x40
// #define LSR_RXFE	0x80

#define BUFSIZE		0x40
#define NO_CHAR   0xff

uint8_t UARTInit( uint8_t portNum, uint32_t Baudrate );
void UART0_IRQHandler( void );
void UART1_IRQHandler( void );
void UARTSend( uint8_t portNum, uint8_t *BufferPtr, uint32_t Length );
void UARTSendNullTerminated( uint8_t portNum, uint8_t *BufferPtr);
uint8_t UARTGetChar( uint8_t portNum, uint8_t blocking );
bool UARTIsEnabled(uint8_t portNum);

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
