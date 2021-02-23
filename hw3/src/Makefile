CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -pthread -lboost_system
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))

#all: echo_server.cpp
	#$(CXX) echo_server.cpp -o echo_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
part1: echo_server.o env.o exe.o cgi.o win.o
	$(CXX) echo_server.o  env.o exe.o -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) cgi.o -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
part2:
	$(CXX) main.cpp env2.cpp -o cgi_server.exe  -lws2_32 -lwsock32 $(CXXFLAGS)

echo_server.o: echo_server.cpp
		$(CXX) -c echo_server.cpp  -o echo_server.o $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
env.o: env.cpp
		$(CXX) -c env.cpp  -o env.o $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
env2.o: env2.cpp
		$(CXX) -c env2.cpp  -o env2.o -lws2_32 -lwsock32 $(CXXFLAGS)

exe.o: exe.cpp
		$(CXX) -c exe.cpp  -o exe.o $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
cgi.o: cgi.cpp
		$(CXX) -c cgi.cpp  -o cgi.o $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
win.o: main.cpp
		$(CXX) -c main.cpp  -o win.o -lws2_32 -lwsock32 $(CXXFLAGS)
clean:
	rm -f http_server
	rm -f console.cgi
	rm -f cgi_server.exe
	rm -f *.o
