# Makefile for sources

HEADERS=$(wildcard *.h)
SOURCES=$(wildcard *.c)

OBJECTS=$(addprefix $(OUTPUTDIR)/,$(SOURCES:.c=.o))
OUTPUT=$(OUTPUTDIR)/$(PROGRAM)

SOURCESDIR=$(abspath .)

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	$(CC) -o $@ $+

$(OBJECTS): $(HEADERS)

$(OUTPUTDIR)/%.o: $(SOURCESDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(OBJECTS)
	rm -f $(OUTPUT)
