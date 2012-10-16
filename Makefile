CFLAGS= -g -Wall -MMD -MP -I . -I include

LIBDIR= $(PWD)/lib

SRCS= debug_main.cpp Transform.cpp Buffers.cpp DebugDraw.cpp DebugDrawShaders.cpp Logger.cpp
OBJS= $(SRCS:.cpp=.o)

debug_main: $(OBJS)
	@echo $(LIBDIR)
	g++ -g -o $@ $^ -L lib -Wl,-rpath,$(LIBDIR) -lGL -lglut -lGLEW

%.o: %.cpp
	g++ $(CFLAGS) -c $<

%.o: src/%.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f debug_main
	rm -f *.o src/*.o *.d src/*.d

-include $(OBJS:.o=.d)
