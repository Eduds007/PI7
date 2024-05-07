#ifndef __trj_program_h
#define __trj_program_h


typedef struct {
	float x;
	float y;
	float z;
} tpr_Data;

extern void tpr_storeProgram(char* texto);
extern tpr_Data tpr_getLine(int line);
extern void tpr_init();
#endif
