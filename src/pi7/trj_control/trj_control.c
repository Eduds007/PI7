/**
 * Modulo: Controlador de trajetoria (exemplo!!)
 *
 */

/*
 * FreeRTOS includes
 */
#include "FreeRTOS.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

// Header files for PI7
#include "trj_control.h"
#include "../trj_program/trj_program.h"
#include "../trj_state/trj_state.h"
#include "../comm_pic/comm_pic.h"

// local variables
int tcl_status;
extern xQueueHandle qCommPIC;

float *inverse_kin(float x, float y){
  float theta= (float)malloc(2 * sizeof(float));
  if (x*x+y*y >= 0 && x*x+y*y <=400){
    float theta2 = acos((x*x+y*y-L1*L1-L2*L2)/(2*L1*L2));
    theta[1]=theta2;
    float tanY = L2*sin(theta2)/(L1+L2*cos(theta2));
    theta[0] = atan((y-x*tanY)/(x+y*tanY));
  }
  return theta;
}

void tcl_generateSetpoint() {

  // TODO: implementar

  int currLine;
  tpr_Data line;
  pic_Data toPic;

  if (tcl_status != STATUS_RUNNING) {
    return;
  }

  currLine = tst_getCurrentLine();
  printf("CurrLine %d\n", currLine);
  line = tpr_getLine(currLine);
  toPic.setPoint1 = line.x;
  toPic.setPoint2 = line.y;
  toPic.setPoint3 = line.z;
  xQueueSend(qCommPIC, &toPic, portMAX_DELAY);
  currLine++;
  tst_setCurrentLine(currLine);
} // trj_generateSetpoint

void tcl_processCommand(tcl_Data data) {

  if ((data.command == CMD_SUSPEND) || (data.command == CMD_STOP)) {
    tcl_status = STATUS_NOT_RUNNING;
  }

  if ((data.command == CMD_START) || (data.command == CMD_RESUME)) {
    printf("starting trajectory\n");
    tcl_status = STATUS_RUNNING;
  }

  if (data.command == CMD_START) {
    tst_setCurrentLine(0);
  }

} // trj_executeCommand

void tcl_init() {
  tcl_status = STATUS_NOT_RUNNING;
} // init
