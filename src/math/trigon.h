/* 
 * trigon.h
 * by WN @ Jun. 20, 2009
 */

#ifndef __TRIGON_H
#define __TRIGON_H
#include <common/defs.h>

#define TRIGON_TABLE_SIZE	(65536*4)
extern float sin_table[TRIGON_TABLE_SIZE];
extern float tan_table[TRIGON_TABLE_SIZE];

/* d means degree */
extern float
sin_d(float X);

extern float
cos_d(float X);

extern float
tan_d(float X);

#endif
