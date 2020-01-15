# CC=clang-9
CC=gcc
CFLAGS=-I./src/ -I. -O1 -Wall -Werror -pedantic -std=c99 -g
OBJ=build/system/application.o build/system/event.o build/system/pools.o build/system/event-loop.o build/system/signal.o build/system/scheduler.o build/utils/circular-queue.o build/utils/closure.o build/utils/linked-list.o build/utils/object-pool.o
TEST_OBJ=build/test/utils/circular-queue.o build/test/utils/closure.o build/test/utils/linked-list.o build/test/utils/object-pool.o build/test/system/event.o build/test/system/pools.o build/test/system/event-loop.o build/test/system/scheduler.o build/test/system/signal.o build/test/system/application.o

dist/libuevloop.so: $(OBJ)
	mkdir -p dist
	$(CC) -shared -fpic -o dist/libuevloop.so $(OBJ) $(CFLAGS) -fprofile-arcs -ftest-coverage

build/system/%.o: src/system/%.c src/system/%.h
	mkdir -p build/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS) -fprofile-arcs -ftest-coverage

build/utils/%.o: src/utils/%.c src/utils/%.h
	mkdir -p build/utils
	$(CC) -c -fpic  -o $@ $< $(CFLAGS) -fprofile-arcs -ftest-coverage

dist/test: dist/libuevloop.so build/test.o $(TEST_OBJ)
	$(CC) -L./dist -o dist/test build/test.o $(TEST_OBJ) -luevloop $(CFLAGS)

build/test.o: test/test.c test/minunit.h
	$(CC) -c -fpic -o build/test.o test/test.c $(CFLAGS)

build/test/system/%.o: test/system/%.c test/system/%.h build/system/%.o test/minunit.h
	mkdir -p build/test/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

build/test/utils/%.o: test/utils/%.c test/utils/%.h build/utils/%.o test/minunit.h
	mkdir -p build/test/utils
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

.PHONY: clean test coverage doc debug

clean:
	rm -rf build dist coverage docs

test: dist/test
	$(MAKE) && LD_LIBRARY_PATH=$(shell pwd)/dist:$(LD_LIBRARY_PATH) LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so ./dist/test

coverage: dist/test
	mkdir -p coverage
	LD_LIBRARY_PATH=$(shell pwd)/dist:$(LD_LIBRARY_PATH) ./dist/test
	gcov test/test.c
	lcov -c --directory . --output-file coverage/test.info
	genhtml coverage/test.info --output-directory coverage

docs: Doxyfile $(OBJ)
	doxygen Doxyfile

debug:
	$(MAKE) && LD_LIBRARY_PATH=$(shell pwd)/dist:$(LD_LIBRARY_PATH) gdb dist/test
