/* 
 * trigon.h
 * by WN @ Jun 20, 2009
 */

#include <common/defs.h>
#include <common/math.h>
#include <math/trigon.h>

float sin_table[TRIGON_TABLE_SIZE];
float tan_table[TRIGON_TABLE_SIZE];

#define STEP	(360.0f / (float)TRIGON_TABLE_SIZE)

static inline float
lookup_d(float X, float * table)
{
	float sig = (X > 0) ? 1.0 : -1.0;
	X = X * sig;
	X = X - floorf(X / 360.0f) * 360.0f;
	float retval = table[(int)(X / STEP)];
	return retval * sig;
}

float
sin_d(float X)
{
	return lookup_d(X, sin_table);
}


float
cos_d(float X)
{
	return lookup_d(X + 90.0f, sin_table);
}

float
tan_d(float X)
{
	return lookup_d(X, tan_table);
}


