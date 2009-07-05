#include <stdio.h>
#include <common/utils.h>
#include <common/utils_png.h>
#include <resource/bitmap.h>
int main()
{
	struct bitmap * bitmapA = NULL, * bitmapB = NULL;
	DEBUG_INIT(NULL);

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		bitmapA = read_bitmap_from_pngfile("./common/rgb.png");
		bitmapB = read_bitmap_from_pngfile("./common/rgba.png");
		push_cleanup(&bitmapA->base.cleanup);
		push_cleanup(&bitmapB->base.cleanup);

		write_to_pngfile_rgb("/tmp/out.png", bitmapA->data,
				bitmapA->w, bitmapA->h);
		write_to_pngfile_rgba("/tmp/out2.png", bitmapB->data,
				bitmapB->w, bitmapB->h);
		write_to_pngfile_rgba("/tmp/out3.png", bitmapB->data,
				bitmapB->w, bitmapB->h);
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

