AM_CPPFLAGS= -I$(top_srcdir)/src

noinst_LTLIBRARIES=libresource.la
libresource_la_SOURCES=resource.c \
		       resource.h

noinst_PROGRAMS=test_bitmap

test_bitmap_SOURCES=test_bitmap.c
test_bitmap_LDADD=libresource.la ../common/libcommon.la


