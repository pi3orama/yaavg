
#include <stdio.h>
#include "debug.h"

int func2()
{
	int i = 1, j = 2;
	DEBUG_MSG(FATAL, SYSTEM, "Debug test system fatal: i=%d, j=%d\n", i, j);

	if (i < j)
		BUG_ON(0);
	printf("Haha\n");
}

int func()
{
	DEBUG_MSG(FATAL, SYSTEM, "Debug test system fatal\n");
	func2();
}

int main()
{
	DEBUG_INIT("/tmp/debug_out");

	DEBUG_MSG(TRACE, VCOMMAND, "Debug test trace\n");
	DEBUG_MSG(VERBOSE, VCOMMAND, "Debug test verbose\n");
	DEBUG_MSG(WARNING, VCOMMAND, "Debug test warning\n");
	DEBUG_MSG(ERROR, VCOMMAND, "Debug test error\n");
	DEBUG_MSG(FATAL, VCOMMAND, "Debug test fatal\n");

	DEBUG_MSG(TRACE, VLIST, "Debug test trace\n");
	DEBUG_MSG(VERBOSE, VCOMMAND, "Debug test verbose\n");
	DEBUG_MSG(WARNING, VCOMMAND, "Debug test warning\n");
	DEBUG_MSG(ERROR, VCOMMAND, "Debug test error\n");
	DEBUG_MSG(FATAL, VCOMMAND, "Debug test fatal\n");


	DEBUG_SET_COMP_LEVEL(VLIST, ERROR);
	DEBUG_MSG(TRACE, VLIST, "Debug test trace\n");
	DEBUG_MSG(FATAL, VLIST, "Debug test trace\n");
	func();
	return 0;
}

