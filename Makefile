SRCS= debug_main.cpp DebugDraw.cpp  DebugDrawShaders.cpp Logger.cpp  Transform.cpp Buffers.cpp
OBJS= $(SRCS:.cpp=.o)

debug_main: $(OBJS)
	g++ -o $@ $^ -L lib -Wl,-rpath,lib -lGL -lglut -lGLEW

%.o: %.cpp
	g++ -Wall -MMD -MP -I include/ -c $<

%.o: src/%.cpp
	g++ -Wall -MMD -MP -I include/ -c $<

clean:
	rm -f debug_main
	rm -f *.o src/*.o *.d src/*.d

-include $(OBJS:.o=.d)
