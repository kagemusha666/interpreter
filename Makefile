# Makefile of interpreter project

PROGRAM=lisp

ARCH=x86_64
CROSS_COMPILE=x86_64-linux-gnu
CFLAGS=-Wall -std=c99 -g -rdynamic

CC=$(CROSS_COMPILE)-gcc
STRIP=$(CROSS_COMPILE)-strip
INSTALL=install

OUTPUTDIR=$(abspath .)/build/$(ARCH)
SOURCESDIR=$(abspath .)/src
TESTSDIR=$(abspath .)/tests
DESTDIR=/
prefix=/usr/local

SOURCES:=$(wildcard $(SOURCESDIR)/*.c)
HEADERS:=$(wildcard $(SOURCESDIR)/*.h)
OBJECTS:=$(addprefix $(OUTPUTDIR)/,$(notdir $(SOURCES:.c=.o)))
TESTS:=$(wildcard $(TESTSDIR)/*.sch)

.PHONY: all install clean

all: $(OUTPUTDIR)/$(PROGRAM)

check: $(OUTPUTDIR)/$(PROGRAM)
	@for test in $(TESTS); do $< $$test; done && echo "ok" || echo "fail"

install: $(OUTPUTDIR)/$(PROGRAM)
	$(INSTALL) --strip --strip-program=$(STRIP) $@ $(DESTDIR)/$(prefix)/$(PROGRAM)

$(OUTPUTDIR)/$(PROGRAM): $(OBJECTS)
	$(CC) -o $@ $+ $(CFLAGS)

$(OUTPUTDIR)/%o:  $(SOURCESDIR)/%c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJECTS): $(HEADERS) | $(OUTPUTDIR)

$(OUTPUTDIR):
	@mkdir -p $@

clean:
	rm -rf build
