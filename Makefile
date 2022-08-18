CC = gcc
FLAGS = -g -O3 -lpthread -lglut -lGLU -lGL -lm -ffast-math -lX11 -lXext -lXt -lSM -lICE
RM = rm -f
prefix = /usr/local

INCS = fileio.c display.c  make_surface.c

SRCS = main 

OBJS = $(SRCS:.c=.o)


main: main.c 
	$(CC) main.c $(INCS) $(FLAGS) -o spod


clean:
	$(RM) spod
	
