# CC=clang-9
CC=gcc
CFLAGS=-I./src/ -I. -Og -Wall -Werror -pedantic -std=c99 -g

OBJ=build/system/event.o build/system/event-loop.o build/system/signal.o build/system/scheduler.o build/system/containers/application.o build/system/containers/system-queues.o build/system/containers/system-pools.o build/utils/circular-queue.o build/utils/closure.o build/utils/linked-list.o build/utils/object-pool.o build/utils/iterator.o build/utils/pipeline.o build/utils/conditional.o build/utils/functional.o

TEST_OBJ=build/test/utils/circular-queue.o build/test/utils/closure.o build/test/utils/linked-list.o build/test/utils/object-pool.o build/test/system/event.o build/test/system/containers/system-pools.o build/test/system/containers/application.o build/test/system/containers/system-queues.o build/test/system/event-loop.o build/test/system/scheduler.o build/test/system/signal.o build/test/utils/conditional.o build/test/utils/pipeline.o build/test/utils/iterator.o build/test/utils/functional.o

dist/libuevloop.so: $(OBJ)
	mkdir -p dist
	$(CC) -shared -fpic -o dist/libuevloop.so $(OBJ) $(CFLAGS) -fprofile-arcs -ftest-coverage

build/system/%.o: src/system/%.c src/system/%.h
	mkdir -p build/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS) -fprofile-arcs -ftest-coverage

build/system/containers/%.o: src/system/containers/%.c src/system/containers/%.h
	mkdir -p build/system/containers
	$(CC) -c -fpic -o $@ $< $(CFLAGS) -fprofile-arcs -ftest-coverage

build/utils/%.o: src/utils/%.c src/utils/%.h
	mkdir -p build/utils
	$(CC) -c -fpic  -o $@ $< $(CFLAGS) -fprofile-arcs -ftest-coverage

dist/test: dist/libuevloop.so build/test.o $(TEST_OBJ)
	$(CC) -L./dist -o dist/test build/test.o $(TEST_OBJ) -luevloop -lm $(CFLAGS)

build/test.o: test/test.c test/uelt.h
	$(CC) -c -fpic -o build/test.o test/test.c $(CFLAGS)

build/test/system/%.o: test/system/%.c test/system/%.h build/system/%.o test/uelt.h
	mkdir -p build/test/system
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

build/test/system/containers/%.o: test/system/containers/%.c test/system/containers/%.h build/system/containers/%.o test/uelt.h
	mkdir -p build/test/system/containers
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

build/test/utils/%.o: test/utils/%.c test/utils/%.h build/utils/%.o test/uelt.h
	mkdir -p build/test/utils
	$(CC) -c -fpic -o $@ $< $(CFLAGS)

.PHONY: clean test coverage docs debug publish

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
	mkdir -p docs
	touch docs/index.html
	doxygen Doxyfile
	echo '<!DOCTYPE html>\n<html><head><meta http-equiv=Refresh content="0;url=html/index.html"></head></html>' > docs/index.html

debug: dist/test
	$(MAKE) && LD_LIBRARY_PATH=$(shell pwd)/dist:$(LD_LIBRARY_PATH) gdb dist/test

publish:
	env bash scripts/publish.sh
