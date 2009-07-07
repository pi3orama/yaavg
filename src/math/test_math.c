/* 
 *
 */

#include <stdlib.h>
#include <common/debug.h>
#include <common/math.h>
#include <math/math.h>
#include <math/matrix.h>
#include <math/trigon.h>

int main()
{
	DEBUG_INIT(NULL);
	math_init();
	if (cpu_cap.have_sse2) {
		TRACE(SYSTEM, "system support sse2\n");
	} else {
		TRACE(SYSTEM, "system doesn't support sse2\n");
	}

	/* test matrix invert */
	mat4x4 d;
	mat4x4 s = {
		.f = {
			0,0,1,0,
			1,0,1,0,
			1,1,1,0,
			0,0,0,1,
		}
	};


	invert_matrix(&d, &s);

	print_matrix(&s);

	print_matrix(&d);

	return 0;
}

