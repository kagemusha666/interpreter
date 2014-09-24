# Makefile of interpreter project


CC = gcc
CFLAGS = -I.src/ -std=c99
DEPS = src/vm.h src/vm_priv.h src/types.h src/error.h src/debug.h
OBJ = src/main.o src/vm.o src/vm_priv.o src/types.o src/error.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

scheme: $(OBJ)
	mkdir -p build
	$(CC) -o build/$@ $^ $(CFLAGS)

clean:
	rm -rf build
	rm -f src/*.o
