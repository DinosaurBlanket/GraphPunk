cc=g++
cflags= -Wall -std=c++11
OpenGL= -lGL -lGLEW -lglut

GraphPunk: *.cpp *.hpp
	$(cc) -c GL_util.cpp $(cflags) $(OpenGL)
	$(cc) -c main.cpp $(cflags) $(OpenGL)
	$(cc) main.o GL_util.o -o $@ $(cflags) $(OpenGL)

clean:
	rm GraphPunk
