CFLAGS= -g -Wall -MMD -MP -DRETRACE -DHAVE_X11 -DGL_GLEXT_PROTOTYPES -DGLX_GLXEXT_PROTOTYPES -I include/khronos -I . -I include

LIBDIR= $(PWD)/lib

SRCS= shared_main.cpp Transform.cpp Buffers.cpp DebugDraw.cpp DebugDrawShaders.cpp Logger.cpp glws.cpp glws_glx.cpp glproc_gl.cpp os_posix.cpp
OBJS= $(SRCS:.cpp=.o)

shared_main: $(OBJS)
	@echo $(LIBDIR)
	g++ -g -o $@ $^ -L lib -Wl,-rpath,$(LIBDIR) -lGL -lGLEW

%.o: %.cpp
	g++ $(CFLAGS) -c $<

%.o: src/%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f shared_main
	rm -f *.o src/*.o *.d src/*.d

-include $(OBJS:.o=.d)
