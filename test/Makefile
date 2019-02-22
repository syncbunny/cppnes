CXX=clang++
CXXFLAGS=-c -Wall -I../src -std=c++11 -O
LD=clang++
LDFLAGS=
LIBS=

SRCDIR=../src/
TARGET=nes
OBJS=main.o nes.o nesfortest.o mapper.o vmapper.o cpu.o ppu.o apu.o

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS}

main.o:main.cpp ${SRCDIR}cpu.h ${SRCDIR}nes.h ${SRCDIR}ppu.h ${SRCDIR}apu.h ${SRCDIR}mapper.h 
	${CXX} ${CXXFLAGS} main.cpp
nes.o: ${SRCDIR}nes.cpp ${SRCDIR}nes.h ${SRCDIR}cpu.h ${SRCDIR}ppu.h ${SRCDIR}mapper.h ${SRCDIR}vmapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}nes.cpp
nesfortest.o: nesfortest.cpp nesfortest.h ${SRCDIR}nes.h ${SRCDIR}cpu.h ${SRCDIR}ppu.h ${SRCDIR}mapper.h ${SRCDIR}vmapper.h
	${CXX} ${CXXFLAGS} nesfortest.cpp
mapper.o: ${SRCDIR}mapper.cpp ${SRCDIR}mapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}mapper.cpp
vmapper.o: ${SRCDIR}vmapper.cpp ${SRCDIR}vmapper.h ${SRCDIR}mapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}vmapper.cpp
cpu.o: ${SRCDIR}cpu.cpp ${SRCDIR}cpu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}cpu.cpp
ppu.o: ${SRCDIR}ppu.cpp ${SRCDIR}ppu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}ppu.cpp
apu.o: ${SRCDIR}apu.cpp ${SRCDIR}apu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}apu.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
