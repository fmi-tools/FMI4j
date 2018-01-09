/**********************************************************
 * This file is generated by 20-sim ANSI-C Code Generator
 *
 *  file:  src\xxfuncs.h
 *  model: ControlledTemperature_2
 *  expmt: ControlledTemperature_2
 *  date:  June 19, 2017
 *  time:  4:45:33 PM
 *  user:  Controllab Internal
 *  from:  Controllab Products B.V., 20-sim 4.7 Professional Single
 *  build: 4.7.1000.7863
 **********************************************************/

#ifndef XX_FUNCS_H
#define XX_FUNCS_H

/* Our own include files */
#include "xxtypes.h"

/* Global constants */
extern XXBoolean xx_stop_simulation;

/* Wrapper functions around strings */
XXDouble XXString2Double(const char* argument);
const char* XXDouble2String(XXDouble argument);

/* 20-sim function prototypes */
XXDouble XXRamp (XXDouble argument, XXDouble time);
/* 20-sim stub prototypes */
#endif
