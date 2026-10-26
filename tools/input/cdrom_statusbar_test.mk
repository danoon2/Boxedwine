.PHONY: cdrom-statusbar-test
cdrom-statusbar-test: INCLUDES := -I../../include -I../../platform/sdl -I../../lib/asmjit -I../../lib/glew/include -I../../lib/imgui -I../../lib/simde -I../../lib/pugixml/src -I../../lib/zlib -I../../lib/zlib/contrib/minizip
cdrom-statusbar-test: $(ALL)
	@$(or $(OBJCOPY),objcopy) --redefine-sym main=boxedwineOriginalMain $(BUILD_DIR)/$(PROJDIR)/platform/sdl/knativesystem.cpp.o $(BUILD_DIR)/statusbar-test-system.o
	@$(CXX) $(CPPFLAGS) -UNDEBUG ../../tools/input/test_cdrom_statusbar.cpp ../../tools/input/test_cdrom_statusbar_wayland.cpp $(filter-out %/knativesystem.cpp.o,$(ALL)) $(BUILD_DIR)/statusbar-test-system.o -o $(BUILD_DIR)/test-cdrom-statusbar $(LDFLAGS)
	@$(BUILD_DIR)/test-cdrom-statusbar $(STATUSBAR_TEST_ARGS)
