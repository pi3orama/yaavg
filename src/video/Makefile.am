AM_CPPFLAGS= -I$(top_srcdir)/src $(GL_CFLAGS) $(SDL_CFLAGS) $(LIBPNG_CFLAGS)

noinst_LTLIBRARIES=libvideo.la

BUILT_SOURCES=gl_funcs.h
CLEANFILES = gl_funcs.h

EXTRA_DIST=generate_gl_funcs.py gl_funcs_list
libvideo_la_SOURCES=rlist.c rcommand.c \
		    rlist.h rcommand.h \
		    video.h video_gl.c video.c \
		    video_gl.h video_gl_sdl.c \
		    video_gl_glx.c	\
		    video.h texture.h texture.c texture_gl.c \
		    video_screenshot.c
libvideo_la_LIBADD=$(SDL_LIBS) $(GL_LIBS) $(LIBPNG_LIBS)

noinst_PROGRAMS=test_rlist

test_rlist_SOURCES=test_rlist.c
test_rlist_LDADD=libvideo.la ../common/libcommon.la

gl_funcs.h: generate_gl_funcs.py gl_funcs_list
	python $(top_srcdir)/src/video/generate_gl_funcs.py \
	       	$(top_srcdir)/src/video/gl_funcs_list > $@


