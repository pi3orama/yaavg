noinst_LTLIBRARIES=libeconfig.la
noinst_PROGRAMS=test_econfig
libeconfig_la_SOURCES=econfig.c \
		     econfig.h

test_econfig_SOURCES=test_econfig.c
test_econfig_LDADD=libeconfig.la

AM_CPPFLAGS= -I$(top_srcdir)/src

