#include <resource/bitmap.h>
#include <common/exception.h>
#include <common/debug.h>

#include <common/utils.h>
#include <common/utils_png.h>

#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char * argv[])
{
	DEBUG_INIT(NULL);
	VERBOSE(SYSTEM, "Start!!!\n");

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		struct bitmap * bitmap1, * bitmap2;
		char * name1 = "../common/rgb.png";
		char * name2 = "../common/rgba.png";
		bitmap1 = res_load_bitmap((res_id_t)(uint32_t)name1);
		bitmap2 = res_load_bitmap((res_id_t)(uint32_t)name2);

		bitmap1 = res_load_bitmap((res_id_t)(uint32_t)name1);
		bitmap2 = res_load_bitmap((res_id_t)(uint32_t)name2);
		write_to_pngfile_rgb("/tmp/rgb.png", bitmap1->data, bitmap1->w, bitmap1->h);
		write_to_pngfile_rgba("/tmp/rgba.png", bitmap2->data, bitmap2->w, bitmap2->h);
	}
	CATCH(exp) {
		case (EXCEPTION_NO_ERROR):
			VERBOSE(SYSTEM, "No error!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			VERBOSE(SYSTEM, "User quit: %s\n", exp.message);
			break;
		default:
			ERROR(SYSTEM, "Error out: %s\n", exp.message);
	}

	do_cleanup();
	gc_cleanup();

	show_mem_info();
	return 0;
}

