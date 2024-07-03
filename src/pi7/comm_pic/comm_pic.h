#ifndef __COMM_PIC_H
#define __COMM_PIC_H

/** struct for communication between TrajectoryControl
 *  and communication to PIC
 */

//#include "hardware/uart.h"
#include <stdint.h>

typedef struct {
	char SOT;
	char ADD;
	char COM;
	int VAL;
	char EOT; 
} pic_Data;

void pic_init(void);
void pic_set(void);
void pic_sendToPIC(uint8_t portNum, pic_Data data);
uint8_t pic_receiveCharFromPIC(uint8_t portNum);

#endif
