/* by WN @ Jan 29, 2009 */
/* Throw aray config subsystem definitions */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <common/defs.h>

__BEGIN_DECLS

typedef enum _conf_type {
	TypeNone = 0,
	TypeInteger,
	TypeString,
	TypeFloat,
	TypeBool,
} conf_type;

struct conf_entry {
	const char * name;
	conf_type type;
	union _conf_value {
		const char * s;
		int i;
		float f;
		bool_t b;
	} value;
};

/*
 * Config initialize. command line options and ENV can overwrite
 * default config value.
 * priorities:
 * 	command line > env > config file > default
 */
extern void conf_init(int argc, char * argv[]);

extern const char * conf_get_string(const char * name, const char * def);
extern int conf_get_integer(const char * name, int def);
extern float conf_get_float(const char * name, float def);
extern bool_t conf_get_bool(const char * name, bool_t def);

extern void conf_set_string(const char * name, const char * v);
extern void conf_set_integer(const char * name, int v);
extern void conf_set_float(const char * name, float v);
extern void conf_set_bool(const char * name, bool_t v);

#define max(a, b)	(((a) > (b)) ? (a) : (b))
#define min(a, b)	(((a) < (b)) ? (a) : (b))

#define max0(a, b)	max(a, b)
#define min0(a, b)	((((a) < (b)) && ((a) != 0)) ? (a) : (b))

__END_DECLS

#endif

