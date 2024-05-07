#ifndef __trj_control_h
#define __trj_control_h

/**
 * Commands for TrajectoryController
 */

#define NO_CMD      0
#define CMD_START   1
#define CMD_SUSPEND 2
#define CMD_RESUME  3
#define CMD_STOP    4

// Possible status for TrajectoryController
#define STATUS_RUNNING   0
#define STATUS_NOT_RUNNING 2

// struct for communication between TrajectoryController and Controller
typedef struct {
	int command;
} tcl_Data;

// external interface
extern void tcl_processCommand(tcl_Data data);
extern void tcl_generateSetpoint();
extern void tcl_init();
#endif
