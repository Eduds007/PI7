#ifndef __trj_state_h
#define __trj_state_h

// external interface
extern int tst_getCurrentLine();
extern void tst_setCurrentLine(int line);
extern float tst_getX();
extern float tst_getY();
extern float tst_getZ();
extern void tst_setX(float x);
extern void tst_setY(float y);
extern void tst_setZ(float z);
extern void tst_init();
#endif
