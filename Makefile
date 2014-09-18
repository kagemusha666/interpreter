# Makefile of interpreter project


CC = gcc
CFLAGS = -I.src/
DEPS = src/core.h src/types.h
OBJ = src/main.o src/core.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheme: $(OBJ)
	mkdir -p build
	$(CC) -o build/$@ $^ $(CFLAGS)

clean:
	rm -rf build
