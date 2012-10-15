SRCS= debug_main.cpp Transform.cpp Buffers.cpp DebugDraw.cpp DebugDrawShaders.cpp Logger.cpp
OBJS= $(SRCS:.cpp=.o)

debug_main: $(OBJS)
	g++ -g -o $@ $^ -L lib -Wl,-rpath,lib -lGL -lglut -lGLEW

%.o: %.cpp
	g++ -g -Wall -MMD -MP -I . -I include/ -c $<

%.o: src/%.cpp
	g++ -g -Wall -MMD -MP -I . -I include/ -c $<

clean:
	rm -f debug_main
	rm -f *.o src/*.o *.d src/*.d

-include $(OBJS:.o=.d)
