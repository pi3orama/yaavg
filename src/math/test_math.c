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

#define check(n)	\
	FORCE(SYSTEM, "sin_d("#n")=%f, sin("#n")=%f\n", sin_d(n), sinf(n / 180.0f * M_PI)); \
	FORCE(SYSTEM, "cos_d("#n")=%f, cos("#n")=%f\n", cos_d(n), cosf(n / 180.0f * M_PI)); \
	FORCE(SYSTEM, "tan_d("#n")=%f, tan("#n")=%f\n", tan_d(n), tanf(n / 180.0f * M_PI)); \

	check(26);
	check(-26);

	check(186);
	check(-186);

	check(3678.43);
	check(-3678.43);

	return 0;
}

