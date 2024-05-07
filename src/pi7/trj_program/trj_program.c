/*
 * Modulo: Programa Trajetoria
 * Armazena o programa da trajetoria a ser executada
 */

// max NC program size
#define MAX_PROGRAM_LINES 50

#include "trj_program.h"

// structure to store NC program
tpr_Data tpr_program[MAX_PROGRAM_LINES];

void tpr_storeProgram(char* texto) {

	// TODO: implementar
  
} // tpr_storeProgram

tpr_Data tpr_getLine(int line) {
	return tpr_program[line];
} // tpr_getLine

void tpr_init() {
  int i;

  for (i=0; i<MAX_PROGRAM_LINES;i++) {
	  tpr_program[i].x = 0;
	  tpr_program[i].y = 0;
	  tpr_program[i].z = 0;
  }
} //tpr_init
