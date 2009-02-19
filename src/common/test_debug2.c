#include <stdio.h>
#include <stdlib.h>
#include <common/debug.h>
#include <unistd.h>
#include <signal.h>

int func2(int * p)
{
	int * p2 = malloc(200);
	memset(p2, 1, 200);
	memcpy(p, p2, 10);
	/* raise a SIGUSR1 */
	raise(SIGUSR1);
	free(p2);

#if 1
	/* generate a segfault */
	*(int*)(NULL) = 10;
#endif

	while(1) {
		printf("xx\n");
		sleep(1000);
	}
	return 0;
}

int func()
{
	/* alloc memory */
	int * p = malloc(200);
	memset(p, 0, 200);
	p = realloc(p, 500);
	raise(SIGUSR1);
	func2(p);
	return 0;
}

int main()
{
	DEBUG_INIT(NULL);
	FATAL(OPENGL, "Test fatal\n");
	func();
	return 0;
}

