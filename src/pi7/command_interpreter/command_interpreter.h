#ifndef __command_interpreter_h
#define __command_interpreter_h

#include <stdint.h>

// identification of register to write
#define REG_START   0
#define REG_PAUSE   1
#define REG_RESUME  2
#define REG_STOP    3
#define REG_JOGX    4
#define REG_JOGY    5
#define STEP_X      6
#define STEP_Y      7
#define REG_X       8
#define REG_Y       9
#define REG_LINE    10
#define REG_PROG    11

// error
#define CTL_ERR -1

extern int ctl_ReadRegister(int registerToRead);
extern int ctl_WriteRegister(int registerToWrite, int value);
extern int ctl_WriteProgram(char* programBytes);
extern void ctl_init();

#endif
