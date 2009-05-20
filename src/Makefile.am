AM_CPPFLAGS= -I$(top_srcdir)/src $(GL_CFLAGS) $(SDL_CFLAGS) $(LIBPNG_CFLAGS)

SUBDIRS = common econfig resource video event spikes

bin_PROGRAMS = test_video test_read_pngfile

test_video_SOURCES=test_video.c

test_video_LDADD= video/libvideo.la \
		 econfig/libeconfig.la \
		 event/libevent.la \
		 common/libcommon.la \
		 econfig/libeconfig.la \
		 resource/libresource.la

test_read_pngfile_SOURCES=test_read_pngfile.c
test_read_pngfile_LDADD= video/libvideo.la \
		 econfig/libeconfig.la \
		 event/libevent.la \
		 common/libcommon.la \
		 econfig/libeconfig.la \
		 resource/libresource.la


