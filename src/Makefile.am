SUBDIRS = common econfig video event spikes

bin_PROGRAMS = test_video

test_video_SOURCES=test_video.c

test_video_LDADD= video/libvideo.la \
		 econfig/libeconfig.la \
		 event/libevent.la \
		 common/libcommon.la 


