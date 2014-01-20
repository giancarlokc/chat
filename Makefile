CFLAG=-Wall
	PROG = chat
	OBJS = chat.c
	CC = gcc
	LIBS = -lpthread

all: $(OBJS)
	gcc -o $(PROG) $(OBJS) $(LIBS)

clean:
	@rm -r $(PROG)

