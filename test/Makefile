CXX=clang++
CXXFLAGS=-c -Wall -I../src -std=c++11 -O
LD=clang++
LDFLAGS=
LIBS=

SRCDIR=../src/
TARGET=nes
OBJS=main.o config.o nes.o core.o nesfortest.o mapper.o vmapper.o cpu.o testcpu.o ppu.o vppu.o apu.o pad.o vpad.o events.o

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS}

main.o:main.cpp ${SRCDIR}cpu.h ${SRCDIR}nes.h ${SRCDIR}ppu.h ${SRCDIR}apu.h ${SRCDIR}mapper.h 
	${CXX} ${CXXFLAGS} main.cpp
nes.o: ${SRCDIR}nes.cpp ${SRCDIR}nes.h ${SRCDIR}cpu.h ${SRCDIR}ppu.h ${SRCDIR}mapper.h ${SRCDIR}vmapper.h ${SRCDIR}pad.h
	${CXX} ${CXXFLAGS} ${SRCDIR}nes.cpp
nesfortest.o: nesfortest.cpp nesfortest.h ${SRCDIR}nes.h ${SRCDIR}cpu.h ${SRCDIR}ppu.h ${SRCDIR}mapper.h ${SRCDIR}vmapper.h
	${CXX} ${CXXFLAGS} nesfortest.cpp
mapper.o: ${SRCDIR}mapper.cpp ${SRCDIR}mapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}mapper.cpp
vmapper.o: ${SRCDIR}vmapper.cpp ${SRCDIR}vmapper.h ${SRCDIR}mapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}vmapper.cpp
config.o: ${SRCDIR}config.cpp ${SRCDIR}config.h
	${CXX} ${CXXFLAGS} ${SRCDIR}config.cpp
core.o: ${SRCDIR}core.cpp ${SRCDIR}core.h
	${CXX} ${CXXFLAGS} ${SRCDIR}core.cpp
cpu.o: ${SRCDIR}cpu.cpp ${SRCDIR}cpu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}cpu.cpp
testcpu.o: testcpu.cpp testcpu.h ${SRCDIR}cpu.h
	${CXX} ${CXXFLAGS} testcpu.cpp
ppu.o: ${SRCDIR}ppu.cpp ${SRCDIR}ppu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}ppu.cpp
vppu.o: ${SRCDIR}vppu.cpp ${SRCDIR}vppu.h ${SRCDIR}ppu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}vppu.cpp
apu.o: ${SRCDIR}apu.cpp ${SRCDIR}apu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}apu.cpp
pad.o: ${SRCDIR}pad.cpp ${SRCDIR}pad.h
	${CXX} ${CXXFLAGS} ${SRCDIR}pad.cpp
vpad.o: ${SRCDIR}vpad.cpp ${SRCDIR}vpad.h ${SRCDIR}pad.h
	${CXX} ${CXXFLAGS} ${SRCDIR}vpad.cpp
events.o: ${SRCDIR}events.cpp ${SRCDIR}events.h
	${CXX} ${CXXFLAGS} ${SRCDIR}events.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
