CC:=cc
CFLAGS:=-Wall -Wextra -Werror -O2 -std=c11 -g
PREFIX?=/usr/local

.PHONY: all deps build install clean clean-deps test

all: build test

build: deps
	$(CC) $(CFLAGS) -o redis-del redis-del.c hiredis/libhiredis.a -I./hiredis

deps:
	git submodule update --init --recursive
	$(MAKE) -C ./hiredis

install:
	install -m 0755 redis-del $(PREFIX)/bin

clean: clean-deps
	rm -f ./redis-del ./redis-del-test

clean-deps:
	cd ./hiredis && $(MAKE) clean

test: build
	$(CC) $(CFLAGS) -o redis-del-test test.c hiredis/libhiredis.a -I./hiredis
	@echo "Running test..."
	./redis-del-test
