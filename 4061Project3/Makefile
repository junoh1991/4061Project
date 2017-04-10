all: main_mm main_malloc packet_sender_receiver

packet_sender_receiver: packet_sender.c packet_receiver.c mm.o
	gcc -o packet_sender packet_sender.c mm.o 		
	gcc -o packet_receiver packet_receiver.c mm.o

main_mm: mm.o main_mm.c
	gcc -o main_mm main_mm.c mm.o

main_malloc: mm.o main_malloc.c
	gcc -o main_malloc main_malloc.c mm.o

mm.o: mm.c
	gcc -o mm.o -c mm.c

clean:
	rm -Rf main_mm main_malloc mm.o packet_sender packet_receiver
