AM_CPPFLAGS= -I$(top_srcdir)/src $(SDL_CFLAGS)
noinst_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=debug.c \
		     exception.c \
		     utils_sdl.c \
		     utils.h \
		     debug.h \
		     exception.h \
		     defs.h \
		     list.h
libcommon_la_LIBADD=$(SDL_LIBS)


noinst_PROGRAMS = test_debug test_debug2 test_exception
test_debug_SOURCES=test_debug.c
test_debug_LDADD=libcommon.la

test_debug2_SOURCES=test_debug2.c
test_debug2_LDADD=libcommon.la

test_exception_SOURCES=test_exception.c
test_exception_LDADD=libcommon.la

