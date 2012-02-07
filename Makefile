libpump.a:pump.o
	ar cr libpump.a pump.o

pump.o:./DeviceInterface/pump.c ./DeviceInterface/DeviceInterface.h ./DeviceInterface/internal.h
	gcc -g -c ./DeviceInterface/pump.c -Wall

testlib:testlib.o libpump.a
	gcc -g -o testlib testlib.o -L. -lpump -pthread
testlib.o:testlib.c
	gcc -g -c testlib.c -Wall
