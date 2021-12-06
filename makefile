SHELL:=/usr/bin/bash
CC=nvc
CFLAGS= -O3 -acc -gpu=cc61 -Minfo=accel #-Minfo=accel -mp -ta=multicore -gpu=cc61 -Minfo=accel -Minfo=mp
DEPS = partticle.h BoundBox.h physics.h draw2D.h bitmap.h
OBJ = main.o particle.o BoundBox.o physics.o draw2D.o bitmap.o
LIBS = -lm 

profile: main
	rm -f main_nsys.*; \
	nsys profile -o main_nsys --trace openacc,cuda,openmp ./main; \
	/home/quin/nsight-systems-2021.5.1/bin/nsys-ui ./main_nsys.qdrep; \


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS); \

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS); \

.PHONY: clean

clean:
	rm -rf *.o
	rm -f main
	rm -rf Output/*
	rm -f main_nsys.*
