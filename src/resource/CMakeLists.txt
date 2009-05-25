SET(SRC resource.c
	resource.h
	bitmap.c
	bitmap.h)

ADD_LIBRARY(resource STATIC ${SRC})
ADD_DEFINITIONS(${PNG_DEFINITIONS})
TARGET_LINK_LIBRARIES(resource ${PNG_LIBRARIES})

build_tests(test_bitmap "resource;common")
# vim:tabstop=4:shiftwidth=4



