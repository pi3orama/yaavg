AM_CPPFLAGS= -I$(top_srcdir)/src $(GL_CFLAGS) $(SDL_CFLAGS)

noinst_LTLIBRARIES=libvideo.la

BUILT_SOURCES=gl_funcs.h
CLEANFILES = gl_funcs.h

EXTRA_DIST=generate_gl_funcs.py gl_funcs_list
libvideo_la_SOURCES=rlist.c rcommand.c \
		    rlist.h rcommand.h \
		    engine.h engine_gl.c engine.c \
		    engine_gl.h engine_gl_sdl.c \
		    video.h texture.h texture_gl.c
libvideo_la_LIBADD=$(SDL_LIBS) $(GL_LIBS)

noinst_PROGRAMS=test_rlist
test_rlist_SOURCES=test_rlist.c
test_rlist_LDADD=libvideo.la

gl_funcs.h: generate_gl_funcs.py gl_funcs_list
	python $(top_srcdir)/src/video/generate_gl_funcs.py \
	       	$(top_srcdir)/src/video/gl_funcs_list > $@


