AM_CPPFLAGS= -I$(top_srcdir)/src
noinst_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=debug.c \
		     debug.h \
		     defs.h \
		     list.h


noinst_PROGRAMS = test_debug test_debug2
test_debug_SOURCES=test_debug.c
test_debug_LDADD=libcommon.la

test_debug2_SOURCES=test_debug2.c
test_debug2_LDADD=libcommon.la

