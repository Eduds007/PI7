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

// float *inverse_kin(float x, float y){
//   float theta= (float)malloc(2 * sizeof(float));
//   if (x*x+y*y >= 0 && x*x+y*y <=400){
//     float theta2 = acos((x*x+y*y-L1*L1-L2*L2)/(2*L1*L2));
//     theta[1]=theta2;
//     float tanY = L2*sin(theta2)/(L1+L2*cos(theta2));
//     theta[0] = atan((y-x*tanY)/(x+y*tanY));
//   }
//   return theta;
// }

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


  //if (line.x*line.x+line.y*line.y >= 0 && line.x*line.x+line.y*line.y <=1369){ //37^2
  //  theta[1] = acos((line.x*line.x+line.y*line.y-L1*L1-L2*L2)/(2*L1*L2));
  //  float tanY = L2*sin(theta[1])/(L1+L2*cos(theta[1]));
  //  theta[0] = atan((line.y-line.x*tanY)/(line.x+line.y*tanY));
  //}


  // Cinemática inversa 
  float L3= sqrt(pow(line.x,2)+pow(line.y,2));
  float theta = atan(line.x/line.y);
  float phi = acos((pow(L3,2)-pow(L1,2)-pow(L2,2))/(2*L1*L3));
  float alpha = atan((L2*sin(phi))/(L1+L2*cos(phi)));
  float qhip = theta + alpha;
  float qknee = phi + qhip;
  float mhip = M_PI - qhip;

  printf("Theta1: %f, Theta2:%f\n", mhip, phi);
  
  toPic1.SOT = (char)':';
  toPic1.ADD = (char)'a';
  toPic1.COM = (char)':';
  toPic1.VAL = mhip / 0.0174532925; //conversão para radianos
  toPic1.EOT = (char)';';
  xQueueSend(qCommPIC1, &toPic1, portMAX_DELAY);
  
  toPic2.SOT = (char)':';
  toPic2.ADD = (char)'a';
  toPic2.COM = (char)':';
  toPic2.VAL = phi / 0.0174532925; //conversão para radianos
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