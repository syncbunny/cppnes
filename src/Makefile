CXX=clang++
CXXFLAGS=-c -Wall -std=c++11 -O
LD=clang++
LDFLAGS=
LIBS=

TARGET=nes
OBJS=main.o nes.o mapper.o vmapper.o cpu.o ppu.o apu.o

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS}

main.o:main.cpp cpu.h nes.h ppu.h mapper.h
	${CXX} ${CXXFLAGS} main.cpp
nes.o: nes.cpp nes.h cpu.h ppu.h mapper.h vmapper.h
	${CXX} ${CXXFLAGS} nes.cpp
mapper.o: mapper.cpp mapper.h
	${CXX} ${CXXFLAGS} mapper.cpp
vmapper.o: vmapper.cpp vmapper.h mapper.h
	${CXX} ${CXXFLAGS} vmapper.cpp
cpu.o: cpu.cpp cpu.h
	${CXX} ${CXXFLAGS} cpu.cpp
ppu.o: ppu.cpp ppu.h
	${CXX} ${CXXFLAGS} ppu.cpp
apu.o: apu.cpp apu.h
	${CXX} ${CXXFLAGS} apu.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
