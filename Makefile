# Makefile for Hello World project

#test: mac_test.o 
#	gcc -o test mac_test.o

#mac_test.o: mac_test.c
#	gcc -c mac_test.c

#clean:
#	rm -f *.o hello
	
socket_test: _ifconfig.o 
	gcc -o socket_test _ifconfig.o

_ifconfig.o: _ifconfig.c
	gcc -c _ifconfig.c

clean:
	rm -f *.o socket_test
