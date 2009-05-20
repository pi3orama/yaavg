#include <stdio.h>
#include <common/utils.h>
#include <resource/bitmap.h>
int main()
{
	struct bitmap * bitmapA = NULL, * bitmapB = NULL;
	DEBUG_INIT(NULL);

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		bitmapA = read_from_pngfile("./common/rgb.png");
		bitmapB = read_from_pngfile("./common/rgba.png");
		push_cleanup(&bitmapA->base.cleanup);
		push_cleanup(&bitmapB->base.cleanup);
	}
	CATCH(exp) {
		case EXCEPTION_NO_ERROR:
			printf("No error\n");
			break;
		default:
			printf("error: %s\n", exp.message);
	}

	printf("BitmapA:\n");
	printf("\tbytes pre pixel: %d\n", bitmapA->format);
	printf("\tsize: %dx%d\n", bitmapA->w, bitmapA->h);

	printf("BitmapB:\n");
	printf("\tbytes pre pixel: %d\n", bitmapB->format);
	printf("\tsize: %dx%d\n", bitmapB->w, bitmapB->h);

	do_cleanup();
	gc_cleanup();
	show_mem_info();
	return 0;
}

