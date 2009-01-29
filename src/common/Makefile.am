noinst_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=debug.c \
		     debug.h

AM_CPPFLAGS= -I$(top_srcdir)/src
bin_PROGRAMS = test_debug
test_debug_SOURCES=test_debug.c
test_debug_LDADD=libcommon.la

