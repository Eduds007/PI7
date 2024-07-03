/**
 * Modulo: Controlador de trajetoria (exemplo!!)
 *
 */

/*
 * FreeRTOS includes
 */

#include "FreeRTOS.h"
#include "queue.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h> // para usar o comando malloc

// Header files for PI7
#include "trj_control.h"
#include "../trj_program/trj_program.h"
#include "../trj_state/trj_state.h"
#include "../comm_pic/comm_pic.h"

// local variables
int tcl_status;
extern xQueueHandle qCommPIC1;
extern xQueueHandle qCommPIC2;
extern xQueueHandle qControlCommands;

#define PI 31415
#define L1 170
#define L2 200
#define SCALE 120

//manda para o PIC um setpoint
void tcl_generateSetpoint() {

  // TODO: implementar

  int currLine; 
  tpr_Data line;
  pic_Data toPic1;
  pic_Data toPic2;

  // função só é executada se o status for STATUS_RUNNING
  if (tcl_status != STATUS_RUNNING) {
    return;
  }

  currLine = tst_getCurrentLine();
  printf("CurrLine %d\n", currLine);
  line = tpr_getLine(currLine);
  printf("Setpoint: %d %d\n", line.x, line.y);


  int L3 = round(sqrt(line.x * line.x + line.y * line.y));
  int L3s = SCALE * L3;

  // Calcular cos(beta) escalado
  int L1s = L1 * SCALE;
  int L2s = L2 * SCALE;
  int L1_squared_scaled = L1s * L1s;
  int L2_squared_scaled = L2s * L2s;
  int L3_squared_scaled = L3s * L3s;
  int cos_beta_scaled =(L1_squared_scaled + L2_squared_scaled - L3_squared_scaled) / (2 * L1 * L2); // OK

  int beta_scaled = round((180 * 10000 / PI) * acos((float)cos_beta_scaled / (SCALE * SCALE)));
  // Calcular gamma
  int gamma = 0;
  int thetas = 0;

  if (line.x == 0) {
    gamma = 90;
  } else {
      thetas = SCALE*atan((float)line.y / line.x);
    if (line.x < 0) {
      gamma = 180;
    }
  }
  
  int cos_gamma_scaled = (L1_squared_scaled + L3_squared_scaled - L2_squared_scaled) / (2 * L1 * L3);
  gamma += (180 * 10000 / PI) * ((float)thetas/SCALE - acos((float) cos_gamma_scaled / (SCALE*SCALE)));

  toPic1.SOT = (char)':';
  toPic1.ADD = (char)'a';
  toPic1.COM = (char)'p';
  toPic1.VAL = gamma; // angulo quadril
  toPic1.EOT = (char)';';
  xQueueSend(qCommPIC1, &toPic1, portMAX_DELAY);
  
  toPic2.SOT = (char)':';
  toPic2.ADD = (char)'b';
  toPic2.COM = (char)'p';
  toPic2.VAL = beta_scaled;//; // angulo joelho
  toPic2.EOT = (char)';';
  xQueueSend(qCommPIC2, &toPic2, portMAX_DELAY);

  currLine++;
  tst_setCurrentLine(currLine);
} // trj_generateSetpoint

void tcl_processCommand(tcl_Data data) {

  if ((data.command == CMD_SUSPEND) || (data.command == CMD_STOP)) {
    tcl_status = STATUS_NOT_RUNNING;
  }

  if ((data.command == CMD_START) || (data.command == CMD_RESUME)) {
    //printf("starting trajectory\n");
    tcl_status = STATUS_RUNNING;
  }

  if (data.command == CMD_START) {
    tst_setCurrentLine(0);
  }

} // trj_executeCommand

void tcl_init() {
   tcl_status = STATUS_NOT_RUNNING;
  // tcl_Data start;
  // start.command = CMD_START;
  // xQueueSend(qControlCommands, &start, portMAX_DELAY);
} // init
