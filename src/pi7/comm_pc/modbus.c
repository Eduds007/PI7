/**
 * Modulo: Comunicacao MODBUS (simplificada)
 * Usa a Serial0 para comunicar-se
 */

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "portmacro.h"
#include "queue.h"
#include "projdefs.h"

// std includes
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

// PICO W include
//#include "hardware/uart.h"
#include "pico/stdio.h"

#define byte uint8_t

// Drivers for UART, LED and Console(debug)
//#include <cr_section_macros.h>
//#include <NXP/crp.h>
//#include "LPC17xx.h"
//#include "type.h"
//#include "drivers/uart/uart.h"
//#include "drivers/console/basic_io.h"
//#include "drivers/ledonboard/leds.h"

// Includes for PI7
#include "modbus.h"
#include "../command_interpreter/command_interpreter.h"

// CommModes: Dev_mode para debug; escreve na console
//            Real_mode para execucao real escreve nas UARTs
//#define DEVELOPMENT_MODE 0
//#define REAL_MODE 1

// *** Configuracao da serial (_mode = REAL_MODE)
//#define BAUD 115200 // 9600
#define MAX_RX_SIZE 1000

// *** endereco deste node
#define MY_ADDRESS 0x01

// Function Codes
#define READ_REGISTER 0x03
#define WRITE_REGISTER 0x06
#define WRITE_FILE 0x15

// Defines de uso geral
#define MB_NO_CHAR 0xff

// Estados do canal de recepcao
#define HUNTING_FOR_START_OF_MESSAGE 0
#define HUNTING_FOR_END_OF_MESSAGE 1
#define IDLE 3
#define MESSAGE_READY 4

int _state;
int _mode;
byte rxBuffer[MAX_RX_SIZE];
int idxRxBuffer;
byte txBuffer[1024];
int idxTxBuffer;
// extern xQueueHandle qCommDev;

/************************************************************************
 initCommunication
 Inicializa modulo de comunicacaoes
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void com_init() {
  _state = HUNTING_FOR_START_OF_MESSAGE;
  // _mode = DEVELOPMENT_MODE; // [jo:231004] testando REAL_MODE
  // //_mode = REAL_MODE; // [jo:231004] testando REAL_MODE
  // if (_mode == REAL_MODE ) {
  //   if (!UARTIsEnabled(0)) UARTInit(0, BAUD); 
  //   if (!UARTIsEnabled(1)) UARTInit(1, BAUD); 
  // }
} // initCommunication

/************************************************************************
 sendTxBufferToSerialUSB
 Envia o conteudo atual do txBuffer pela serial USB
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void sendTxBufferToSerialUSB(void) {
  printf("%s\r", txBuffer); 
}

/************************************************************************
 putCharToSerial
 Escreve o conteudo atual do txBuffer na serial, de acordo com _mode
 _mode = DEV_MODE : printf na console de debug
 _mode = REAL_MODE : escreve da UART0
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void putCharToSerial() {
  // if (_mode == DEVELOPMENT_MODE ) {
  //   // enviar para o console 
  //   //vPrintString(txBuffer); // [jo:230929] TODO: achar substituto
  //   //vPrintString("\r");     // [jo:230929] TODO: achar substituto
  //   printf("%s\r", txBuffer); // [jo:231003] SOLVED: pode enviar direto para o console
  // } else { // _mode == REAL_MODE
  //   // enviar para UART
  //   UARTSendNullTerminated(0, txBuffer);     
  // }
  printf("%s\r", txBuffer);
} // putCharToSerial

/************************************************************************
 getCharFromSerial
 Obtem um caracter da interface serial. A interface a utilizar depende de
 _mode = DEV_MODE : obter caracter da fila qCommDev
 _mode = REAL_MODE : obter da UART0
 Parametros de entrada:
    nenhum
 Retorno:
    (int) caracter obtido ou NO_CHAR se nenhum caracter disponivel
*************************************************************************/
char getCharFromSerial() {
  // char ch = NO_CHAR;
  // BaseType_t status = pdFAIL;
  // if (_mode == DEVELOPMENT_MODE ) {
	//   status = xQueueReceive(qCommDev, &ch, pdMS_TO_TICKS(200)); // portMAX_DELAY); // [jo:231004] original: portMAX_DELAY é infinito, estou fazendo seguir antes (200 ms)
  //   if (status == pdFAIL) ch = NO_CHAR;
  // } else { // REAL_MODE
  //   ch = UARTGetChar(0, false);
  // }
  // return ch;
  return 0;
} // getCharFromSerial

/************************************************************************
 decode, encodeLow, encodeHigh
 Transforma de/para o codigo ASCII de 2 bytes usado no protocolo
 Parametros de entrada:
    decode: high, low
    encodeLow, encodeHigh, value
 Retorno:
    decode: (byte) conversao de 2 bytes ASCII para 1 byte
    encodeLow, encodeHigh: (byte) valor convertido de 1 byte para 1 byte ASCII
*************************************************************************/
byte decode(byte high, byte low) {
  byte x, y;
  if (low < 'A') {
    x = (low & 0x0f);
  } else {
    x = (low & 0x0f) + 0x09;
  }
  if ( high < 'A') {
    y = (high & 0x0f);
  } else {
    y = (high & 0x0f) + 0x09;
  }
  return ( x | ( y << 4) );
} // decode

byte encodeLow(byte value) {
  byte x;
  x = value & 0x0f;
  if ( x < 10) {
    return (0x30 + x);
  } else {
    return (0x41 + (x-10));
  }
} // encodeLow

byte encodeHigh(byte value) {
  byte x;
  x = ((value & 0xf0) >> 4);
  if ( x < 10) {
    return (0x30 + x);
  } else {
    return (0x41 + (x-10));
  }
} // encodeHigh

/************************************************************************
 calculateLRC, checkLRC
 Calcula e verifica o checksum
 Parametros de entrada:
    calculateLRC: (byte[]) bytes, (int) start, (int) end
    checkLRC: nenhum
 Retorno:
    calculateLRC: (byte) LRC calculado
    checkLRC: TRUE se correto, FALSE caso contrario
*************************************************************************/
byte calculateLRC(byte* frame, int start, int end) {
  byte accum;
  byte ff;
  byte um;
  int i;
  accum = 0;
  ff = (byte)0xff;
  um = (byte)0x01;

  for (i= start; i < end; i++) {
    accum += frame[i];
  }
  accum = (byte) (ff - accum);
  accum = accum + um;
  return accum;
} // calculateLRC

int checkLRC() {
  int retval;
  byte receivedLRC;
  byte calculatedLRC;

  // if (_mode == DEVELOPMENT_MODE) {
	//   // do not check LRC in DEV mode
	//   return true; // TRUE;
  // }

  retval = false;
  receivedLRC = decode(rxBuffer[idxRxBuffer-3], rxBuffer[idxRxBuffer-2]);
  calculatedLRC = calculateLRC(rxBuffer, 1, idxRxBuffer - 3);
  printf("LCR rx=%x calc=%x \n", receivedLRC, calculatedLRC);
  if ( receivedLRC == calculatedLRC) {
    retval = true;
  }
  return retval;
} // checkLRC

/************************************************************************
 processReadRegister, processWriteRegister, processWriteFile
 As funcoes realizam o processamento das mensagens
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void processReadRegister() {
  int registerToRead;
  int registerValue;
  byte lrc;

  registerToRead = decode ( rxBuffer[7], rxBuffer[8]);
  // Aciona controller para obter valor. Note que a informacao
  // ate´ poderia ser acessada diretamente. Mas a arquitetura MVC
  // exige que todas as interacoes se deem atraves do controller.
  registerValue = ctl_ReadRegister(registerToRead);

  // Monta frame de resposta e a envia
  txBuffer[0] = ':';
  txBuffer[1] = encodeHigh(MY_ADDRESS);
  txBuffer[2] = encodeLow(MY_ADDRESS);
  txBuffer[3] = encodeHigh(READ_REGISTER);
  txBuffer[4] = encodeLow(READ_REGISTER);
  txBuffer[5] = encodeHigh(1); // byte count field  (high part)
  txBuffer[6] = encodeLow(1);  // byte count field (low part)
  txBuffer[7] = encodeHigh(registerValue);
  txBuffer[8] = encodeLow(registerValue);
  lrc = calculateLRC(txBuffer, 1, 8);
  txBuffer[9] = encodeHigh(lrc);
  txBuffer[10] = encodeLow(lrc);
  txBuffer[11] = 0x0d;
  txBuffer[12] = 0x0a;
  txBuffer[13] = 0; // null to end as string
  //putCharToSerial(); // [jo:231005] original
  sendTxBufferToSerialUSB(); // [jo:231005] atualizado para 2024
} // processReadRegister

void processWriteRegister() {
  int registerToWrite;
  int registerValue;
  byte lrc;

  registerToWrite = decode ( rxBuffer[7], rxBuffer[8]);
  registerValue = decode(rxBuffer[9], rxBuffer[10]);

  // Aciona controller porque a arquitetura MVC
  // exige que todas as interacoes se deem atraves do controller.
  registerValue = ctl_WriteRegister(registerToWrite, registerValue);

  // Monta frame de resposta e a envia
  txBuffer[0] = ':';
  txBuffer[1] = encodeHigh(MY_ADDRESS);
  txBuffer[2] = encodeLow(MY_ADDRESS);
  txBuffer[3] = encodeHigh(WRITE_REGISTER);
  txBuffer[4] = encodeLow(WRITE_REGISTER);
  txBuffer[5] = encodeHigh(1); // byte count field  (high part)
  txBuffer[6] = encodeLow(1);  // byte count field (low part)
  txBuffer[7] = encodeHigh(registerToWrite);
  txBuffer[8] = encodeLow(registerToWrite);
  txBuffer[9] = encodeHigh(registerValue);
  txBuffer[10] = encodeLow(registerValue);
  lrc = calculateLRC(txBuffer, 1, 10);
  txBuffer[11] = encodeHigh(lrc);
  txBuffer[12] = encodeLow(lrc);
  txBuffer[13] = 0x0d;
  txBuffer[14] = 0x0a;
  txBuffer[15] = 0; // null to end as string
  //putCharToSerial(); // [jo:231005] original
  sendTxBufferToSerialUSB(); // [jo:231005] atualizado para 2024
} // processWriteRegister

void processWriteFile() {

	// TODO: implementar

} // processWriteProgram

/************************************************************************
 decodeFunctionCode
 Extrai o function code
 Parametros de entrada:
    nenhum
 Retorno:
    (int) retorna o function code
*************************************************************************/
int decodeFunctionCode() {
   return decode(rxBuffer[3], rxBuffer[4]);
} // extractFunctionCode

/************************************************************************
 processMessage
 Processa uma mensagem ModBus. Inicialmente, verifica o checksum.
 Se estiver correto, aciona a funcao que realiza o processamento
 propriamente dito, de acordo com o function code especificado
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void processMessage() {
  int functionCode;
  if (checkLRC()) {
    functionCode = decodeFunctionCode();
    switch (functionCode) {
    case READ_REGISTER:
      processReadRegister();
      break;
    case WRITE_REGISTER:
      processWriteRegister();
      break;
    case WRITE_FILE:
      processWriteFile();
      break;
    } // switch on FunctionCode
  }
  _state = HUNTING_FOR_START_OF_MESSAGE;
} // processMessage

/************************************************************************
 receiveMessage
 Recebe uma mensagem, byte a byte. Notar que, para o multi-tasking
 cooperativo funcionar, cada funcao deve retornar o mais rapidamente possivel.
 Isso ate nem seria necessario com o FreeRTOS, mas exemplifica a ideia de
 multitasking cooperativo.
 Assim, a recepcao não fica em loop esperando terminar de receber toda a mensagem.
 A mensagem recebida vai sendo armazenada em rxBuffer; idxRxBuffer indica
 em que posicao armazenar o caracter recebido. Ao verificar que a msg foi
 completada (recebendo 0x0D, 0x0A), sinaliza que a msg foi
 recebida fazendo _state = MESSAGE_READY.
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void receiveMessage() {
  char ch = MB_NO_CHAR;

  //ch = getCharFromSerial(); // [jo:231005] original
   if ((ch = getchar_timeout_us(0)) /* != PICO_ERROR_TIMEOUT && ch */ != MB_NO_CHAR) { // [jo:231005] modbus só pela serial USB
  // if (ch != NO_CHAR) { // [jo:231005] original

    printf(" [%x] ", ch); // [jo:231004] teste

    if (_state == HUNTING_FOR_START_OF_MESSAGE) {
      if (ch == ':') {
        idxRxBuffer = 0;
        rxBuffer[idxRxBuffer] = ch;
        _state = HUNTING_FOR_END_OF_MESSAGE;
        putchar_raw(ch); // [jo:231006] teste
        return;
      }
    } else if (_state == HUNTING_FOR_END_OF_MESSAGE) {
      idxRxBuffer++;
      if (idxRxBuffer > MAX_RX_SIZE) {
        _state = HUNTING_FOR_START_OF_MESSAGE;
        idxRxBuffer = 0;
        return;
      }
      rxBuffer[idxRxBuffer] = ch;
      putchar_raw(ch); // [jo:231006] teste
      if ((rxBuffer[idxRxBuffer] == 0x0A) &&
        (rxBuffer[idxRxBuffer - 1] == 0x0D)) {
        _state = MESSAGE_READY;
      }
    }
  } // if not NO_CHAR
}  // receiveMessage

/************************************************************************
 executeCommunication
 Recebeu uma requisicao ModBus e a processa
 Parametros de entrada:
    nenhum
 Retorno:
    nenhum
*************************************************************************/
void com_executeCommunication() {
  receiveMessage();
  if ( _state == MESSAGE_READY ) {
    processMessage();
  }
} // executeCommunication
