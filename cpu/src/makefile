CC := gcc
CFLAGS := -Wall
ISL := -I../../shared-library
LSL := -L../../shared-library/shared-library

OBJS := cpu.o funcionesParser.o funcionesCPU.o
HDRS := funcionesParser.h funcionesCPU.h


all: cpu

cpu: $(OBJS)
	$(CC) $(ISL) $(LSL) $(CFLAGS) -o cpu $(OBJS) -lshared-library -lparser-ansisop -lcommons

cpu.o: cpu.c $(HDRS)
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) cpu.c -lshared-library -lparser-ansisop -lcommons

funcionesParser.o: funcionesParser.h
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) funcionesParser.c -lshared-library -lparser-ansisop -lcommons -o funcionesParser.o

funcionesCPU.o: funcionesCPU.h
	$(CC) $(ISL) $(LSL) -c $(CFLAGS) funcionesCPU.c -lshared-library -lcommons -o funcionesCPU.o

clean:
	rm cpu $(OBJS)
