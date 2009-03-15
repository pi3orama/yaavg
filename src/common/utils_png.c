/* 
 * utils_png.c
 * by WN @ Mar. 15, 2009
 *
 * png related utils
 */

#include <png.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <common/debug.h>
#include <common/exception.h>


struct png_write_cleanup {
	struct cleanup base;
	png_structp write_ptr;
	/* if we don't want to free writer_ptr, but need to free
	 * info_ptr, we must save the writer_ptr*/
	png_structp write_ptr_save;
	png_infop info_ptr;
};

static void
do_png_write_cleanup(struct cleanup * str)
{
	struct png_write_cleanup * pcleanup;
	pcleanup = (struct png_write_cleanup *)(str->args);

	if (pcleanup->write_ptr != NULL)
		png_destroy_write_struct(&pcleanup->write_ptr,
				&pcleanup->info_ptr);
	/* NOTICE: png_destroy_write_struct can set info_ptr and
	 * write_ptr to NULL. if we don't have a write_ptr but
	 * have info_ptr, we need free info_ptr. */
	if (pcleanup->info_ptr != NULL) {
		assert(pcleanup->write_ptr_save != NULL);
		png_destroy_info_struct(pcleanup->write_ptr_save,
				&pcleanup->info_ptr);
	}

	free(pcleanup);
}

static struct png_write_cleanup *
alloc_png_write_cleanup(void)
{
	struct png_write_cleanup * pcleanup;
	pcleanup = calloc(1, sizeof(*pcleanup));
	assert(pcleanup != NULL);

	pcleanup->base.function = do_png_write_cleanup;
	pcleanup->base.args = pcleanup;

	pcleanup->write_ptr = NULL;
	pcleanup->info_ptr = NULL;
	return pcleanup;
}

struct png_writer {
	png_rw_ptr write_fn;
	png_flush_ptr flush_fn;
	png_voidp io_ptr;
	png_structp write_ptr;
	png_infop info_ptr;
};

static void
png_write(struct png_writer writer, uint8_t * buffer, int w, int h, int type)
{
	struct png_write_cleanup * pcleanup;

	pcleanup = alloc_png_write_cleanup();
	assert(pcleanup != NULL);
	make_cleanup(&pcleanup->base);

	png_structp write_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;
	png_text texts[1];


	/* if png handler is passed from writer, wo needn't
	 * create them again, however, we also needn't chain
	 * them into cleanup */
	if (writer.write_ptr == NULL) {
		/* if we create a writer_ptr, then we can only use
		 * a new info_ptr */
		writer.info_ptr = NULL;
		write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, png_error, png_warning);
		pcleanup->write_ptr = write_ptr;
	} else {
		write_ptr = writer.write_ptr;
		pcleanup->write_ptr_save = write_ptr;
	}

	if (write_ptr == NULL) {
		ERROR(SYSTEM, "libpng: create write_ptr error\n");
		throw_exception(EXCEPTION_CONTINUE, "libpng error");
	}

	/* We have to use setjmp here, because if we 
	 * neglect to set up our own setjmp(), libpng will
	 * call abort(). */
	if (setjmp(png_jmpbuf(write_ptr))) {
		ERROR(SYSTEM, "libpng: write error\n");
		throw_exception(EXCEPTION_CONTINUE, "libpng: write error");
	}

	if (writer.info_ptr == NULL) {
		info_ptr = png_create_info_struct(write_ptr);
		pcleanup->info_ptr = info_ptr;
	} else {
		info_ptr = writer.info_ptr;
	}

	/* use our custom writer  */
	png_set_write_fn(write_ptr, writer.io_ptr,
			writer.write_fn, writer.flush_fn);

	/* write routine */

	png_set_IHDR(write_ptr, info_ptr, w, h, 8, type,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	texts[0].key = "Software";
	texts[0].text = "yaavg";
	texts[0].compression = PNG_TEXT_COMPRESSION_NONE;

	png_set_text(write_ptr, info_ptr, texts, 1);
	png_write_info(write_ptr, info_ptr);


	int n, row_size;
	switch (type) {
		case PNG_COLOR_TYPE_RGB:
			row_size = 3 * w;
			break;
		case PNG_COLOR_TYPE_RGBA:
			row_size = 4 * w;
			break;
		default:
			FATAL(SYSTEM, "png write format error: %d\n", type);
			throw_exception(FATAL, "png write format error");
	}

	for (n = h - 1; n >= 0; n--) {
		uint8_t * prow = buffer + n * row_size;
		png_write_rows(write_ptr, &prow, 1);
	}
	png_write_end(write_ptr, info_ptr);
}

/* ********************************************************** */
static void
file_writer(png_structp str, png_bytep byte, png_size_t size)
{
	int err;
	FILE * fp = (FILE*)(str->io_ptr);
	uint8_t * buffer = (uint8_t*)byte;

	assert(fp != NULL);
	err = fwrite(buffer, 1, size, fp);
	if (err != size) {
		WARNING(SYSTEM, "file write error: %d\n", err);
		throw_exception(EXCEPTION_CONTINUE, "file write error");
	}
	return;
}

static void
file_flusher(png_structp str)
{
	int err;
	FILE * fp = (FILE*)(str->io_ptr);
	fflush(fp);
}


static void
png_write_file(FILE * fp, uint8_t * buffer, int w, int h, int type)
{
	struct png_writer writer;
	writer.write_fn = file_writer;
	writer.flush_fn = file_flusher;
	writer.io_ptr = fp;
	writer.write_ptr = NULL;
	writer.info_ptr = NULL;
	png_write(writer, buffer, w, h, type);
}

/* **************************************************************8 */

struct write_file_cleanup {
	struct cleanup base;
	FILE * fp;
};

static void
do_write_file_cleanup(struct cleanup * base)
{
	struct write_file_cleanup * pcleanup =
		(struct write_file_cleanup *)base->args;
	if (pcleanup->fp != NULL) {
		fflush(pcleanup->fp);
		fclose(pcleanup->fp);
	}

	free(pcleanup);
}

static struct write_file_cleanup *
alloc_write_file_cleanup(void)
{
	struct write_file_cleanup * pcleanup;
	pcleanup = calloc(1, sizeof(*pcleanup));
	assert(pcleanup != NULL);
	pcleanup->base.function = do_write_file_cleanup;
	pcleanup->base.args = pcleanup;
	return pcleanup;
}

static void
write_to_pngfile(char * filename, uint8_t * buffer,
		int w, int h, int type)
{
	struct write_file_cleanup * pcleanup;
	FILE * fp;

	if ((type != PNG_COLOR_TYPE_RGB) && (type != PNG_COLOR_TYPE_RGBA)) {
		FATAL(SYSTEM, "write png format error: %d\n", type);
		throw_exception(EXCEPTION_FATAL, "write png format error");
	}

	fp = fopen(filename, "wb");
	if (NULL == fp) {
		WARNING(SYSTEM, "open file %s failed\n", filename);
		throw_exception(EXCEPTION_CONTINUE,
				"open file for write failed");
		return;
	}
	
	pcleanup = alloc_write_file_cleanup();
	assert(pcleanup != NULL);
	pcleanup->fp = fp;
	make_cleanup(&pcleanup->base);

	png_write_file(fp, buffer, w, h, type);
}

void
write_to_pngfile_rgb(char * filename, uint8_t * buffer, int w, int h)
{
	write_to_pngfile(filename, buffer, w, h, PNG_COLOR_TYPE_RGB);
}

void
write_to_pngfile_rgba(char * filename, uint8_t * buffer, int w, int h)
{
	write_to_pngfile(filename, buffer, w, h, PNG_COLOR_TYPE_RGBA);
}

