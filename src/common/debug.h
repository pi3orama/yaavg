/* Wang Nan @ Jan 24, 2009 */
#ifndef YAAVG_DEBUG
#define YAAVG_DEBUG

#include <sys/cdefs.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

__BEGIN_DECLS


enum debug_level {
	TRACE = 0,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL,
	SILENT,
	NR_DEBUG_LEVELS
};

enum debug_component {
	RCOMMAND = 0,
	RLIST,
	VIDEO,
	SYSTEM,
	MEMORY,
	OPENGL,
	SDL,
	NR_COMPONENTS
};

#ifdef YAAVG_DEBUG_C

static const char * debug_comp_name[NR_COMPONENTS] = {
	[RCOMMAND] = "VCOM",
	[RLIST] = "VLST",
	[SYSTEM] = "SYS ",
	[MEMORY] = "MEM ",
	[OPENGL] = "GL  ",
	[SDL] = "SDL ",
	[VIDEO]="VENG",
};

static enum debug_level debug_levels[NR_COMPONENTS] = {
	[RCOMMAND] = WARNING,
	[RLIST] = TRACE,
	[MEMORY] = VERBOSE,
	[SYSTEM] = TRACE,
	[OPENGL] = TRACE,
	[SDL] = TRACE,
	[VIDEO] = TRACE,
};
#endif

#ifndef YAAVG_DEBUG_OFF
extern void debug_init(const char * filename);
extern void debug_out(int prefix, enum debug_level level, enum debug_component comp,
	       const char * func_name, int line_no,
	       const char * fmt, ...);
extern void set_comp_level(enum debug_component comp, enum debug_level level);
extern enum debug_level get_comp_level(enum debug_component comp);
#else
# define debug_init(a, b) do{} while(0)
# define debug_out(z, a, b, c, d, e,...) do{} while(0)
# define set_comp_level(a, b)   do {} while(0)
# define get_comp_level(a, b)   do {} while(0)
#endif

#define DEBUG_INIT(file)	do { debug_init(file);} while(0)
#define DEBUG_MSG(level, comp, str...) do{ debug_out(1, level, comp, __FUNCTION__, __LINE__, str); } while(0)
/*  Don't output prefix */
#define DEBUG_MSG_CONT(level, comp, str...) do{ debug_out(0, level, comp, __FUNCTION__, __LINE__, str); } while(0)
#define DEBUG_SET_COMP_LEVEL(mask, level) do { set_comp_level(mask, level); } while(0)

#define TRACE(comp, str...) DEBUG_MSG(TRACE, comp, str)
#define VERBOSE(comp, str...) DEBUG_MSG(VERBOSE, comp, str)
#ifndef YAAVG_DEBUG_OFF
# define WARNING(comp, str...) DEBUG_MSG(WARNING, comp, str)
# define ERROR(comp, str...) DEBUG_MSG(ERROR, comp, str)
# define FATAL(comp, str...) DEBUG_MSG(FATAL, comp, str)
#else
extern void WARNING(enum debug_component, char * fmt, ...);
extern void ERROR(enum debug_component, char * fmt, ...);
extern void FATAL(enum debug_component, char * fmt, ...);
#endif


/* Define bug functions. Reference: assert.h */
/* Don't use such a BUG_ON facility, use assert is enough.
 * we use signal handler to print backtrace. */
#if 0
extern void __bug_on(const char * __assertion, const char * __file,
		unsigned int __line, const char * __function)
	__attribute__((__noreturn__));

#define BUG_ON(expr)	\
	((expr))	\
	 ? __bug_on(__STRING(expr), __FILE__, __LINE__, __FUNCTION__)	\
	 : (void)0
#endif


#ifndef YAAVG_DEBUG_OFF
/* memory leak detection */
extern void * yaavg_malloc(size_t size);
extern void yaavg_free(void * ptr);
extern char * yaavg_strdup(const char * S);
extern void * yaavg_calloc(size_t count, size_t eltsize);
extern void show_mem_info();
#ifndef YAAVG_DEBUG_C
# ifdef malloc
#  undef malloc
# endif
# define malloc(s)	yaavg_malloc(s)

# ifdef free
#  undef free
# endif
# define free(p)	yaavg_free(p)

# ifdef strdup
#  undef strdup
# endif
# define strdup(S)	yaavg_strdup(S)

# ifdef calloc
#  undef calloc
# endif
# define calloc(S)	yyavg_calloc(S)
#endif
#endif

__END_DECLS


#endif

