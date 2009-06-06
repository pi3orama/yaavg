/* By Wn @ Jan 29, 2009 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <common/debug.h>
#include <common/defs.h>
#include <econfig/econfig.h>


#define vcast(x)	(union _conf_value)(x)
static struct conf_entry entries[] = {
//	{"video.resolution.w", TypeInteger, vcast(1280)},
//	{"video.resolution.h", TypeInteger, vcast(800)},
	{"video.resolution.w", TypeInteger, vcast(800)},
	{"video.resolution.h", TypeInteger, vcast(600)},
	{"video.viewport.w", TypeInteger, vcast(800)},
	{"video.viewport.h", TypeInteger, vcast(600)},
//	{"video.viewport.w", TypeInteger, vcast(1024)},
//	{"video.viewport.h", TypeInteger, vcast(768)},
	{"video.mspf.fallback", TypeInteger, vcast(100)},
//	{"video.mspf", TypeInteger, vcast(0)},
	{"video.mspf", TypeInteger, vcast(17)},	/* 17ms: 60fps */
//	{"video.fullscreen", TypeBool, vcast(FALSE)},
	{"video.fullscreen", TypeBool, vcast(TRUE)},
	/* full screen engine,
	 * 0 is default,
	 * 1 is XVID,
	 * 2 is XME,
	 * 3 is XRANDR
	 * 4 is trival */
	{"video.gl.glx.fullscreen.engine", TypeInteger, vcast(0)},
//	{"video.fullscreen", TypeBool, vcast(FALSE)},
//	{"video.resizable", TypeBool, vcast(TRUE)},
	{"video.resizable", TypeBool, vcast(FALSE)},
	{"video.opengl.gllibrary", TypeString, vcast((const char*)"/home/wn/src/Mesa-7.0.1/lib/libGL.so")},
//	{"video.opengl.gllibrary", TypeString, vcast((const char*)NULL)},
//	{"video.opengl.gllibrary", TypeString, vcast((const char*)"/usr/local/lib/libGL.so")},
	{"video.opengl.bpp", TypeInteger, vcast(32)},
	/* vsync related */
	{"video.opengl.swapcontrol", TypeInteger, vcast(0)},
	{"video.opengl.multisample", TypeInteger, vcast(0)},
//	{"video.opengl.multisample", TypeInteger, vcast(4)},
	{"video.screenshotdir", TypeString, vcast((const char *)"/tmp")},
	{"video.caption", TypeString, vcast((const char *)"test")},
	/* If the SIGINT is raised during SwapBuffer, the SDL cleanup
	 * function cause deadlock.  */
	{"video.sdl.blocksigint", TypeBool, vcast(TRUE)},
//	{"video.sdl.blocksigint", TypeBool, vcast(FALSE)},
	{"sys.mem.threshold",	TypeInteger, vcast(1048576)},	/* in Kbs, no use now */
	{"video.opengl.texture.totalhwsize", TypeInteger, vcast(0)},	/* in Kbs, no use now, 0 means don't care */
	/* 0 means use max avaliable size */	
	{"video.opengl.texture.maxsize", TypeInteger, vcast(0)},
	{"video.opengl.texture.enableCOMPRESSION", TypeBool, vcast(TRUE)},
	{"video.opengl.texture.enableNPOT", TypeBool, vcast(TRUE)},
	{"video.opengl.texture.enableRECT", TypeBool, vcast(TRUE)},
	{"video.opengl.glx.confinemouse", TypeBool, vcast(FALSE)},
//	{"video.opengl.glx.confinemouse", TypeBool, vcast(TRUE)},
	/* Don't set it to TRUE unless you're sure what you're doing */
	{"video.opengl.glx.grabkeyboard", TypeBool, vcast(FALSE)},
	{NULL, TypeNone, vcast(0)},
};

static struct conf_entry *
conf_get(const char * name)
{
	struct conf_entry * pos = &entries[0];
	assert(name != NULL);
	assert(name[0] != '\0');
	while (pos->name != NULL) {
		if (0 == strncmp(pos->name, name, strlen(pos->name)))
			return pos;
		pos ++;
	}
	return NULL;
}

#define confvalue(x) (((struct conf_entry *)(x))->value)
#define type_of_short(shorttype) \
	typeof(confvalue(0).shorttype)

#define DEF_CONF_GET(TYPE, shorttype)			\
	type_of_short(shorttype)				\
	conf_get_##TYPE(const char * name, type_of_short(shorttype) def) { \
		struct conf_entry * entry;			\
		entry = conf_get(name);				\
		if (entry != NULL)				\
			return entry->value.shorttype;		\
		else						\
			return def;				\
	}

#define DEF_CONF_SET(TYPE, shorttype)			\
	void						\
	conf_set_##TYPE(const char * name, type_of_short(shorttype) v)	{	\
		struct conf_entry * entry;			\
		entry = conf_get(name);				\
		if (entry != NULL)				\
			entry->value.shorttype = v;		\
	}

DEF_CONF_GET(string, s)
DEF_CONF_GET(integer, i)
DEF_CONF_GET(float, f)
DEF_CONF_GET(bool, b)

DEF_CONF_SET(string, s)
DEF_CONF_SET(integer, i)
DEF_CONF_SET(float, f)
DEF_CONF_SET(bool, b)

void conf_init(int argc, char * argv[])
{
	return;
}

