/*
 * Modulo: Estado Trajetoria
 * Contem as variaveis de estado da trajetoria e de controle da maquina em geral
 */

#include "trj_state.h"
#include <stdio.h>

int tst_line;
float tst_x;
float tst_y;
float tst_z;

int tst_getCurrentLine() {
	return tst_line;
} // tst_getCurrentLine

void tst_setCurrentLine(int line) {
	tst_line = line;
} // tst_setCurrentLine

float tst_getX() {
	return tst_x;
} // tst_getX

float tst_getY() {
	return tst_y;
} // tst_getY

float tst_getZ() {
	return tst_z;
} // tst_getZ

void tst_setX(float x) {
	tst_x = x;
} // tst_setX

void tst_setY(float y) {
	tst_y = y;
} // tst_setY

void tst_setZ(float z) {
	tst_z = z;
} // tst_setZ

void tst_init() {
} // tst_init

