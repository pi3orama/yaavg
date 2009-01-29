/* 
 * WN @ Jan 24, 2009
 */

/* Debug facility */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <memory.h>

#ifdef HAVE_EXECINFO_H
/* backtrace */
# include <execinfo.h>
#endif

#define YAAVG_DEBUG_C /* Debug masks */
#include "common/debug.h"

static FILE * fdebug_out = NULL;

static void debug_close()
{
	fclose(fdebug_out);
	fdebug_out = NULL;
}

#ifdef debug_init
#undef debug_init
#endif
void debug_init(const char * filename)
{
	int i;
	if (filename == NULL)
		fdebug_out = stderr;
	else {
		fdebug_out = fopen(filename, "a");
		assert(fdebug_out != NULL);
		atexit(debug_close);
	}

}


#ifdef set_comp_level
#undef set_comp_level
#endif
void set_comp_level(enum debug_component comp, enum debug_level level)
{
	debug_levels[comp] = level;
}

static const char * get_comp_name(enum debug_component comp)
{
	return debug_comp_name[comp];
}

static const char * get_level_name(enum debug_level level)
{
	if (level == SILENT)
		return "silent";
	if (level == VERBOSE)
		return "verbose";
	if (level == TRACE)
		return "trace";
	if (level == WARNING)
		return "warning";
	if (level == ERROR)
		return "error";
	if (level == FATAL)
		return "fatal";
	return "unknown";

}

#ifdef debug_out
#undef debug_out
#endif
void debug_out(int prefix, enum debug_level level, enum debug_component comp,
	       const char * func_name, int line_no,
       	       const char * fmt, ...)
{
	va_list ap;
	/* output to stderr even if haven't init */
	if (fdebug_out == NULL)
		fdebug_out = stderr;

	if (debug_levels[comp] <= level) {
		if (prefix) {
			fprintf (fdebug_out, "[%s %s@%s:%d]\t", get_comp_name(comp),
					get_level_name(level), func_name, line_no);
		}
		va_start(ap, fmt);
		vfprintf (fdebug_out, fmt, ap);
		va_end(ap);
		fflush(fdebug_out);
	}
}

#ifdef debug_backtrace
#undef debug_backtrace
#endif

/* Define backtrace facilities */
static void * buffer[256];
static inline void debug_backtrace(FILE * fp)
{
#ifdef HAVE_BACKTRACE
	size_t count, i;
	char **strings;

	count = backtrace(buffer, 256);
	strings = backtrace_symbols(buffer, count);

	for (i = 0; i < count; i++)
		fprintf(fp, "%s\n", strings[i]);
#ifdef free
# undef free
#endif
	free(strings);
#endif
	return;
}

void __bug_on(const char * __assertion, const char * __file,
	                unsigned int __line, const char * __function)
		 __attribute__((__noreturn__));

void __bug_on(const char * __assertion, const char * __file,
	                unsigned int __line, const char * __function)
{
	FILE * files[2];
	int i;

	if (fdebug_out == NULL)
		fdebug_out = stderr;

	files[0] = stderr;
	if (fdebug_out != stderr)
		files[1] = fdebug_out;
	else
		files[1] = NULL;

	for (i = 0; i < 2; i++) {
		if (files[i] == NULL)
			break;
		fprintf(files[i], "Bug on %s (%s:%d): assertion `%s' failed\n",
				__function, __file, __line, __assertion);
		debug_backtrace(files[i]);
	}

	exit(1);
}

/* Memory leak detection */
static int malloc_times = 0;
static int calloc_times = 0;
static int free_times = 0;
static int strdup_times = 0;

#define MEM_MSG(str...) DEBUG_MSG(VERBOSE, MEMORY, str)
#define MEM_TRACE(str...) DEBUG_MSG(TRACE, MEMORY, str)

void show_mem_info()
{
	MEM_MSG("malloc times:\t %d\n", malloc_times);
	MEM_MSG("calloc times:\t %d\n", calloc_times);
	MEM_MSG("free times:\t %d\n", free_times);
	MEM_MSG("strdup times:\t %d\n", strdup_times);

	if (free_times != malloc_times +
			calloc_times +
			strdup_times)
		MEM_MSG("Memery leak found!!!!\n");
	else
		MEM_MSG("No memory leak found.\n");
	return;
}

void * yaavg_malloc(size_t size)
{
	void * res;
	res = malloc(size);
	assert(res != NULL);
	MEM_TRACE("malloc %d, res=%p\n", size, res);
	malloc_times ++;
	return res;
}


void yaavg_free(void * ptr)
{
	MEM_TRACE("free %p\n", ptr);
	free(ptr);
	free_times ++;
	return;
}

void * yaavg_calloc(size_t count, size_t eltsize)
{
	void * res = NULL;
	res = calloc (count, eltsize);
	assert(res != NULL);
	MEM_TRACE("calloc %d\tres=%p\n", count, res);
	calloc_times ++;
	return res;
}

char * yaavg_strdup(const char * S)
{
	char * res = NULL;
	res = strdup (S);
	assert(res != NULL);
	MEM_TRACE("strdup %s\tres=%p\n", S, res);
	strdup_times ++;
	return res;
}

