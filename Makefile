CC:=cc
CFLAGS:=-g
PREFIX?=/usr/local

.PHONY: all, deps, build, install, clean, clean-deps

all: build

build: deps
	$(CC) -o redis-del redis-del.c hiredis/libhiredis.a

deps:
	git submodule update --init --recursive
	$(MAKE) -C ./hiredis

install:
	install -m 0755 redis-del $(prefix)/bin

clean: clean-deps
	rm -f ./redis-del

clean-deps:
	cd ./hiredis && $(MAKE) clean

