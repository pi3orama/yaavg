noinst_LTLIBRARIES=libcommon.la
libcommon_la_SOURCES=debug.c

AM_CPPFLAGS= -I$(top_srcdir)/src/common
bin_PROGRAMS = test_debug
test_debug_SOURCES=test_debug.c
test_debug_LDADD=libcommon.la

