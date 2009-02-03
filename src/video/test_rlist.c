/* by WN @ Jan 29, 2009 */

#include <stdio.h>
#include <video/video.h>

int main()
{
	char buf[256];
	struct RenderList list;
	struct RenderCommand rc1, rc2;

	RCommandInit(&rc1);
	RCommandInit(&rc2);

	rc2.name = "Test RC";

	RListInit(&list, NULL);
	RListLinkTail(&list, &rc1);
	RListLinkTail(&list, &rc2);

	sprint_rlist(buf, &list);
	printf("%s\n", buf);
	return 0;
}

