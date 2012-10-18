CFLAGS= -pipe -g -W -Wall -Wno-unused-parameter -MMD -MP -DRETRACE -DHAVE_X11 -DGL_GLEXT_PROTOTYPES -DGLX_GLXEXT_PROTOTYPES -I include/khronos -I . -I include

LIBDIR= $(PWD)/lib

SRCS= shared_main.cpp Transform.cpp Programs.cpp Buffers.cpp DebugDraw.cpp DebugDrawShaders.cpp Logger.cpp glws.cpp glws_glx.cpp glproc_gl.cpp os_posix.cpp
OBJS= $(SRCS:.cpp=.o)

shared_main: $(OBJS)
	g++ -g -o $@ $^ -L lib -Wl,-rpath,$(LIBDIR) -lGL -lX11 -ldl

%.o: %.cpp
	g++ $(CFLAGS) -c $<

%.o: src/%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f shared_main
	rm -f *.o src/*.o *.d src/*.d

-include $(OBJS:.o=.d)
