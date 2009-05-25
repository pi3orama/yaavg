SET(SRC resource.c
	resource.h
	bitmap.c
	bitmap.h)

ADD_LIBRARY(resource STATIC ${SRC})
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
ADD_DEFINITIONS(${PNG_DEFINITIONS})
TARGET_LINK_LIBRARIES(resource ${PNG_LIBRARIES})

TARGET_LINK_LIBRARIES(resource common)

build_tests(test_bitmap "resource;common")
# vim:tabstop=4:shiftwidth=4



