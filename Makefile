# Thin wrapper around CMake so common tasks are a single short command.
# The real build graph lives in CMakeLists.txt / core/ / platform/.

BUILD_DIR := build
BUILD_TYPE ?= Debug
SPRITE_PNG := $(BUILD_DIR)/sprite_sheet.png

.PHONY: all build run test tsp title trackplayer clean

all: build

$(BUILD_DIR)/CMakeCache.txt:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR) -j

# run the actual game window
run: build
	./$(BUILD_DIR)/drift_sdl2

# title screen only, looping forever (never hands off to the game stub)
title: build
	./$(BUILD_DIR)/title_demo

# plays one sfx track in a loop; LEFT/RIGHT switches tracks, e.g.
#   make trackplayer TRACK=1
trackplayer: build
	./$(BUILD_DIR)/track_player $(TRACK)

# headless regression test for core/*.hpp (no window, no SDL needed)
test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# sprite/palette smoke test: renders every chara sprite def to a PNG so
# pixel-art/palette conversion can be checked without a live window
tsp: build
	./$(BUILD_DIR)/sprite_viewer $(SPRITE_PNG)
	@echo "wrote $(SPRITE_PNG)"

clean:
	rm -rf $(BUILD_DIR)
