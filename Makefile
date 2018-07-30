.PHONY = build run

build:
	gcc main.c -o main -O2 -Wall -Wextra -std=gnu99 -lGL -lGLU \
			`pkg-config --static --cflags --libs x11 xrandr xxf86vm glew glfw3 freetype2` \
			`curl-config --cflags --libs` -lm -lpthread -g

run: build
	./main
