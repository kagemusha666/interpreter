# Makefile for tests

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

OBJECTS=$(addprefix $(OUTPUTDIR)/,$(SOURCES:.c=.o))
OUTPUT=$(OUTPUTDIR)/test

SOURCESDIR=$(abspath .)

CFLAGS+=-I../src

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $@ $+

$(OBJECTS): $(HEADERS)

$(OUTPUTDIR)/%.o: $(SOURCESDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(OBJECTS)
	rm -f $(OUTPUT)
