cc=g++
cflags= -Wall -std=c++11
OpenGL= -lGL -lGLEW -lglut

GraphPunk: main.cpp
	$(cc) -o $@ $< $(cflags) $(OpenGL)

clean:
	rm GraphPunk
