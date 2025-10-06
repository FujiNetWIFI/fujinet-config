CC_DEFAULT ?= cl65 --color off
AS_DEFAULT ?= $(CC_DEFAULT)
LD_DEFAULT ?= $(CC_DEFAULT)
AR_DEFAULT ?= ar65

include $(MWD)/tc-common.mk

CFLAGS += -O --cpu 6502
AFLAGS += --cpu 6502
LDFLAGS +=

define include-dir-flag
  --include-dir $1
endef

define asm-include-dir-flag
  --asm-include-dir $1
endef

define library-dir-flag
  --lib-path $1
endef

define library-flag
  $1
endef

define link-lib
  $(AR) a $1 $2
endef

define link-bin
  $(LD) -vm -t $(PLATFORM) $(LDFLAGS) $2 $(LIBS) -o $1
endef

define compile
  $(CC) -l $(basename $1).lst --create-dep $(OBJ_DIR)/$(basename $(notdir $2)).d -c $(CFLAGS) -t $(PLATFORM) -o $1 $2
endef

define assemble
  $(AS) -l $(basename $1).lst -c $(AFLAGS) -t $(PLATFORM) -o $1 $2
endef
