# Makefile of interpreter project

PROGRAM=lisp

ARCH=x86_64
CROSS_COMPILE=x86_64-linux-gnu

CC=$(CROSS_COMPILE)-gcc
STRIP=$(CROSS_COMPILE)-strip
INSTALL=install

OUTPUTDIR=$(abspath .)/build/$(ARCH)
DESTDIR=/
prefix=/usr/local

CFLAGS=-std=c99

export

.PHONY: outputdir sources tests check install clean

all: sources

install:
	@$(INSTALL) --strip --strip-program=$(STRIP) $(OUTPUTDIR)/$(PROGRAM) $(DESTDIR)/$(prefix)/$(PROGRAM)

check: tests
	@$(OUTPUTDIR)/test

tests: sources
	@$(MAKE) -C tests

sources: outputdir
	@$(MAKE) -C src

outputdir:
	@mkdir -p $(OUTPUTDIR)

clean:
	rm -rf build
