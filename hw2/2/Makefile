CC = g++
CFLAGS = -g

main:main.o shell.o cmd.o env.o number_pipe.o client.o user_pipe.o
	$(CC) main.o shell.o cmd.o env.o number_pipe.o client.o user_pipe.o -o np_simple
main.o:test.cpp
	$(CC) $(CFLAGS) -c test.cpp -o main.o
shell.o:shell.cpp
	$(CC) $(CFLAGS) -c shell.cpp -o shell.o
cmd.o:cmd.cpp
	$(CC) $(CFLAGS) -c cmd.cpp -o cmd.o
env.o:env.cpp
	$(CC) $(CFLAGS) -c env.cpp -o env.o
number_pipe.o:number_pipe.cpp
	$(CC) $(CFLAGS) -c number_pipe.cpp -o number_pipe.o

user_pipe.o:user_pipe.cpp
	$(CC) $(CFLAGS) -c user_pipe.cpp -o user_pipe.o
client.o:client.cpp
	$(CC) $(CFLAGS) -c client.cpp -o client.o

clean:
	rm -rf *.o
