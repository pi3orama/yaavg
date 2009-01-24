/* Wang Nan @ Jan 24, 2009 */
#ifndef YAAVG_DEBUG
#define YAAVG_DEBUG

#include <sys/cdefs.h>

__BEGIN_DECLS


enum debug_level {
	SILENT = 0,
	TRACE,
	VERBOSE,
	WARNING,
	ERROR,
	FATAL,
	NR_DEBUG_LEVELS
};

enum debug_component {
	VCOMMAND = 0,
	VLIST,
	SYSTEM,
	NR_COMPONENTS
};

#ifdef YAAVG_DEBUG_C

static const char * debug_comp_name[NR_COMPONENTS] = {
	[VCOMMAND] = "video-command",
	[VLIST] = "video-list",
	[SYSTEM] = "system"
};

static enum debug_level debug_levels[NR_COMPONENTS] = {
	[VCOMMAND] = WARNING,
	[VLIST] = TRACE,
};
#endif

#ifndef YAAVG_DEBUG_OFF
extern void debug_init(const char * filename);
extern void debug_out(int prefix, enum debug_level level, enum debug_component comp,
	       const char * func_name, int line_no,
	       const char * fmt, ...);
extern void set_comp_level(enum debug_component comp, enum debug_level level);
#else
# define debug_init(a, b) do{} while(0)
# define debug_out(z, a, b, c, d, e,...) do{} while(0)
# define set_comp_level(a, b)   do {} while(0)
#endif

#define DEBUG_INIT(file)	do { debug_init(file);} while(0)
#define DEBUG_MSG(level, comp, str...) do{ debug_out(1, level, comp, __FUNCTION__, __LINE__, str); } while(0)
/*  Don't output prefix */
#define DEBUG_MSG_CONT(level, comp, str...) do{ debug_out(0, level, comp, __FUNCTION__, __LINE__, str); } while(0)
#define DEBUG_SET_COMP_LEVEL(mask, level) do { set_comp_level(mask, level); } while(0)



/* Define bug functions. Reference: assert.h */
extern void __bug_on(const char * __assertion, const char * __file,
		unsigned int __line, const char * __function)
	__attribute__((__noreturn__));

#define BUG_ON(expr)	\
	((expr))	\
	 ? (void)0	\
	 : __bug_on(__STRING(expr), __FILE__, __LINE__, __FUNCTION__)

__END_DECLS


#endif

