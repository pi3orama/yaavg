
#include <stdio.h>
#include "debug.h"

int func2()
{
	int i = 1, j = 2;
	DEBUG_MSG(FATAL, SYSTEM, "Debug test system fatal: i=%d, j=%d\n", i, j);

	if (i < j)
		INTERNAL_ERROR(SYSTEM, "Try internal error %d\n", 100);
//		internal_error(SYSTEM, "Try internal error %d\n", 100);
//		assert(0);
	printf("Haha\n");
	return 0;
}

int func()
{
	DEBUG_MSG(FATAL, SYSTEM, "Debug test system fatal\n");
	func2();
	return 0;
}

int main()
{
	DEBUG_INIT(NULL);

	DEBUG_MSG(TRACE, RCOMMAND, "Debug test trace\n");
	DEBUG_MSG(VERBOSE, RCOMMAND, "Debug test verbose\n");
	DEBUG_MSG(WARNING, RCOMMAND, "Debug test warning\n");
	DEBUG_MSG(ERROR, RCOMMAND, "Debug test error\n");
	DEBUG_MSG(FATAL, RCOMMAND, "Debug test fatal\n");

	DEBUG_MSG(TRACE, RLIST, "Debug test trace\n");
	DEBUG_MSG(VERBOSE, RCOMMAND, "Debug test verbose\n");
	DEBUG_MSG(WARNING, RCOMMAND, "Debug test warning\n");
	DEBUG_MSG(ERROR, RCOMMAND, "Debug test error\n");
	DEBUG_MSG(FATAL, RCOMMAND, "Debug test fatal\n");


	DEBUG_SET_COMP_LEVEL(RLIST, ERROR);
	DEBUG_MSG(TRACE, RLIST, "Debug test trace\n");
	DEBUG_MSG(FATAL, RLIST, "Debug test trace\n");
	func();
	return 0;
}

