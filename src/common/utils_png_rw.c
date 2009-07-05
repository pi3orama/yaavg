/* 
 * utils_png.c
 * by WN @ Mar. 15, 2009
 *
 * png related utils
 */

#include <png.h>

#include <config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include <common/debug.h>
#include <common/exception.h>
#include <common/list.h>
#include <common/utils_png.h>
#include <common/mm.h>

#include <resource/bitmap.h>

/* ****************** READ UTILS ********************** */
struct png_reader {
	png_rw_ptr read_fn;
	png_voidp io_ptr;
	png_structp read_ptr;
	png_structp read_ptr_save;
	png_infop info_ptr;
	void * row_pointers;
};

static void
cleanup_reader(struct png_reader * reader)
{
	if (reader->read_ptr != NULL)
		png_destroy_read_struct(&reader->read_ptr,
				reader->info_ptr ? &reader->info_ptr : (png_infopp)0,
				(png_infopp)0);
	if (reader->info_ptr != NULL) {
		assert(reader->read_ptr_save != NULL);
		png_destroy_info_struct(reader->read_ptr_save,
				&reader->info_ptr);
	}
#ifndef HAVE_ALLOCA
	xfree(reader->row_pointers);
	reader->row_pointers = NULL;
#endif
	reader->read_ptr = NULL;
	reader->info_ptr = NULL;
}

static void *
do_png_read(struct png_reader * reader,
	void *(*alloc)(int w, int h, int bpp, void** datap))
{

	png_structp read_ptr;
	png_infop info_ptr;

	TRACE(SYSTEM, "ready to read png stream\n");

	if (reader->read_ptr == NULL) {
		reader->info_ptr = NULL;
		read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				NULL, png_error, png_warning);
		reader->read_ptr = read_ptr;
	} else {
		read_ptr = reader->read_ptr;
		reader->read_ptr_save = read_ptr;
	}

	if (read_ptr == NULL) {
		ERROR(SYSTEM, "libpng: create read_ptr error\n");
		THROW(EXCEPTION_CONTINUE, "libpng read error");
	}

	if (setjmp(png_jmpbuf(read_ptr))) {
		ERROR(SYSTEM, "libpng: read error\n");
		THROW(EXCEPTION_RESOURCE_LOST, "libpng read error");
	}

	if (reader->info_ptr == NULL) {
		info_ptr = png_create_info_struct(read_ptr);
		reader->info_ptr = info_ptr;
	} else {
		info_ptr = reader->info_ptr;
	}

	png_set_read_fn(read_ptr, reader->io_ptr,
			reader->read_fn);

	/* here: begin read */
	/* code borrowed from SDL_img's IMG_png.c */
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_read_info(read_ptr, info_ptr);
	png_get_IHDR(read_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	TRACE(SYSTEM, "png bitmap: %dx%d:%d\n", width, height, bit_depth);

	if (bit_depth > 8)
		png_set_strip_16(read_ptr);
	if (bit_depth < 8)
		png_set_packing(read_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY) {
		TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_GRAY\n");
		png_set_expand(read_ptr);
	}

	if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_GRAY_ALPHA\n");
		png_set_gray_to_rgb(read_ptr);
	}
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
		TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_PALETTE\n");
        png_set_palette_to_rgb(read_ptr);
	}

    if (png_get_valid(read_ptr, info_ptr, PNG_INFO_tRNS)) {
		TRACE(SYSTEM, "\tstream has PNG_INFO_tRNS\n");
		png_set_tRNS_to_alpha(read_ptr);
	}

	png_read_update_info(read_ptr, info_ptr);
	png_get_IHDR(read_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	TRACE(SYSTEM, "update png bitmap: %dx%d:%d\n", width, height, bit_depth);

	/* start read */
	if (bit_depth != 8) {
		WARNING(RESOURCE, "We don't support this png stream\n");
		THROW(EXCEPTION_RESOURCE_LOST, "format error");
	}

	int bytes_pp;

	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_GRAY\n");
			bytes_pp = 1;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_GRAY_ALPHA\n");
			bytes_pp = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_RGB\n");
			bytes_pp = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			TRACE(SYSTEM, "\tcolor type: PNG_COLOR_TYPE_RGB_ALPHA\n");
			bytes_pp = 4;
			break;
		case PNG_COLOR_TYPE_PALETTE:
			WARNING(SYSTEM,
					"PNG_COLOR_TYPE_PALETTE:"
					" I don't know how to read PALETTE png stream...\n");
			if (png_get_valid(read_ptr, info_ptr, PNG_INFO_tRNS))
				bytes_pp = 3;
			else
				bytes_pp = 4;
			break;
		default:
			WARNING(RESOURCE, "We don't support this png stream\n");
			THROW(EXCEPTION_RESOURCE_LOST, "format error");
	};

	/* alloc data */
	void * retval = NULL;
	void * data = NULL;
	retval = alloc(width, height, bytes_pp, &data);
	assert(retval != NULL);
	assert(data != NULL);

	/* start read!! */
	VERBOSE(SYSTEM, "png stream read start\n");
	png_bytep *volatile row_pointers;

	int row;
#ifdef HAVE_ALLOCA
	row_pointers = (png_bytep*)alloca(sizeof(png_bytep)*height);
#else
	row_pointers = (png_bytep*)xmalloc(sizeof(png_bytep)*height);
	reader->row_pointers = row_pointers;
#endif
	assert(row_pointers != NULL);

	for (row = 0; row < (int)height; row++) {
		row_pointers[row] = (png_bytep)(
				(uint8_t*)(data + ((int)height-row) * bytes_pp * width));
	}
	png_read_image(read_ptr, row_pointers);

#ifndef HAVE_ALLOCA
	GC_FREE_BLOCK_SET(row_pointers);
	reader->row_pointers = NULL;
#endif

	return retval;
}

struct read_file_cleanup {
	GC_TAG;
	struct cleanup base;
	struct png_reader reader;
};

static void
file_reader(png_structp str, png_bytep byte, png_size_t size)
{
	int err;
	FILE * fp = (FILE*)(str->io_ptr);
	uint8_t * buffer = (uint8_t*)byte;

	assert(fp != NULL);
	err = fread(buffer, 1, size, fp);
	if (err == 0) {
		/* error occurs or end of file */
		if (!feof(fp)) {
			WARNING(SYSTEM, "file read error: %d\n", errno);
			THROW(EXCEPTION_RESOURCE_LOST, "file read error");
		}
	}
	return;
}

static void
do_read_file_cleanup(struct cleanup * base)
{
	struct read_file_cleanup * pcleanup;
	remove_cleanup(base);

	pcleanup = container_of(base, struct read_file_cleanup, base);
	struct png_reader * reader = &pcleanup->reader;
	cleanup_reader(reader);

	if (reader->io_ptr != NULL) {
		FILE * fp = reader->io_ptr;
		fclose(fp);
		reader->io_ptr = NULL;
	}
	GC_TRIVAL_FREE(pcleanup);
}

void *
read_from_pngfile(char * filename,
	void *(*alloc)(int w, int h, int bpp, void** datap))
{
	struct read_file_cleanup * clup;
	void * retval;

	VERBOSE(SYSTEM, "read bitmap data from png file %s\n",
			filename);

	FILE * fp = fopen(filename, "rb");
	if (NULL == fp) {
		WARNING(SYSTEM, "open file %s for read failed\n", filename);
		THROW(EXCEPTION_RESOURCE_LOST,
				"open file for read error\n");
	}

	clup = GC_TRIVAL_CALLOC(clup);
	assert(clup != NULL);
	clup->base.function = do_read_file_cleanup;

	struct png_reader * reader = &clup->reader;
	reader->io_ptr = fp;
	reader->read_fn = file_reader;
	reader->read_ptr = NULL;
	reader->info_ptr = NULL;

	make_cleanup(&clup->base);

	retval = do_png_read(reader, alloc);
	CLEANUP(&clup->base);
	return retval;
}

/* ******* WRITE UTILS ************ */

struct png_writer {
	png_rw_ptr write_fn;
	png_flush_ptr flush_fn;
	png_voidp io_ptr;
	png_structp write_ptr;
	png_structp write_ptr_save;
	png_infop info_ptr;
};

static void
cleanup_writer(struct png_writer * writer)
{
	if (writer->write_ptr != NULL)
		png_destroy_write_struct(&writer->write_ptr,
				writer->info_ptr ? &writer->info_ptr : (png_infopp)0);
	if (writer->info_ptr != NULL) {
		assert(writer->write_ptr_save != NULL);
		png_destroy_info_struct(writer->write_ptr_save,
				&writer->info_ptr);
	}

	writer->write_ptr = NULL;
	writer->info_ptr = NULL;
}

static void
do_png_write(struct png_writer * writer, void * buffer, int w, int h, int type)
{
	TRACE(SYSTEM, "write a %dx%d image into a png stream\n", w, h);
	
	png_structp write_ptr;
	png_infop info_ptr;
	png_text texts[1];

	/* if png handler is passed from writer, we needn't
	 * create them again, however, we also needn't chain
	 * them into cleanup */
	if (writer->write_ptr == NULL) {
		/* if we create a writer_ptr, then we can only use
		 * a new info_ptr */
		writer->info_ptr = NULL;
		write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				NULL, png_error, png_warning);
		writer->write_ptr = write_ptr;
	} else {
		write_ptr = writer->write_ptr;
		writer->write_ptr_save = write_ptr;
	}

	if (write_ptr == NULL) {
		ERROR(SYSTEM, "libpng: create write_ptr error\n");
		THROW(EXCEPTION_CONTINUE, "libpng error");
	}

	/* We have to use setjmp here, because if we 
	 * neglect to set up our own setjmp(), libpng will
	 * call abort(). */
	if (setjmp(png_jmpbuf(write_ptr))) {
		ERROR(SYSTEM, "libpng: write error\n");
		THROW(EXCEPTION_CONTINUE, "libpng: write error");
	}
	
	if (writer->info_ptr == NULL) {
		info_ptr = png_create_info_struct(write_ptr);
		writer->info_ptr = info_ptr;
	} else {
		info_ptr = writer->info_ptr;
	}

	/* use our custom writer  */
	png_set_write_fn(write_ptr, writer->io_ptr,
			writer->write_fn, writer->flush_fn);

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
			THROW(FATAL, "png write format error");
	}

	VERBOSE(SYSTEM, "png stream write start\n");
	for (n = h - 1; n >= 0; n--) {
		uint8_t * prow = buffer + n * row_size;
		png_write_rows(write_ptr, &prow, 1);
	}
	png_write_end(write_ptr, info_ptr);
	cleanup_writer(writer);
}

struct write_file_cleanup {
	GC_TAG;
	struct cleanup base;
	struct png_writer writer;
};

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
		THROW(EXCEPTION_CONTINUE, "file write error");
	}
	return;
}

static void
file_flusher(png_structp str)
{
	int err;
	FILE * fp = (FILE*)(str->io_ptr);
	err = fflush(fp);
	if (err != 0) {
		WARNING(SYSTEM, "png fflush error: %s\n", strerror(errno));
	}
}

static void
do_write_file_cleanup(struct cleanup * base)
{
	struct write_file_cleanup * pcleanup;
	remove_cleanup(base);

	pcleanup = container_of(base, struct write_file_cleanup, base);

	struct png_writer * writer = &pcleanup->writer;
	cleanup_writer(writer);

	if (writer->io_ptr != NULL) {
		FILE * fp = writer->io_ptr;
		fflush(fp);
		fclose(fp);
		writer->io_ptr = NULL;
	}

	GC_TRIVAL_FREE(pcleanup);
}

static void
do_write_pngfile(char * filename, uint8_t * buffer,
		int w, int h, int type)
{
	FILE * fp = NULL;
	if ((type != PNG_COLOR_TYPE_RGB) && (type != PNG_COLOR_TYPE_RGBA)) {
		FATAL(SYSTEM, "write png format error: %d\n", type);
		THROW(EXCEPTION_CONTINUE, "write png format error");
	}

	fp = fopen(filename, "wb");
	if (NULL == fp) {
		WARNING(SYSTEM, "open file %s failed\n", filename);
		THROW(EXCEPTION_CONTINUE,
				"open file for write failed");
		return;
	}
	TRACE(SYSTEM, "file %s opened for write\n", filename);

	struct write_file_cleanup * clup;
	clup = GC_TRIVAL_CALLOC(clup);
	assert(clup != NULL);
	clup->base.function = do_write_file_cleanup;

	struct png_writer * writer = &clup->writer;
	writer->write_fn = file_writer;
	writer->flush_fn = file_flusher;
	writer->io_ptr = fp;
	writer->write_ptr = NULL;
	writer->info_ptr = NULL;

	make_cleanup(&clup->base);

	do_png_write(writer, buffer, w, h, type);
	CLEANUP(&clup->base);
	return;
}


void
write_to_pngfile_rgb(char * filename, uint8_t * buffer, int w, int h)
{
	TRACE(SYSTEM, "write a %dx%d image in RGB format into file %s\n",
			w, h, filename);
	do_write_pngfile(filename, buffer, w, h, PNG_COLOR_TYPE_RGB);
}

void
write_to_pngfile_rgba(char * filename, uint8_t * buffer, int w, int h)
{
	TRACE(SYSTEM, "write a %dx%d image in RGBA format into file %s\n",
			w, h, filename);
	do_write_pngfile(filename, buffer, w, h, PNG_COLOR_TYPE_RGBA);
}

// vim:tabstop=4:shiftwidth=4

