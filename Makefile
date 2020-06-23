
ARCH=$(shell uname)-$(shell uname -p)
CC=gcc
CPP=g++ -std=c++11
LD=g++ -g
AR=ar
CFLAGS=-pipe -g -O2 -Wall -fPIC 
LIBS=-L${BUILDDIR} -lfltklayout `fltk-config --ldflags` -pthread
INC=-I.
LDFLAGS=

BUILDDIR=build-${ARCH}

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.cpp)
SOURCES+=$(wildcard *.cxx)
SOURCES+=$(wildcard *.c)

OBJS=${BUILDDIR}/fltklayout.o

all: info .depends libfltklayout.a fltklayout_designer test test2 test3

${BUILDDIR}/.dir:
	mkdir -p ${BUILDDIR}
	touch ${BUILDDIR}/.dir

clean:
	rm -rf .depends ${BUILDDIR} libfltklayout.a fltklayout_designer test

git-clean: clean

info:
	@echo && echo "==== Building fltklayout ====" && echo

${BUILDDIR}/fltklayout_designer: ${BUILDDIR}/fltklayout_designer.o ${BUILDDIR}/libfltklayout.a
	$(LD) -o ${BUILDDIR}/fltklayout_designer ${BUILDDIR}/fltklayout_designer.o $(LDFLAGS) $(LIBS)

fltklayout_designer: ${BUILDDIR}/fltklayout_designer
	cp ${BUILDDIR}/fltklayout_designer .

${BUILDDIR}/test: ${BUILDDIR}/test.o ${BUILDDIR}/libfltklayout.a
	$(LD) -o ${BUILDDIR}/test ${BUILDDIR}/test.o $(LDFLAGS) $(LIBS)

test: ${BUILDDIR}/test
	cp ${BUILDDIR}/test .

${BUILDDIR}/test2: ${BUILDDIR}/test2.o ${BUILDDIR}/libfltklayout.a
	$(LD) -o ${BUILDDIR}/test2 ${BUILDDIR}/test2.o $(LDFLAGS) $(LIBS)

test2: ${BUILDDIR}/test2
	cp ${BUILDDIR}/test2 .

${BUILDDIR}/test3: ${BUILDDIR}/test3.o ${BUILDDIR}/libfltklayout.a
	$(LD) -o ${BUILDDIR}/test3 ${BUILDDIR}/test3.o $(LDFLAGS) $(LIBS)

test3: ${BUILDDIR}/test3
	cp ${BUILDDIR}/test3 .

${BUILDDIR}/libfltklayout.a: ${BUILDDIR}/.dir $(OBJS)
	$(AR) rcs ${BUILDDIR}/libfltklayout.a $(OBJS)

libfltklayout.a: ${BUILDDIR}/libfltklayout.a
	cp ${BUILDDIR}/libfltklayout.a .

.depends: $(SOURCES) $(HEADERS)
	$(CPP) $(CFLAGS) $(INC) -MM $(SOURCES) | perl -pne "print '${BUILDDIR}/' if /^[^#\s]/" > .depends

${BUILDDIR}/%.o: %.c ${BUILDDIR}/.dir
	$(CC) $(CFLAGS) -c $(INC) $< -o ${BUILDDIR}/$(patsubst %.c,%.o,$<)

${BUILDDIR}/%.o: %.cpp ${BUILDDIR}/.dir
	$(CPP) $(CFLAGS) -c $(INC) $< -o ${BUILDDIR}/$(patsubst %.cpp,%.o,$<)

${BUILDDIR}/%.o: %.cxx ${BUILDDIR}/.dir
	$(CPP) $(CFLAGS) -c $(INC) $< -o ${BUILDDIR}/$(patsubst %.cxx,%.o,$<)

.PHONY: .depends

-include .depends

