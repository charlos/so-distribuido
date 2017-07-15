all: file-system

file-system: file-system.c
	gcc -Wall -I../../shared-library -L../../shared-library/shared-library file-system.c -lshared-library -lcommons -lpthread -o file-system

clean: 
	rm file-system
