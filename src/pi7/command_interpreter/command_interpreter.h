#ifndef __command_interpreter_h
#define __command_interpreter_h

#include <stdint.h>

// identification of registers to read
#define REG_X 0
#define REG_Y 1
#define REG_Z 2
#define REG_LINHA 3

// identification of register to write
#define REG_START 0

// error
#define CTL_ERR -1

extern int ctl_ReadRegister(int registerToRead);
extern int ctl_WriteRegister(int registerToWrite, int value);
extern int ctl_WriteProgram(uint8_t* programBytes);
extern void ctl_init();

#endif
