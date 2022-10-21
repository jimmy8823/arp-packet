CC:=gcc

arp: arp.o main.o
	$(CC) -o arp arp.o main.o
main.o:
	$(CC) -c arp.c main.c
arp.o:
	$(CC) -c arp.c 

clean:
	rm -f arp.o main.o 