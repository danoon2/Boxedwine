# Link the production native runtime into an isolated SDL input harness.
# Only host window/cursor APIs are replaced; no display or game is required.
.PHONY: native-input-test
native-input-test: INCLUDES := -I../../include -I../../platform/sdl -I../../lib/asmjit -I../../lib/glew/include -I../../lib/imgui -I../../lib/simde -I../../lib/pugixml/src -I../../lib/zlib -I../../lib/zlib/contrib/minizip
native-input-test: $(ALL)
	@$(or $(OBJCOPY),objcopy) --redefine-sym main=boxedwineOriginalMain $(BUILD_DIR)/$(PROJDIR)/platform/sdl/knativesystem.cpp.o $(BUILD_DIR)/mouse-test-system.o
	@$(CXX) $(CPPFLAGS) -UNDEBUG ../../tools/input/test_mouse_capture.cpp $(filter-out %/knativesystem.cpp.o,$(ALL)) $(BUILD_DIR)/mouse-test-system.o -o $(BUILD_DIR)/test-mouse-capture $(LDFLAGS)
	@$(BUILD_DIR)/test-mouse-capture
