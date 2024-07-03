/*
 * Modulo: Estado Trajetoria
 * Contem as variaveis de estado da trajetoria e de controle da maquina em geral
 */

#include "trj_state.h"
#include <stdio.h>

int tst_line;
int tst_x;
int tst_y;

int tst_getCurrentLine() {
	return tst_line;
} // tst_getCurrentLine

void tst_setCurrentLine(int line) {
	tst_line = line;
} // tst_setCurrentLine

int tst_getX() {
	return tst_x;
} // tst_getX

int tst_getY() {
	return tst_y;
} // tst_getY

void tst_setX(int x) {
	tst_x = x;
} // tst_setX

void tst_setY(int y) {
	tst_y = y;
} // tst_setY

void tst_init() {
} // tst_init

