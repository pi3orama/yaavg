SET(SRC
	vector.h
	matrix.h
	matrix.c
	math.c
	math.h
	trigon.c
	trigon.h
	sqrt.h
	)

ADD_LIBRARY(math STATIC ${SRC})

SET(CMAKE_C_FLAGS_DEBUG "-g -O3")
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

TARGET_LINK_LIBRARIES(math "-lm")


SET(tests
	test_math)
build_tests("${tests}" "common;math")


#build_tests("test_math", "common;math")

