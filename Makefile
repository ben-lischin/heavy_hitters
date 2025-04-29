all: test

CC = g++
OPT= -g -flto -Ofast
CFLAGS = $(OPT) -Wall
LIBS = -lssl -lcrypto 

test: test.cpp zipf.c hashutil.c sketching/count_sketch.cpp sketching/count_min_sketch.cpp sketching/misra_gries.cpp
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f test test.o
