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
//	{"video.resolution.w", TypeInteger, vcast(1280)},
//	{"video.resolution.h", TypeInteger, vcast(800)},
	{"video.resolution.w", TypeInteger, vcast(800)},
	{"video.resolution.h", TypeInteger, vcast(600)},
	{"video.mspf.fallback", TypeInteger, vcast(100)},
	{"video.mspf", TypeInteger, vcast(17)},	/* 17ms: 60fps */
//	{"video.fullscreen", TypeBool, vcast(TRUE)},
	{"video.fullscreen", TypeBool, vcast(FALSE)},
//	{"video.resizable", TypeBool, vcast(TRUE)},
	{"video.resizable", TypeBool, vcast(FALSE)},
	{"video.opengl.driver.gllibrary", TypeString, vcast((const char*)"/home/wn/src/Mesa-7.0.1/lib/libGL.so")},
//	{"video.opengl.driver.gllibrary", TypeString, vcast((const char*)NULL)},
//	{"video.opengl.driver.gllibrary", TypeString, vcast((const char*)"/usr/lib/libGL.so")},
//	{"video.opengl.driver.bpp", TypeInteger, vcast(32)},
	{"video.opengl.driver.vsync", TypeBool, vcast(FALSE)},
//	{"video.opengl.driver.vsync", TypeBool, vcast(TRUE)},
//	{"video.opengl.driver.multisample", TypeInteger, vcast(0)},
//	{"video.opengl.driver.multisample", TypeInteger, vcast(4)},
	{"video.screenshotdir", TypeString, vcast((const char *)"/tmp")},
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

#define DEF_CONF_GET(TYPE, shorttype)			\
	type_of_short(shorttype)				\
	ConfGet##TYPE(const char * name, type_of_short(shorttype) def)	{		\
		struct ConfigEntry * entry;			\
		entry = ConfGet(name);				\
		if (entry != NULL)				\
			return entry->value.shorttype;		\
		else						\
			return def;				\
	}

#define DEF_CONF_SET(TYPE, shorttype)			\
	void						\
	ConfSet##TYPE(const char * name, type_of_short(shorttype) v)	{	\
		struct ConfigEntry * entry;			\
		entry = ConfGet(name);				\
		if (entry != NULL)				\
			entry->value.shorttype = v;		\
	}

DEF_CONF_GET(String, s)
DEF_CONF_GET(Integer, i)
DEF_CONF_GET(Float, f)
DEF_CONF_GET(Bool, b)

DEF_CONF_SET(String, s)
DEF_CONF_SET(Integer, i)
DEF_CONF_SET(Float, f)
DEF_CONF_SET(Bool, b)

void ConfInit(int argc, char * argv[])
{
	return;
}

