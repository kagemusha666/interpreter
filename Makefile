# Makefile of interpreter project


CC = gcc
CFLAGS = -I.src/
DEPS = src/vm.h src/types.h src/debug.h src/errors.h
OBJ = src/main.o src/vm.o src/types.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheme: $(OBJ)
	mkdir -p build
	$(CC) -o build/$@ $^ $(CFLAGS)

clean:
	rm -rf build
