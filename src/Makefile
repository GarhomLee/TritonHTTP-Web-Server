
CXX=g++
CXXFLAGS=-std=c++11 -ggdb -Wall -Wextra -pedantic -Wnon-virtual-dtor -I../dependencies/include
SERVEROBJS= server-main.o logger.o ResponseBuilder.o RequestHandler.o MessageParser.o HttpdServer.o

default: httpd

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

httpd: $(SERVEROBJS)
	$(CXX) $(CXXFLAGS) -o httpd $(SERVEROBJS) -pthread

.c.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f httpd *.o
