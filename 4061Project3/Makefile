all: main_mm main_malloc

main_mm: mm.o main_mm.c
	gcc -o main_mm main_mm.c mm.o

main_malloc: mm.o main_malloc.c
	gcc -o main_malloc main_malloc.c mm.o

mm.o: mm.c
	gcc -o mm.o -c mm.c

clean:
	rm main_mm mm.o
