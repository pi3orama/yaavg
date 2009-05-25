SET(SRC econfig.c
	econfig.h)

ADD_LIBRARY(econfig STATIC ${SRC})

IF(BUILD_TESTS)
	ADD_EXECUTABLE(test_econfig test_econfig.c)
	TARGET_LINK_LIBRARIES(test_econfig econfig)
ENDIF(BUILD_TESTS)

