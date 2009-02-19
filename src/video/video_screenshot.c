/* by WN @ Feb 19, 2009 */

#include <config.h>
#include <common/defs.h>
#include <common/debug.h>
#include <econfig/econfig.h>
#include <video/engine.h>
#include <video/engine_driver.h>

#include <png.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
static void
write_to_pngfile(char * filename, int w, int h, uint8_t * buffer)
{
	png_structp write_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;
	png_text texts[1];

	texts[0].key = "Software";
	texts[0].text = "yaavg";
	texts[0].compression = PNG_TEXT_COMPRESSION_NONE;

	const char * prefix = ConfGetString("video.screenshotdir", "/tmp");

	/* Test the path */
	struct stat statb;
	if ((stat(prefix, &statb)) || (!S_ISDIR(statb.st_mode))) {
		WARNING(VIDEO, "screenshotdir %s is not a valid direction\n",
				prefix);
		return;
	}

	char * fullname = alloca(strlen(prefix) + strlen(filename) + 1);
	sprintf(fullname, "%s/%s", prefix, filename);

	/* Open the file */
	FILE * pngfile = fopen(fullname, "wb");
	if (pngfile == NULL) {
		WARNING(VIDEO, "open file failed, error = %s\n", strerror(errno));
		return;
	}

	/* write data */
	write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
			png_error, png_warning);
	info_ptr = png_create_info_struct(write_ptr);
	png_init_io(write_ptr, pngfile);

	png_set_IHDR(write_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_set_text(write_ptr, info_ptr, texts, 1);
	png_write_info(write_ptr, info_ptr);

#define SIZE_OF_ROW (4*w)
	int n;
	for (n = h - 1; n >= 0; n--) {
		uint8_t * prow = buffer + n * SIZE_OF_ROW;
		png_write_rows(write_ptr, &prow, 1);
	}

	png_write_end(write_ptr, info_ptr);


	/* close file */
	fclose(pngfile);

	return;
}

extern struct VideoContext * VideoCtx;

void
VideoScreenShot(void)
{
#ifndef ENABLE_SCREENSHOT
	WARNING(VIDEO, "Screen shot is disabled in compiling.\n");
	return;
#else
	uint8_t * buffer = NULL;
	int x, y, w, h;
	int err;

	if (VideoCtx == NULL) {
		WARNING(VIDEO, "Video system has not beed inited\n");
		return;
	}

	x = VideoCtx->vp_x;
	y = VideoCtx->vp_y;
	w = VideoCtx->vp_w;
	h = VideoCtx->vp_h;

	buffer = malloc(sizeof(uint8_t) * w * h * 4);
	assert(buffer != NULL);
	err = DriverReadPixels(buffer, x, y, w, h);
	if (err) {
		WARNING(VIDEO, "Read pixels failed\n");
		free(buffer);
		return;
	}

	/* Generate filename */
	struct tm tm, *ptm;
	time_t stime;
	stime = time(NULL);
	if (stime == (time_t)(-1)) {
		WARNING(VIDEO, "Get current time failed\n");
		stime = 0;
	}

	ptm = localtime_r(&stime, &tm);
	if (ptm != &tm) {
		ERROR(VIDEO, "Cover to struct tm failed\n");
		free(buffer);
		return;
		
	}
	
	char * filename;
	filename = alloca(64);
	memset(filename, '\0', 64);
	strftime(filename, 64, "YAAVG-%Y%m%d%H%M%s.png", ptm);

	VERBOSE(VIDEO, "Taking screen shot to file %s\n", filename);
	
	write_to_pngfile(filename, w, h, buffer);

	free(buffer);
#endif
}

