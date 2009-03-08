#include <econfig/econfig.h>
#include <stdio.h>
int main(int argc, char * argv[])
{
	conf_init(argc, argv);

	printf("%s\n", conf_get_string("video.screenshotdir", NULL));

	return 0;
}

