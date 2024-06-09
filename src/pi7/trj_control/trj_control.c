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

#define L1 17
#define L2 20
#define M_PI 3.14159265358979323846


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
  printf("Setpoint: %f %f %f\n", line.x, line.y, line.z);


  float L3= sqrt(pow(line.x,2)+pow(line.y,2));
  int y = 33- line.y;
  float beta = (180/M_PI)*acos((pow(L1,2)+pow(L2,2)-pow(L3,2))/(2*L1*L2));
  float gamma=0;
  float theta=0;
  if (line.x==0){
    gamma=90;
  } else{
    float theta=atan(y/line.x);
    if(line.x<0){
      gamma=180;
    }
  }
  gamma+= (180/M_PI) * (theta - acos((pow(L1,2)+pow(L3,2)-pow(L2,2))/(2*L1*L3)));

  toPic1.SOT = (char)':';
  toPic1.ADD = (char)'1';
  toPic1.COM = (char)'p';
  toPic1.VAL = gamma; // angulo quadril
  toPic1.EOT = (char)';';
  xQueueSend(qCommPIC1, &toPic1, portMAX_DELAY);
  
  toPic2.SOT = (char)':';
  toPic2.ADD = (char)'2';
  toPic2.COM = (char)'p';
  toPic2.VAL = beta; // angulo joelho
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
    printf("starting trajectory\n");
    tcl_status = STATUS_RUNNING;
  }

  if (data.command == CMD_START) {
    tst_setCurrentLine(0);
  }

} // trj_executeCommand

void tcl_init() {
  // tcl_status = STATUS_NOT_RUNNING;
  tcl_Data start;
  start.command = CMD_START;
  xQueueSend(qControlCommands, &start, portMAX_DELAY);
} // init