CC      ?= gcc
CFLAGS  ?= -std=gnu11 -O2 -g -Wall -Wextra -Wno-unused-parameter
SAN     := -fsanitize=address,undefined -fno-omit-frame-pointer

SRC := src/dnf.c src/big.c src/net.c src/lang.c

all: build/cubical

build/cubical: $(SRC) src/core.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC) -lpthread

build/cubical-asan: $(SRC) src/core.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(SAN) -o $@ $(SRC) -lpthread

# tests check: the module cache is off, except in the cache test (a fresh directory, run twice)
test: export CUBICAL_NOCACHE = 1
test: build/cubical build/cubical-asan
	./build/cubical lib/prelude.ctt tests/cubical.ctt
	./build/cubical lib/prelude.ctt lib/code.ctt tests/code.ctt
	./build/cubical lib/prelude.ctt tests/hit.ctt
	./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt tests/check.ctt
	./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt tests/reject.ctt
	rm -rf build/cache-test
	env -u CUBICAL_NOCACHE CUBICAL_CACHE=build/cache-test ./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt tests/mod/main.ctt | tail -1
	env -u CUBICAL_NOCACHE CUBICAL_CACHE=build/cache-test ./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt tests/mod/main.ctt | grep -c "^cached: " | grep -qx 6
	# the step counts (every reading's, and the totals) and the answers do not depend on the number of workers
	set -e; for s in "tests/cubical.ctt" "lib/code.ctt tests/code.ctt" "tests/hit.ctt" \
	    "lib/code.ctt lib/check.ctt tests/check.ctt" "lib/code.ctt lib/check.ctt lib/std.ctt tests/reject.ctt"; do \
	  CUBICAL_WORKERS=0 ./build/cubical lib/prelude.ctt $$s > build/workers-0.out; \
	  for w in 1 2 4 8 16; do CUBICAL_WORKERS=$$w ./build/cubical lib/prelude.ctt $$s > build/workers-$$w.out; \
	    cmp -s build/workers-0.out build/workers-$$w.out || { echo "differs with $$w workers: $$s"; diff build/workers-0.out build/workers-$$w.out | head; exit 1; }; done; \
	done; echo "every suite: identical output with 0, 1, 2, 4, 8, 16 workers"
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.ctt tests/cubical.ctt > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.ctt lib/code.ctt tests/code.ctt > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.ctt tests/hit.ctt > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.ctt lib/code.ctt lib/check.ctt tests/check.ctt > /dev/null

# the checked library and the acceptance tests (the paper's terms, 9.15's sorting): long, run once
library: export CUBICAL_NOCACHE = 1
library: build/cubical
	./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt tests/boxes.ctt
	./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt tests/sort.ctt

# programs written on top of the machine (playground/): not part of the runtime or its tests
.PHONY: playground
playground: export CUBICAL_NOCACHE = 1
playground: build/cubical
	./build/cubical lib/prelude.ctt lib/code.ctt lib/check.ctt lib/std.ctt playground/sort-ordered.ctt

build/cubical-tsan: $(SRC) src/core.h
	@mkdir -p build
	$(CC) -std=gnu11 -O1 -g -fsanitize=thread -o $@ $(SRC) -lpthread

tsan: build/cubical-tsan
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.ctt tests/cubical.ctt > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.ctt lib/code.ctt tests/code.ctt > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.ctt tests/hit.ctt > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.ctt lib/code.ctt lib/check.ctt tests/check.ctt > /dev/null

clean:
	rm -rf build
