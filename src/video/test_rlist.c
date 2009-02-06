/* by WN @ Jan 29, 2009 */

#include <stdio.h>
#include <video/video.h>

int main()
{
	char buf[256];
	struct RenderList list;
	struct RenderCommand rc1, rc2;

	RCommandInit(&rc1, "Test Name", NULL, NULL, NULL, NULL);
	RCommandInit(&rc2, NULL, NULL, NULL, NULL, NULL);


	RListInit(&list, NULL);
	RListLinkTail(&list, &rc1);
	RListLinkTail(&list, &rc2);

	rlist_sprint(buf, &list);
	printf("%s\n", buf);
	return 0;
}

