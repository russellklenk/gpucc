COMMON_LIBRARIES          = -lstdc++ -lrt -lm
COMMON_HEADERS            = $(wildcard include/*.h)
COMMON_SOURCES            = $(wildcard src/*.cc) $(wildcard src/linux/*.cc)
COMMON_OBJECTS            = ${COMMON_SOURCES:.cc=.o}
COMMON_DEPENDENCIES       = ${COMMON_OBJECTS:.cc=.dep}
COMMON_INCLUDE_DIRS       = -I. -Iinclude
COMMON_LIBRARY_DIRS       = -Llibs
COMMON_WARNINGS           = -Wall -Wextra
COMMON_CCFLAGS            = -std=c++11 -fstrict-aliasing -D__STDC_FORMAT_MACROS ${COMMON_INCLUDE_DIRS} ${COMMON_WARNINGS}
COMMON_LDFLAGS            = 

TARGET1                   = target1
TARGET1_MAIN              = main/target1.cc
TARGET1_WARNINGS          = -Werror
TARGET1_LIBRARIES         = ${SDL2_LDFLAGS}
TARGET1_CCFLAGS           = -ggdb ${TARGET1_WARNINGS} ${SDL2_CFLAGS}
TARGET1_LDFLAGS           = 
TARGET1_OBJECTS           = ${TARGET1_MAIN:.cc=.o}
TARGET1_DEPENDENCIES      = ${TARGET1_MAIN:.cc=.dep}

.PHONY: all clean distclean output

all:: ${TARGET1}

${COMMON_OBJECTS}: %.o: %.cc
	${CC} ${CCFLAGS} ${COMMON_CCFLAGS} -o $@ -c $<

${TARGET1}: ${COMMON_OBJECTS} ${TARGET1_OBJECTS}
	${CC} ${LDFLAGS} ${COMMON_LDFLAGS} ${TARGET1_LDFLAGS} -o $@ $^ ${COMMON_LIBRARIES} ${TARGET1_LIBRARIES}

${TARGET1_OBJECTS}: %.o: %.cc ${TARGET1_DEPENDENCIES}
	${CC} ${CCFLAGS} ${COMMON_CCFLAGS} ${TARGET1_CCFLAGS} -o $@ -c $<

${TARGET1_DEPENDENCIES}: %.dep: %.cc ${COMMON_HEADERS} Makefile
	${CC} ${CCFLAGS} ${COMMON_CCFLAGS} ${TARGET1_CCFLAGS} -MM $< > $@

output:: ${TARGET1}

clean::
	rm -f *~ *.o *.dep src/*~ src/*.o src/*.dep src/linux/*~ src/linux/*.o src/linux/*.dep main/*~ main/*.o main/*.dep ${TARGET1}

distclean:: clean ${TARGET1}

