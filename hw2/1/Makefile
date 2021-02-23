CC = g++
CFLAGS = -g

main:main.o shell.o cmd.o env.o number_pipe.o
	$(CC) main.o shell.o cmd.o env.o number_pipe.o -o np_simple
main.o:main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o
shell.o:shell.cpp
	$(CC) $(CFLAGS) -c shell.cpp -o shell.o
cmd.o:cmd.cpp
	$(CC) $(CFLAGS) -c cmd.cpp -o cmd.o
env.o:env.cpp
	$(CC) $(CFLAGS) -c env.cpp -o env.o
number_pipe.o:number_pipe.cpp
	$(CC) $(CFLAGS) -c number_pipe.cpp -o number_pipe.o

clean:
	rm -rf *.o
