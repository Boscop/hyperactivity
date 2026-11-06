CC      ?= gcc
CFLAGS  ?= -std=gnu11 -O2 -g -Wall -Wextra -Wno-unused-parameter
SAN     := -fsanitize=address,undefined -fno-omit-frame-pointer

SRC := src/dnf.c src/big.c src/net.c src/lang.c

all: build/cubical

build/cubical: $(SRC) src/core.h src/syntax.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC) -lpthread

build/cubical-asan: $(SRC) src/core.h src/syntax.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(SAN) -o $@ $(SRC) -lpthread

# tests check: the module cache is off, except in the cache test (a fresh directory, run twice)
test: export CUBICAL_NOCACHE = 1
test: build/cubical build/cubical-asan
	./build/cubical lib/prelude.cub tests/cubical.cub
	./build/cubical lib/prelude.cub lib/code.cub tests/code.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/fo.cub lib/emitc.cub
	python3 experiments/fo/run.py
	./build/cubical lib/prelude.cub lib/code.cub lib/match.cub lib/nav.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/match.cub lib/lin.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/fo.cub lib/match.cub lib/check.cub lib/specialize.cub
	./build/cubical lib/prelude.cub tests/hit.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub tests/check.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/reject.cub tests/quote.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/codata.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub lib/linproof.cub lib/eqproof.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/recursion.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/guarded.cub
	# Brunerie's number (Ljungström's brunerie', an untyped port): -2, with one and with four workers
	for w in 0 4; do CUBICAL_WORKERS=$$w ./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub \
	  experiments/brunerie/brunerie.cub experiments/brunerie/run-brunerie.cub | grep -A1 -x brunerie | grep -q "= negsuc (suc zero) " \
	  || { echo "brunerie: not -2 with $$w workers"; exit 1; }; done; echo "brunerie = negsuc (suc zero)"
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub experiments/brunerie/brunerie.cub \
	  experiments/brunerie/run-squash.cub | tail -1 | grep -q " 0 failed"
	# the typed port: every definition checked (the recursors and surfc are primitives), and -2
	for w in 0 4; do CUBICAL_WORKERS=$$w ./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub \
	  experiments/brunerie/typed.cub experiments/brunerie/run-typed.cub > build/typed-$$w.out; \
	  tail -1 build/typed-$$w.out | grep -q " 0 failed" || { echo "typed brunerie: a check failed with $$w workers"; grep -A3 ^FAIL build/typed-$$w.out | head; exit 1; }; \
	  grep -A1 -x brunerie build/typed-$$w.out | grep -q "= negsuc (suc zero) " || { echo "typed brunerie: not -2 with $$w workers"; exit 1; }; done; echo "typed brunerie = negsuc (suc zero)"
	rm -rf build/cache-test
	env -u CUBICAL_NOCACHE CUBICAL_CACHE=build/cache-test ./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/mod/main.cub | tail -1
	env -u CUBICAL_NOCACHE CUBICAL_CACHE=build/cache-test ./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/mod/main.cub | grep -c "^cached: " | grep -qx 6
	# without the memo, the step counts (every reading's, and the totals) and the answers do not depend
	# on the number of workers; with it, the answers do not (what a reading finds already computed
	# depends on the order of the rewrites, so its count can)
	set -e; for s in "tests/cubical.cub" "lib/code.cub tests/code.cub" "tests/hit.cub" "tests/memo.cub" \
	    "lib/code.cub lib/check.cub tests/check.cub" "lib/code.cub lib/check.cub lib/std.cub tests/reject.cub tests/quote.cub" \
	    "lib/code.cub lib/check.cub lib/std.cub tests/codata.cub" "lib/code.cub lib/check.cub lib/std.cub tests/guarded.cub"; do \
	  CUBICAL_NOMEMO=1 CUBICAL_WORKERS=0 ./build/cubical lib/prelude.cub $$s > build/workers-0.out; \
	  for w in 1 2 4 8 16; do CUBICAL_NOMEMO=1 CUBICAL_WORKERS=$$w ./build/cubical lib/prelude.cub $$s > build/workers-$$w.out; \
	    cmp -s build/workers-0.out build/workers-$$w.out || { echo "differs with $$w workers: $$s"; diff build/workers-0.out build/workers-$$w.out | head; exit 1; }; done; \
	  CUBICAL_WORKERS=0 ./build/cubical lib/prelude.cub $$s | sed 's/ *\[[0-9]* steps[^]]*\]//; /^[0-9]* passed/d' > build/workers-0.out; \
	  for w in 1 2 4 8 16; do CUBICAL_WORKERS=$$w ./build/cubical lib/prelude.cub $$s | sed 's/ *\[[0-9]* steps[^]]*\]//; /^[0-9]* passed/d' > build/workers-$$w.out; \
	    cmp -s build/workers-0.out build/workers-$$w.out || { echo "answers differ with $$w workers: $$s"; diff build/workers-0.out build/workers-$$w.out | head; exit 1; }; done; \
	done; echo "every suite: identical output with 0, 1, 2, 4, 8, 16 workers without the memo; identical answers with it"
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.cub tests/cubical.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.cub lib/code.cub tests/code.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.cub tests/hit.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.cub lib/code.cub lib/check.cub tests/check.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 ./build/cubical-asan lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/quote.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 CUBICAL_WORKERS=4 ./build/cubical-asan lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/codata.cub > /dev/null
	ASAN_OPTIONS=detect_leaks=0 CUBICAL_WORKERS=4 ./build/cubical-asan lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/guarded.cub > /dev/null

# the checked library and the acceptance tests (the paper's terms, 9.15's sorting): long, run once
library: export CUBICAL_NOCACHE = 1
library: build/cubical
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/boxes.cub
	./build/cubical lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/sort.cub
	./build/cubical lib/prelude.cub tests/memo.cub

build/cubical-tsan: $(SRC) src/core.h src/syntax.c
	@mkdir -p build
	$(CC) -std=gnu11 -O1 -g -fsanitize=thread -o $@ $(SRC) -lpthread

tsan: build/cubical-tsan
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.cub tests/cubical.cub > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.cub lib/code.cub tests/code.cub > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.cub tests/hit.cub > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.cub lib/code.cub lib/check.cub tests/check.cub > /dev/null
	CUBICAL_WORKERS=4 TSAN_OPTIONS=halt_on_error=1 ./build/cubical-tsan lib/prelude.cub lib/code.cub lib/check.cub lib/std.cub tests/quote.cub > /dev/null

clean:
	rm -rf build
