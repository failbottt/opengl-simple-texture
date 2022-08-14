CC=gcc 
CFLAGS=-std=c99 -Wall -Wextra -g
LINK=-L./external/glfw/build/src
INCLUDE=-I./external/glfw/include

main: main.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LINK) -o exe main.c -lGL -lglfw3 -ldl -lm -lpthread
