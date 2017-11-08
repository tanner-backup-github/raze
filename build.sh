
# @TODO: 	check if debug build and do valgrind if so
clear
gcc -D"$1" main.c -o main.o -O2 -Wall -Wextra -std=gnu99 -lGL -lGLU \
    `pkg-config --static --cflags --libs x11 xrandr xi xxf86vm glew glfw3 freetype2` `curl-config --cflags --libs` \
    -lm -lpthread -g
if [ "$1" == "DEBUG" ]; then
    valgrind ./main.o
else
    ./main.o
fi
