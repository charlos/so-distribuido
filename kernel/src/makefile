CC := gcc
CFLAGS := -Wall
ISL := -I../../shared-library
LSL := -L../../shared-library/shared-library
LIBS := -lcommons -lparser-ansisop -lpthread -lshared-library

OBJS := kernel.o kernel_generales.o solicitudes.o consola.o
HDRS := kernel_generales.h solicitudes.h

all: kernel

kernel: $(OBJS)
	$(CC) $(ISL) $(LSL) $(CFLAGS) -o kernel $(OBJS) $(LIBS)
	
kernel.o: kernel.c $(HDRS)
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) kernel.c $(LIBS)
	
kernel_generales.o: kernel_generales.h
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) kernel_generales.c $(LIBS) -o kernel_generales.o
	
solicitudes.o: solicitudes.h
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) solicitudes.c $(LIBS) -o solicitudes.o
	
consola.o: consola.c
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) consola.c $(LIBS) -o consola.o

clean:
	rm kernel $(OBJS)