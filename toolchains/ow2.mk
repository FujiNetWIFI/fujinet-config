CC_DEFAULT ?= wcc
AS_DEFAULT ?= wasm
LD_DEFAULT ?= wlink OPTION quiet

include $(MWD)/tc-common.mk
CFLAGS += -0 -bt=dos -ms -s -osh -zu
AFLAGS +=
LDFLAGS += SYSTEM dos OPTION MAP LIBPATH $(FUJINET_LIB_DIR)

define include-dir-flag
  -I$1
endef

define asm-include-dir-flag
  -I$1
endef

define library-dir-flag
endef

define library-flag
  $1
endef

define link-lib
  $(LIB) -n $1 $2
endef

define link-bin
  $(LD) $(LDFLAGS) \
    disable 1014 \
    name $1 \
    file {$2} \
    library {$(LIBS)}
endef

define compile
  $(CC) $(CFLAGS) -ad=$(OBJ_DIR)/$(basename $(notdir $2)).d -fo=$1 $2
endef

define assemble
  $(AS) -c $(AFLAGS) -o $1 $2 2>&1
endef
