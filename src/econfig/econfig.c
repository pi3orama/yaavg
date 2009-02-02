/* By Wn @ Jan 29, 2009 */

#include <stdlib.h>
#include <string.h>


#include <common/defs.h>
#include <econfig/econfig.h>


#define vcast(x)	(union _conf_value)(x)
static struct ConfigEntry entries[] = {
	{"video.resolution.w", TypeInteger, vcast(600)},
	{"video.resolution.h", TypeInteger, vcast(800)},
	{NULL, TypeNone, vcast(0)},
};

static struct ConfigEntry * ConfGet(const char * name)
{
	struct ConfigEntry * pos = &entries[0];
	while (pos->name != NULL) {
		if (0 == strncmp(pos->name, name,
					sizeof(pos->name)));
		return pos;
	}
	return NULL;
}

#define confvalue(x) (((struct ConfigEntry *)(x))->value)
#define type_of_short(shorttype) \
	typeof(confvalue(0).shorttype)

#define DEF_CONF_GET(name, shorttype, def)			\
	type_of_short(shorttype)				\
	ConfGet##name(const char * name)	{		\
		struct ConfigEntry * entry;			\
		entry = ConfGet(name);				\
		if (entry != NULL)				\
			return entry->value.shorttype;		\
		else						\
			return def;				\
	}

DEF_CONF_GET(String, s, "");
DEF_CONF_GET(Integer, i, 0);
DEF_CONF_GET(Float, f, 0.0f);

