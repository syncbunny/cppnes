CXX=clang++
CXXFLAGS=-c -Wall -std=c++11 -O
LD=clang++
LDFLAGS=
LIBS=

TARGET=nes
OBJS=main.o nes.o mapper.o vmapper.o cpu.o ppu.o vppu.o apu.o pad.o vpad.o

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS}

main.o:main.cpp cpu.h nes.h ppu.h apu.h mapper.h 
	${CXX} ${CXXFLAGS} main.cpp
nes.o: nes.cpp nes.h cpu.h ppu.h vppu.h mapper.h vmapper.h
	${CXX} ${CXXFLAGS} nes.cpp
mapper.o: mapper.cpp mapper.h
	${CXX} ${CXXFLAGS} mapper.cpp
vmapper.o: vmapper.cpp vmapper.h mapper.h
	${CXX} ${CXXFLAGS} vmapper.cpp
cpu.o: cpu.cpp cpu.h
	${CXX} ${CXXFLAGS} cpu.cpp
ppu.o: ppu.cpp ppu.h
	${CXX} ${CXXFLAGS} ppu.cpp
vppu.o: vppu.cpp vppu.h ppu.h
	${CXX} ${CXXFLAGS} vppu.cpp
apu.o: apu.cpp apu.h
	${CXX} ${CXXFLAGS} apu.cpp
pad.o: pad.cpp pad.h
	${CXX} ${CXXFLAGS} pad.cpp
vpad.o: vpad.cpp vpad.h pad.h
	${CXX} ${CXXFLAGS} vpad.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
