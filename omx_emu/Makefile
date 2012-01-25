videoshmat:videoshmat.o
	gcc -Wall -g -pthread videoshmat.o -o videoshmat 


videoshmat.o:videoshmat.c videoshmat.h
	gcc -Wall -g -c videoshmat.c

client:client.o
	gcc -Wall -g -pthread client.o -o client 


videoshmat.o:client.c videoshmat.h
	gcc -Wall -g -c client.c
