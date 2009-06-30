#include <common/math.h>
#include <math/math.h>


#ifndef __SQRT_H
#define __SQRT_H


__BEGIN_DECLS

#ifdef __X86_32__
/* See http://www.gamedev.net/community/forums/topic.asp?topic_id=139956,
 * from QUAKE3's code */
static inline float
invert_sqrt(float x)
{
	union fltint {
		float f;
		int32_t i;
	} * ux, * ux2;
	ux = (void*)(&x);
	float xhalf = 0.5f * x;
	int i = ux->i;
	i = 0x5f3759df - (i >> 1);
	ux2 = (void*)&i;
#define F (ux2->f)	
	x = F * (1.5f - xhalf * F * F);
#undef F
	return x;
}

#define sqrt(x)	(1.0f / invert_sqrt(x))

#else

static inline float
invert_sqrt(float x)
{
	return 1.0f / sqrtf(x);
}

static inline float
sqrt(float x)
{
	return sqrtf(x);
}


#endif /* __X86_32__ */

__END_DECLS

#endif

// vim:ts=4:sw=4
