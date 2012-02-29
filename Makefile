include ${EZSDK}/Rules.make
libpump.a:pump.o
	ar cr libpump.a pump.o

pump.o:./DeviceInterface/pump.c ./DeviceInterface/DeviceInterface.h ./DeviceInterface/internal.h
	$(CSTOOL_PREFIX)gcc -g -c ./DeviceInterface/pump.c -Wall

testliba:testlib.o libpump.a
	$(CSTOOL_PREFIX)gcc -g -o testliba testlib.o -L. -lpump -pthread

testlib.o:./testlib/testlib.c ./DeviceInterface/list.h
	$(CSTOOL_PREFIX)gcc -g -c ./testlib/testlib.c -Wall

omx_test:serv.o test.o
	$(CSTOOL_PREFIX)gcc -g -o omx_test serv.o test.o -pthread

serv.o:./omx_emu/serv.c ./omx_emu/serv.h ./DeviceInterface/internal.h
	$(CSTOOL_PREFIX)gcc -g -c ./omx_emu/serv.c -Wall

test.o:./omx_emu/test.c ./omx_emu/serv.h ./DeviceInterface/internal.h
	$(CSTOOL_PREFIX)gcc -g -c ./omx_emu/test.c -Wall


