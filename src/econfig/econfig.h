/* by WN @ Jan 29, 2009 */
/* Throw aray config subsystem definitions */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <common/defs.h>

__BEGIN_DECLS

typedef enum _ConfType {
	TypeNone = 0,
	TypeInteger,
	TypeString,
	TypeFloat,
	TypeBool,
} ConfType;

struct ConfigEntry {
	const char * name;
	ConfType type;
	union _conf_value{
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
extern void ConfInit(int argc, char * argv[]);

extern const char * ConfGetString(const char * name, const char * def);
extern int ConfGetInteger(const char * name, int def);
extern float ConfGetFloat(const char * name, float def);
extern bool_t ConfGetBool(const char * name, bool_t def);

extern void ConfSetString(const char * name, const char * v);
extern void ConfSetInteger(const char * name, int v);
extern void ConfSetFloat(const char * name, float v);
extern void ConfSetBool(const char * name, bool_t v);


__END_DECLS

#endif

