# Checks the Adam new-disk EOS directory against eosfs. Run: make -C tests/adam
# EOSFS=<path> uses an installed eosfs instead of building the pinned one.

HERE    := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC     := $(HERE)../../src/adam/system.c
OUT     := $(HERE)../../build/tests/adam
EOSFS_COMMIT := c11801d
EOSFS   ?= $(OUT)/eosfs/eosfs
CC      ?= cc

# size:directory blocks, matching system_build_directory's thresholds
CASES   := 128:1 160:1 161:2 256:2 319:2 320:3 719:3 720:6 1440:6
LABELS  := - TEST ABCDEFGHIJK

.PHONY: test
test: $(OUT)/eosdir_test $(EOSFS)
	@fail=0; \
	for c in $(CASES); do \
	  n=$${c%%:*}; db=$${c##*:}; \
	  for l in $(LABELS); do \
	    v=$$l; vopt="-v $$l"; [ "$$l" = - ] && { v=; vopt=; }; \
	    $(OUT)/eosdir_test 1 $$n "$$v" $(OUT)/got.img || { echo "FAIL: $$n blocks '$$v': harness"; fail=1; continue; }; \
	    $(EOSFS) create $(OUT)/want.img ddp --blocks $$n -d $$db $$vopt >/dev/null || { echo "FAIL: eosfs create"; fail=1; continue; }; \
	    cmp -s $(OUT)/got.img $(OUT)/want.img || { echo "FAIL: $$n blocks '$$v': differs from eosfs"; fail=1; continue; }; \
	    $(EOSFS) list $(OUT)/got.img --type ddp >/dev/null || { echo "FAIL: $$n blocks '$$v': eosfs list"; fail=1; continue; }; \
	    echo "ok: $$n blocks, $$db directory blocks, label '$$v'"; \
	  done; \
	done; \
	exit $$fail

# Copy system.c beside test-owned headers: its quoted includes resolve next to it
$(OUT)/eosdir_test: $(HERE)eosdir_test.c $(SRC) $(wildcard $(HERE)stub/*.h)
	@mkdir -p $(OUT)/src/adam
	cp $(HERE)stub/system.h $(HERE)stub/globals.h $(OUT)/src/
	cp $(SRC) $(OUT)/src/adam/system.c
	$(CC) -std=gnu99 -Wall -Werror -Wno-unused-function -I$(HERE)stub -o $@ $(HERE)eosdir_test.c $(OUT)/src/adam/system.c

$(OUT)/eosfs/eosfs:
	@mkdir -p $(OUT)
	git clone -q https://github.com/tschak909/eosfs $(OUT)/eosfs
	git -C $(OUT)/eosfs checkout -q $(EOSFS_COMMIT)
	$(MAKE) -C $(OUT)/eosfs eosfs
