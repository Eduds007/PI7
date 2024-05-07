
/**
 * Modulo: Comunicacao UART
 * 
 */

//#include "lpc17xx.h"
//#include "type.h"
#include "uart.h"

//MB+ interface with FreeRTOS
#include "FreeRTOS.h"
#include "pico/stdio.h"
#include "projdefs.h"
#include "queue.h"

// PICO SDK
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/cyw43_arch.h"
#include <stdbool.h>
#include <stdio.h>

// extern uint32_t SystemFrequency;    /*!< System Clock Frequency (Core Clock)  */

// volatile uint32_t UART0Status, UART1Status;
// volatile uint8_t UART0TxEmpty = 1, UART1TxEmpty = 1;
// volatile uint8_t UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE];
// volatile uint32_t UART0Count = 0, UART1Count = 0;

volatile xQueueHandle UART0Queue;
volatile xQueueHandle UART1Queue;

// #define NO_CHAR -1

/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART port, setup pin select,
**						clock, parity, stop bits, FIFO, etc.
**
** parameters:			portNum(0 or 1) and UART baudrate
** Returned value:		true or false, return false only if the 
**						interrupt handler can't be installed to the 
**						VIC table
** 
*****************************************************************************/
uint8_t UARTInit(uint8_t PortNum, uint32_t baudrate) {
  // uint32_t Fdiv;
  // uint32_t pclkdiv, pclk;

  if ( PortNum == 0 ) { // UART0

    // código original UART0
    // LPC_PINCON->PINSEL0 &= ~0x000000F0;
    // LPC_PINCON->PINSEL0 |= 0x00000050;  /* RxD0 is P0.3 and TxD0 is P0.2 */
    //
    // /* By default, the PCLKSELx value is zero, thus, the PCLK for
    // all the peripherals is 1/4 of the SystemFrequency. */
    // /* Bit 6~7 is for UART0 */
    // pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
    // switch ( pclkdiv )
    // {
    //   case 0x00:
    //   default:
    // 	pclk = SystemFrequency/4;
    // 	break;
    //   case 0x01:
    // 	pclk = SystemFrequency;
    // 	break; 
    //   case 0x02:
    // 	pclk = SystemFrequency/2;
    // 	break; 
    //   case 0x03:
    // 	pclk = SystemFrequency/8;
    // 	break;
    // }
    //
    //   LPC_UART0->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
    // Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
    //   LPC_UART0->DLM = Fdiv / 256;							
    //   LPC_UART0->DLL = Fdiv % 256;
    // LPC_UART0->LCR = 0x03;		/* DLAB = 0 */
    //   LPC_UART0->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
    //
    //  	NVIC_EnableIRQ(UART0_IRQn);
    //
    //   LPC_UART0->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART0 interrupt */

    uart_init(uart0, baudrate); // [jo:230927] Initialise UART 0
    // Set the GPIO pin mux to the UART - 0 is TX, 1 is RX
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    irq_set_exclusive_handler(UART0_IRQ, UART0_IRQHandler);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);

    /*
    * Create a queue to receive characters from Serial with space for 50 characters
    */
    // UART0Queue = xQueueCreate(50,1); // [jo:231005] original
    UART0Queue = xQueueCreate(BUFSIZE, sizeof(char)); // [jo:231005]
    if (UART0Queue == NULL) {
      fprintf(stderr, "UARTinit Error: UART0 queue not created.");
      return false;
    }

    return true;

  } else if ( PortNum == 1 ) { // UART1

    // código original UART1
    // LPC_PINCON->PINSEL4 &= ~0x0000000F;
    // LPC_PINCON->PINSEL4 |= 0x0000000A;	/* Enable RxD1 P2.1, TxD1 P2.0 */
    //
    // /* By default, the PCLKSELx value is zero, thus, the PCLK for
    // all the peripherals is 1/4 of the SystemFrequency. */
    // /* Bit 8,9 are for UART1 */
    // pclkdiv = (LPC_SC->PCLKSEL0 >> 8) & 0x03;
    // switch ( pclkdiv )
    // {
    //   case 0x00:
    //   default:
    // 	pclk = SystemFrequency/4;
    // 	break;
    //   case 0x01:
    // 	pclk = SystemFrequency;
    // 	break; 
    //   case 0x02:
    // 	pclk = SystemFrequency/2;
    // 	break; 
    //   case 0x03:
    // 	pclk = SystemFrequency/8;
    // 	break;
    // }
    //
    //   LPC_UART1->LCR = 0x83;		/* 8 bits, no Parity, 1 Stop bit */
    // Fdiv = ( pclk / 16 ) / baudrate ;	/*baud rate */
    //   LPC_UART1->DLM = Fdiv / 256;							
    //   LPC_UART1->DLL = Fdiv % 256;
    // LPC_UART1->LCR = 0x03;		/* DLAB = 0 */
    //   LPC_UART1->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */
    //
    //  	NVIC_EnableIRQ(UART1_IRQn);
    //
    //   LPC_UART1->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART1 interrupt */

    uart_init(uart1, baudrate); // [jo:230927] Initialise UART 1
    // Set the GPIO pin mux to the UART - 4 is TX, 5 is RX
    gpio_set_function(4, GPIO_FUNC_UART);
    gpio_set_function(5, GPIO_FUNC_UART);

    irq_set_exclusive_handler(UART1_IRQ, UART1_IRQHandler);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(uart1, true, false);

    /*
    * Create a queue to receive characters from Serial with space for 50 characters
    */
    // UART1Queue = xQueueCreate(50,1); // [jo:231005] original
    UART1Queue = xQueueCreate(BUFSIZE, sizeof(char)); // [jo:231005]
    if (UART1Queue == NULL) {
      fprintf(stderr, "UARTinit Error: UART1 queue not created.");
      return false;
    }
    return true;
  } // UART1 - end

  return false; // se portNum não for 0 nem 1
}

/*****************************************************************************
** Function name:		UARTIsEnabled
**
** Descriptions:		testa se a UART está habilitada
**
** parameters:		  portNum (0 or 1) 
** Returned value:	true or false, se habilitada ou não
** 
*****************************************************************************/
bool UARTIsEnabled(uint8_t portNum) {
 if (portNum == 0) return uart_is_enabled	(uart0);	// UART0
 else uart_is_enabled	(uart1); // UART1
}

/*****************************************************************************
** Function name:		UART0_IRQHandler
**
** Descriptions:		UART0 interrupt handler
**
** parameters:			None
** Returned value:	None
** 
*****************************************************************************/
void UART0_IRQHandler (void) {
  // uint8_t IIRValue, LSRValue;
  // uint8_t Dummy = Dummy;
  uint8_t rdChar;
  // static uint32_t UART0Count = 0;

  // código original UART0
  // IIRValue = LPC_UART0->IIR;
  //
  // IIRValue >>= 1;			/* skip pending bit in IIR */
  // IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  // if ( IIRValue == IIR_RLS )	{	/* Receive Line Status */
  //
  //   LSRValue = LPC_UART0->LSR;
  //   /* Receive Line Status */
  //   if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) ) {
  //
  //     /* There are errors or break interrupt */
  //     /* Read LSR will clear the interrupt */
  //     UART0Status = LSRValue;
  //     Dummy = LPC_UART0->RBR;		/* Dummy read on RX to clear 
  //               interrupt, then bail out */
  //     return;
  //   }
  //   if ( LSRValue & LSR_RDR )	{ /* Receive Data Ready */				
  //     /* If no error on RLS, normal ready, save into the data buffer. */
  //     /* Note: read RBR will clear the interrupt */
  //     rdChar = LPC_UART0->RBR;
  //     UART0Buffer[UART0Count] = rdChar;
  //     if (UART0Queue) {
  //       xQueueSendFromISR(UART0Queue, &rdChar, pdFALSE);
  //     }
  //     UART0Count++;
  //     if ( UART0Count == BUFSIZE ) {
  //       UART0Count = 0;		/* buffer overflow */
  //     }	
  //   }
  // } else if ( IIRValue == IIR_RDA )	{ /* Receive Data Available */
	// /* Receive Data Available */
  //   rdChar = LPC_UART0->RBR;
  //   UART0Buffer[UART0Count] = rdChar;
  //   if (UART0Queue) {
	//    xQueueSendFromISR(UART0Queue, &rdChar, pdFALSE);
  //   }
	//   UART0Count++;
  //   if ( UART0Count == BUFSIZE ) {
  //     UART0Count = 0;		/* buffer overflow */
  //   }
  // }
  // else if ( IIRValue == IIR_CTI )	{ /* Character timeout indicator */
  //
	// /* Character Time-out indicator */
	// UART0Status |= 0x100;		/* Bit 9 as the CTI error */
  // } else if ( IIRValue == IIR_THRE )	{ /* THRE, transmit holding register empty */
  //
	// /* THRE interrupt */
  //   LSRValue = LPC_UART0->LSR;		/* Check status in the LSR to see if
	// 								valid data in U0THR or not */
  //   if ( LSRValue & LSR_THRE ) {
  //     UART0TxEmpty = 1;
  //   } else {
  //     UART0TxEmpty = 0;
  //   }
  // }   
  
  if (uart_is_readable(uart0)) {
    rdChar = uart_getc(uart0);
    //if (UART0Queue) {
	   xQueueSendFromISR(UART0Queue, &rdChar, NULL);
    //}
    // UART0Buffer[UART0Count++] = rdChar;
    // if (UART0Count == BUFSIZE) UART0Count = 0;		/* buffer circular */
  }
}

/*****************************************************************************
** Function name:		UART1_IRQHandler
**
** Descriptions:		UART1 interrupt handler
**
** parameters:			None
** Returned value:	None
** 
*****************************************************************************/
void UART1_IRQHandler(void) {
  // uint8_t IIRValue, LSRValue;
  // uint8_t Dummy = Dummy;
  uint8_t rdChar;
  // static uint32_t UART1Count = 0;

  // código original UART1
  // IIRValue = LPC_UART1->IIR;
  //
  // IIRValue >>= 1;          /* skip pending bit in IIR */
  // IIRValue &= 0x07;        /* check bit 1~3, interrupt identification */
  // if (IIRValue == IIR_RLS) { /* Receive Line Status */
  //   LSRValue = LPC_UART1->LSR;
  //   /* Receive Line Status */
  //   if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI)) {
  //     /* There are errors or break interrupt */
  //     /* Read LSR will clear the interrupt */
  //     UART1Status = LSRValue;
  //     Dummy = LPC_UART1->RBR; /* Dummy read on RX to clear
  //                                             interrupt, then bail out */
  //     return;
  //   }
  //   if (LSRValue & LSR_RDR) { /* Receive Data Ready */
  //     /* If no error on RLS, normal ready, save into the data buffer. */
  //     /* Note: read RBR will clear the interrupt */
  //     rdChar = LPC_UART1->RBR;
  //     UART1Buffer[UART1Count] = rdChar;
  //     if (UART1Queue) {
  //           xQueueSendFromISR(UART1Queue, &rdChar, pdFALSE);
  //     }
  //     UART1Count++;
  //     if (UART1Count == BUFSIZE) {
  //           UART1Count = 0; /* buffer overflow */
  //     }
  //   }
  // } else if (IIRValue == IIR_RDA) { /* Receive Data Available */
  //   /* Receive Data Available */
  //   rdChar = LPC_UART1->RBR;
  //   UART1Buffer[UART1Count] = rdChar;
  //   if (UART1Queue) {
  //     xQueueSendFromISR(UART1Queue, &rdChar, pdFALSE);
  //   }
  //   UART1Count++;
  //   if (UART1Count == BUFSIZE) {
  //     UART1Count = 0; /* buffer overflow */
  //   }
  // } else if (IIRValue == IIR_CTI) { /* Character timeout indicator */
  //
  //   /* Character Time-out indicator */
  //   UART1Status |= 0x100;      /* Bit 9 as the CTI error */
  // } else if (IIRValue == IIR_THRE) { /* THRE, transmit holding register empty */
  //
  //   /* THRE interrupt */
  //   LSRValue = LPC_UART1->LSR; /* Check status in the LSR to see if
  //                                               valid data in U0THR or not */
  //   if (LSRValue & LSR_THRE) {
  //     UART1TxEmpty = 1;
  //   } else {
  //     UART1TxEmpty = 0;
  //   }
  // }

  if (uart_is_readable(uart1)) {
    rdChar = uart_getc(uart1);
    //if (UART1Queue) {
      xQueueSendFromISR(UART1Queue, &rdChar, NULL);
    //}
    // UART1Buffer[UART1Count++] = rdChar;
    // if (UART1Count == BUFSIZE) UART1Count = 0;		/* buffer circular */
  }
}

/*****************************************************************************
** Function name:		UARTGetChar
**
** Descriptions:		Get a char from portNum serial port
**
** parameters:			portNum : 0 or 1
**                      blocking: TRUE if should be blocked until a char has
**                      been received
** Returned value:		character received from serial port or -1 (uint32_t)
**
*****************************************************************************/
uint8_t UARTGetChar( uint8_t portNum, uint8_t blocking ) {
  xQueueHandle queue;
	uint8_t ch;
  portBASE_TYPE status;

  queue = (portNum == 0 ? UART0Queue : UART1Queue);

  if (blocking) {
		status = xQueueReceive(queue, &ch, portMAX_DELAY); // [jo:231004] original: portMAX_DELAY é infinito
	} else {
		status = xQueueReceive(queue, &ch, 0);
	}

	if (status == pdPASS) return ch;
  return NO_CHAR; // [jo:231005] em caso de não ter nenhum caracter na queue
} // UARTGetChar

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**						on the data length
**
** parameters:			portNum, buffer pointer, and data length
** Returned value:		None
** 
*****************************************************************************/
void UARTSend( uint8_t portNum, uint8_t *BufferPtr, uint32_t Length ) {

  // uart_putc_raw((portNum==0 ? uart0 : uart1), 'a'); // [jo:231004] meu teste

  if ( portNum == 0 ) { // UART0
    while ( Length-- != 0 ) {
      // /* THRE status, contain valid data */
      // while (!(UART0TxEmpty & 0x01));	
      // LPC_UART0->THR = *BufferPtr;
      uart_putc_raw(uart0, *(BufferPtr++));
      //UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */ // [jo:231004] tem que ser fora do while
      //BufferPtr++;
      //Length--;
    }
    // UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */ // [jo:231004] movido para cá
  } else {
    while ( Length-- != 0 ) {
      // /* THRE status, contain valid data */
      // while (!(UART1TxEmpty & 0x01));	
      // LPC_UART1->THR = *BufferPtr;
      uart_putc_raw(uart1, *(BufferPtr++));
      //UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */ // [jo:231004] tem que ser fora do while
      //BufferPtr++;
      //Length--;
    }
    // UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */ // [jo:231004] movido para cá
  }
  return;
}

/*****************************************************************************
** Function name:		UARTSendNullTerminated
**
** Descriptions:		Send a block of data to the UART 0 port;
**                      block of data MUST be NULL TERMINATED
**
** parameters:			portNum, buffer pointer
** Returned value:		None
**
*****************************************************************************/
void UARTSendNullTerminated( uint8_t portNum, uint8_t *BufferPtr) {

  // uart_putc_raw((portNum==0 ? uart0 : uart1), 'b'); // [jo:231004] meu teste

  if ( portNum == 0 ) { // UART0
    // while (!(UART0TxEmpty & 0x01)); // [jo:231004] não entendi, vou deixar assim por enquanto
    uart_puts(uart0, (char*)BufferPtr); // [jo:231004] funciona assim
    // UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */
    // while ( *BufferPtr != 0 ) {
    //   /* THRE status, contain valid data */
    //   while (!(UART0TxEmpty & 0x01)); // [jo:231004] não entendi
    //   // LPC_UART0->THR = *BufferPtr;
    //   uart_putc_raw(uart0, *BufferPtr);
    //   UART0TxEmpty = 0;	/* not empty in the THR until it shifts out */
    //   BufferPtr++;
    // }
  } else { // UART1
    //while (!(UART1TxEmpty & 0x01)); // [jo:231004] não entendi, vou deixar assim por enquanto
    uart_puts(uart1, (char*)BufferPtr); // [jo:231004] funciona assim
    //UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */
    // while ( *BufferPtr != 0 ) {
    //   /* THRE status, contain valid data */
    //   while ( !(UART1TxEmpty & 0x01) ); // [jo:231004] não entendi
    //   //LPC_UART1->THR = *BufferPtr;
    //   uart_putc_raw(uart1, *BufferPtr);
    //   UART1TxEmpty = 0;	/* not empty in the THR until it shifts out */
    //   BufferPtr++;
    //   }
  }
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
