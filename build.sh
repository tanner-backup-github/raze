
# @TODO: 	check if debug build and do valgrind if so
clear
gcc -DDEBUG main.c -o main.o -O2 -Wall -Wextra -std=gnu99 -lGL -lGLU -lSDL2_ttf `sdl2-config --cflags --libs` `pkg-config --static --cflags --libs x11 xrandr xi xxf86vm glew glfw3 freetype2` `curl-config --cflags --libs` -lm -lpthread -g
valgrind ./main.o
