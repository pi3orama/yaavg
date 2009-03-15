#include <stdio.h>
#include <video/video.h>
#include <common/debug.h>

#define NUM	(256)
int main()
{
	char buf[NUM];
	struct render_list list;
	struct render_command rc1, rc2;

	DEBUG_INIT(NULL);
	rcmd_init(&rc1, "command 1", FALSE,
			NULL, NULL);
	rcmd_init(&rc2, "command 2", FALSE,
			NULL, NULL);

	rlist_init(&list);

	rlist_link_tail(&list, &rc1);
	rlist_link_tail(&list, &rc2);

	int x;
	x = rlist_snprintf(buf, NUM, &list);
	printf("%s\n%d\n", buf, x);
}

