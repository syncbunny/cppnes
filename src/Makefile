CXX=clang++
CXXFLAGS=-std=c++11 -c -I../src -Wall -O -DGL_SILENCE_DEPRECATION
LD=clang++
LDFLAGS=
LIBS=-lglfw -lGLEW
FWS=-framework OpenGL -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenAL

TARGET=nes
OBJS=main.o config.o logger.o nes.o mapper.o vmapper.o cpu.o ppu.o vppu.o apu.o openalapu.o pad.o vpad.o events.o renderer.o core.o glfw.o glfwrenderer.o profiler.o

${TARGET}:${OBJS}
	${LD} -o ${TARGET} ${OBJS} ${LDFLAGS} ${LIBS} ${FWS}

main.o:main.cpp config.h core.h cpu.h nes.h ppu.h apu.h mapper.h
	${CXX} ${CXXFLAGS} main.cpp
config.o:config.cpp config.h logger.h
	${CXX} ${CXXFLAGS} config.cpp
logger.o:logger.cpp logger.h config.h
	${CXX} ${CXXFLAGS} logger.cpp
nes.o: nes.cpp nes.h config.h cpu.h ppu.h apu.h openalapu.h vppu.h mapper.h vmapper.h core.h frameworker.h profiler.h
	${CXX} ${CXXFLAGS} nes.cpp
mapper.o: mapper.cpp mapper.h ppu.h cpu.h
	${CXX} ${CXXFLAGS} mapper.cpp
vmapper.o: vmapper.cpp vmapper.h mapper.h
	${CXX} ${CXXFLAGS} vmapper.cpp
cpu.o: cpu.cpp cpu.h config.h core.h
	${CXX} ${CXXFLAGS} cpu.cpp
ppu.o: ppu.cpp ppu.h events.h renderer.h core.h
	${CXX} ${CXXFLAGS} ppu.cpp
vppu.o: vppu.cpp vppu.h ppu.h
	${CXX} ${CXXFLAGS} vppu.cpp
apu.o: apu.cpp apu.h
	${CXX} ${CXXFLAGS} apu.cpp
openalapu.o: openalapu.cpp openalapu.h apu.h vapu.h frameworker.h
	${CXX} ${CXXFLAGS} openalapu.cpp
pad.o: pad.cpp pad.h
	${CXX} ${CXXFLAGS} pad.cpp
vpad.o: vpad.cpp vpad.h pad.h
	${CXX} ${CXXFLAGS} vpad.cpp
events.o: events.cpp events.h
	${CXX} ${CXXFLAGS} events.cpp
renderer.o: renderer.cpp renderer.h
	${CXX} ${CXXFLAGS} renderer.cpp
core.o: core.cpp core.h
	${CXX} ${CXXFLAGS} core.cpp
glfw.o: glfw.cpp glfw.h
	${CXX} ${CXXFLAGS} glfw.cpp
glfwrenderer.o: glfwrenderer.cpp glfwrenderer.h renderer.h glfw.h
	${CXX} ${CXXFLAGS} glfwrenderer.cpp
profiler.o: profiler.cpp profiler.h frameworker.h
	${CXX} ${CXXFLAGS} profiler.cpp

clean:
	-rm -f ${TARGET} ${OBJS}
