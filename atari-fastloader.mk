define cache_repo
  url=$1; \
  dir=$$(basename $$url .git); \
  mkdir -p $(CACHE_DIR); \
  if [ -d "$(CACHE_DIR)/$$dir/.git" ]; then \
    echo "Updating $$dir"; \
    git -C "$(CACHE_DIR)/$$dir" fetch --all --tags; \
  else \
    echo "Cloning $$url"; \
    git -C "$(CACHE_DIR)" clone $$url; \
  fi
endef

CONFIG_TOOLS_REPO = https://github.com/FujiNetWIFI/fujinet-tools.git
CONFIG_LOADER_REPO = https://github.com/FujiNetWIFI/fujinet-config-loader.git

CACHE_TOOLS = $(CACHE_DIR)/$(basename $(notdir $(CONFIG_TOOLS_REPO)))
CACHE_LOADER = $(CACHE_DIR)/$(basename $(notdir $(CONFIG_LOADER_REPO)))

$(CACHE_TOOLS): | $(CACHE_DIR)
	$(call cache_repo,$(CONFIG_TOOLS_REPO))

$(CACHE_LOADER): | $(CACHE_DIR)
	$(call cache_repo,$(CONFIG_LOADER_REPO))

atari/disk-post:: $(CACHE_TOOLS) $(CACHE_LOADER)
	make -C $(CACHE_TOOLS)/pre_migration/atari
	make -C $(CACHE_LOADER) CONFIG_PROG=$$(realpath --relative-to=$(CACHE_LOADER) $(BUILD_EXEC))
	$(RM) $(BUILD_DISK)
	$(RM) -rf $(CACHE_PLATFORM)/disk
	$(MKDIR_P) $(CACHE_PLATFORM)/disk
	cp $(CACHE_LOADER)/src/cloader.zx0 $(CACHE_PLATFORM)/disk
	cp $(CACHE_LOADER)/src/$(notdir $(BUILD_EXEC)) $(CACHE_PLATFORM)/disk
	cp $(CACHE_TOOLS)/pre_migration/atari/dist/*.com dist/ 2>/dev/null || true
	cp $(CACHE_TOOLS)/pre_migration/atari/dist/*.COM dist/ 2>/dev/null || true
	$(DISK_TOOL) -S -B $(CACHE_LOADER)/src/zx0boot.bin $(BUILD_DISK) $(CACHE_PLATFORM)/disk
	$(CACHE_LOADER)/tools/update-atr.py $(BUILD_DISK) cloader.zx0 config.com
