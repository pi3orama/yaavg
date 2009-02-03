AM_CPPFLAGS= -g -O0 -I$(top_srcdir)/src $(GL_CFLAGS) $(SDL_CFLAGS)

noinst_LTLIBRARIES=libvideo.la
libvideo_la_SOURCES=rlist.c rcommand.c \
		    rlist.h rcommand.h \
		    engine.h engine_gl.c engine.c \
		    engine_gl.h engine_gl_sdl.c \
		    video.h
libvideo_la_LIBADD=$(SDL_LIBS) $(GL_LIBS)

noinst_PROGRAMS=test_rlist
test_rlist_SOURCES=test_rlist.c
test_rlist_LDADD=libvideo.la

