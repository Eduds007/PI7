/*
 * Modulo: Programa Trajetoria
 * Armazena o programa da trajetoria a ser executada
 */

// max NC program size
#define MAX_PROGRAM_LINES 200

#include "trj_program.h"
#include "../command_interpreter/command_interpreter.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// structure to store NC program
tpr_Data tpr_program[MAX_PROGRAM_LINES];

int tpr_storeProgram(char* texto) {

	int i;
	char* separa = strtok(texto, "-");

	while (separa != NULL) {
		if (i%2==0) {
		tpr_program[i/2].x = (int)separa[0];
		tpr_program[i++/2].y = (int)separa[1];	
		}
		separa = strtok(NULL, "-");
	}
	return 1;
} // tpr_storeProgram

tpr_Data tpr_getLine(int line) {
	return tpr_program[line];
} // tpr_getLine

void tpr_init() {
  for (int i=0; i<MAX_PROGRAM_LINES;i++) {
	  tpr_program[i].x = 0;
	  tpr_program[i].y = 0;
  }
} //tpr_init
