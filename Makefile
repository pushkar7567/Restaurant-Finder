CC=g++
OBJS=exercise4 exercise4.o
CFLAGS=-c -Wall -O2
PROGRAM=prog

PROGRAM: $(OBJS)

exercise4: exercise4.o
	$(CC) exercise4.o -o exercise4 

exercise4.o: exercise4.cpp
	$(CC) exercise4.cpp -o exercise4.o $(CFLAGS)

clean:
	@rm -f $(OBJS)
	@rm -f $(PROGRAM)