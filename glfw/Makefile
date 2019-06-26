CXX=clang++
CXXFLAGS=-std=c++11 -c -I../src -Wall -O -DGL_SILENCE_DEPRECATION
LD=clang++
LDFLAGS=
LIBS=-lglfw -lGLEW
FWS=-framework OpenGL -framework CoreVideo -framework IOKit -framework Cocoa

TARGET=nes
OBJS=main.o nes.o mapper.o vmapper.o cpu.o ppu.o vppu.o apu.o pad.o vpad.o events.o renderer.o glfw.o glfwrenderer.o
SRCDIR=../src

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS} ${FWS}

main.o: main.cpp
	${CXX} ${CXXFLAGS} main.cpp
nes.o: ${SRCDIR}/nes.cpp ${SRCDIR}/nes.h ${SRCDIR}/cpu.h ${SRCDIR}/ppu.h ${SRCDIR}/vppu.h ${SRCDIR}/mapper.h ${SRCDIR}/vmapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/nes.cpp
mapper.o: ${SRCDIR}/mapper.cpp ${SRCDIR}/mapper.h ${SRCDIR}/ppu.h ${SRCDIR}/cpu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/mapper.cpp
vmapper.o: ${SRCDIR}/vmapper.cpp ${SRCDIR}/vmapper.h ${SRCDIR}/mapper.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/vmapper.cpp
cpu.o: ${SRCDIR}/cpu.cpp ${SRCDIR}/cpu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/cpu.cpp
ppu.o: ${SRCDIR}/ppu.cpp ${SRCDIR}/ppu.h ${SRCDIR}/events.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/ppu.cpp
vppu.o: ${SRCDIR}/vppu.cpp ${SRCDIR}/vppu.h ${SRCDIR}/ppu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/vppu.cpp
apu.o: ${SRCDIR}/apu.cpp ${SRCDIR}/apu.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/apu.cpp
pad.o: ${SRCDIR}/pad.cpp ${SRCDIR}/pad.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/pad.cpp
vpad.o: ${SRCDIR}/vpad.cpp ${SRCDIR}/vpad.h ${SRCDIR}/pad.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/vpad.cpp
events.o: ${SRCDIR}/events.cpp ${SRCDIR}/events.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/events.cpp
renderer.o: ${SRCDIR}/renderer.cpp ${SRCDIR}/renderer.h
	${CXX} ${CXXFLAGS} ${SRCDIR}/renderer.cpp
glfw.o: glfw.cpp glfw.h
	${CXX} ${CXXFLAGS} glfw.cpp
glfwrenderer.o: glfwrenderer.cpp glfwrenderer.h ${SRCDIR}/renderer.h glfw.h
	${CXX} ${CXXFLAGS} glfwrenderer.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
