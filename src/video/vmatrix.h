/* 
 * vmatrix.h
 */
#include <math/matrix.h>

enum mat_type {
	MATRIX_MODELVIEW	= 0,
	MATRIX_PROJECTION	= 1,
};

extern mat4x4 * current_modelview;
extern mat4x4 * current_projection;

extern void
mat_load_identity(enum mat_type t);

extern void
mat_ortho(void);

extern void
mat_translate(float x, float y, float z);

