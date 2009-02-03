/* By Wn @ Jan 29, 2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <common/debug.h>
#include <common/defs.h>
#include <econfig/econfig.h>


#define vcast(x)	(union _conf_value)(x)
static struct ConfigEntry entries[] = {
	{"video.resolution.w", TypeInteger, vcast(600)},
	{"video.resolution.h", TypeInteger, vcast(800)},
	{"video.opengl.driver.gllibrary", TypeString, vcast((char*)NULL)},
	{"video.opengl.driver.bpp", TypeInteger, vcast(16)},
	{"video.opengl.driver.fullscreen", TypeBool, vcast(FALSE)},
	{"video.opengl.driver.vsync", TypeBool, vcast(TRUE)},
	{"video.opengl.driver.multisample", TypeInteger, vcast(4)},
	{NULL, TypeNone, vcast(0)},
};

static struct ConfigEntry * ConfGet(const char * name)
{
	struct ConfigEntry * pos = &entries[0];
	assert(name != NULL);
	assert(name[0] != '\0');
	while (pos->name != NULL) {
		if (0 == strncmp(pos->name, name, strlen(pos->name)))
			return pos;
		pos ++;
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
DEF_CONF_GET(Bool, b, FALSE);

void ConfInit(int argc, char * argv[])
{
	return;
}

