/* 
 * test_geom.c
 * by WN @ May. 16, 2009
 */

#include <stdio.h>
#include <common/geometry.h>

static void
check(struct rectangle r1, struct rectangle r2)
{
	struct rectangle r3 = geom_rect_cover(r1, r2);
	struct rectangle r4 = geom_rect_cover(r2, r1);
	
	printf(RECT_FMT " cover " RECT_FMT " = " RECT_FMT " and " RECT_FMT "\n",
			RECT_ARG(&r1), RECT_ARG(&r2), RECT_ARG(&r3),
			RECT_ARG(&r4));
}

int
main()
{
	struct rectangle r01 = {
		0, 0, 101, 102
	};

	struct rectangle r02 = {
		53, 54, 15, 16
	};


	check(r01, r02);

	struct rectangle r11 = {
		0, 0, 101, 102
	};

	struct rectangle r12 = {
		53, 54, 105, 106
	};

	check(r11, r12);

	struct rectangle r21 = {
		0, 0, 101, 102
	};

	struct rectangle r22 = {
		-53, 54, 105, 106
	};
	check(r21, r22);

	struct rectangle r31 = {
		0, 0, 101, 102
	};

	struct rectangle r32 = {
		-53, -54, 105, 106
	};
	check(r31, r32);


	struct rectangle r41 = {
		0, 0, 101, 102
	};

	struct rectangle r42 = {
		53, -54, 105, 106
	};
	check(r41, r42);

	struct rectangle r51 = {
		1,2,3,4
	};

	struct rectangle r52 = {
		1,2,3,4
	};

	if (RECT_SAME(&r51, &r52))
		printf("r51 == r52\n");
	else
		printf("error\n");
	return 0;
}

