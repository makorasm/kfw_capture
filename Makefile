libpump.a:pump.o sync_sem.o
	ar cr libpump.a pump.o 

pump.o:./DeviceInterface/pump.c ./DeviceInterface/DeviceInterface.h ./DeviceInterface/internal.h ./DeviceInterface/sync_sem.h
	gcc -g -c ./DeviceInterface/pump.c -Wall


testliba:testlib.o libpump.a sync_sem.o
	gcc -g -o testliba testlib.o sync_sem.o -L. -lpump -pthread

testlib.o:./testlib/testlib.c ./DeviceInterface/list.h
	gcc -g -c ./testlib/testlib.c -Wall

omx_test:serv.o test.o sync_sem.o
	gcc -g -o omx_test serv.o test.o sync_sem.o -pthread

serv.o:./omx_emu/serv.c ./omx_emu/serv.h ./DeviceInterface/internal.h
	gcc -g -c ./omx_emu/serv.c -Wall

test.o:./omx_emu/test.c ./omx_emu/serv.h ./DeviceInterface/internal.h
	gcc -g -c ./omx_emu/test.c -Wall

sync_sem.o: ./DeviceInterface/sync_sem.c ./DeviceInterface/sync_sem.h
	gcc -g -c ./DeviceInterface/sync_sem.c -Wall
