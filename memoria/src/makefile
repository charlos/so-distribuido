all: memoria

memoria: memoria.c
	gcc -Wall -I../../shared-library -L../../shared-library/shared-library memoria.c -lshared-library -lcommons -lpthread -o memoria

clean: 
	rm memoria
