AM_CPPFLAGS= -I$(top_srcdir)/src $(SDL_CFLAGS)
noinst_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=debug.c \
		     exception.c \
		     utils.c \
		     utils_common.c \
		     utils_png.c \
		     utils_linux.c \
		     mm.c \
		     utils.h \
		     debug.h \
		     exception.h \
		     defs.h \
		     mm.h \
		     math.h \
		     list.h

libcommon_la_LIBADD= -lrt $(SDL_LIBS) $(LIBPNG_LIBS) 


noinst_PROGRAMS = test_debug test_debug2 test_exception test_exception2 \
		  test_geom
test_debug_SOURCES=test_debug.c
test_debug_LDADD=libcommon.la

test_debug2_SOURCES=test_debug2.c
test_debug2_LDADD=libcommon.la

test_exception_SOURCES=test_exception.c
test_exception_LDADD=libcommon.la

test_exception2_SOURCES=test_exception2.c
test_exception2_LDADD=libcommon.la

test_geom_SOURCES=test_geom.c
test_geom_LDADD=libcommon.la

