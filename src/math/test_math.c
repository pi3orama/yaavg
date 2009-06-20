/* 
 *
 */

#include <stdlib.h>
#include <common/debug.h>
#include <math/math.h>
#include <math/matrix.h>

int main()
{
	DEBUG_INIT(NULL);
	cpu_detect();
	if (cpu_cap.have_sse2) {
		TRACE(SYSTEM, "system support sse2\n");
	} else {
		TRACE(SYSTEM, "system doesn't support sse2\n");
	}

	vec4 vec = {
		.v = {1.0, 2.0, 3.0, 4.0},
	};

	mat4x4 compmax1 = {
		.m = {
			{1,2,3,4},
			{5,6,7,8},
			{9,10,11,12},
			{13,14,15,16},
		}
	};


	mat4x4 compmax2 = {
		.m = {
			{1,5,9,13},
			{2,6,10,14},
			{3,7,11,15},
			{4,8,12,16},
		}
	};

	print_matrix(&compmax1);
	print_matrix(&compmax2);
	mulmm(&compmax1, &compmax1, &compmax2);
	print_matrix(&compmax1);

	return 0;
}

