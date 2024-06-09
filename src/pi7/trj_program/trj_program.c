/*
 * Modulo: Programa Trajetoria
 * Armazena o programa da trajetoria a ser executada
 */

// max NC program size
#define MAX_PROGRAM_LINES 50

#include "trj_program.h"
#include "../command_interpreter/command_interpreter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// structure to store NC program
tpr_Data tpr_program[MAX_PROGRAM_LINES];

void tpr_storeProgram(int pico_registers[], int tam) {

	tpr_Data data; 
	int line = 0;
	for (int i = 0; i<2*MAX_PROGRAM_LINES; i= i+2) {
		if (i>=tam){
			break;
		}
		data.x = pico_registers[i+REG_PROG];
		data.y = pico_registers[i+1 + REG_PROG];
		tpr_program[line] = data;
		line ++;
	}
  
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
