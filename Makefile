CC:=cc
CFLAGS:=-g
PREFIX?=/usr/local

.PHONY: all, deps, build, install, clean, clean-deps

all: deps, build

deps:
	$(MAKE) -C ./hiredis

build:
	$(CC) -o redis-del redis-del.c hiredis/libhiredis.a

install:
	install -m 0755 redis-del $(prefix)/bin

clean: clean-deps
	rm -f ./redis-del

clean-deps:
	cd ./hiredis && $(MAKE) clean

