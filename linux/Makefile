CXX ?= g++
SDL_CONFIG ?= sdl2-config
SDL_CFLAGS ?= $(shell $(SDL_CONFIG) --cflags)
SDL_LIBS ?= $(shell $(SDL_CONFIG) --libs)
BUILD_DIR := build
TARGET ?= $(BUILD_DIR)/yokoi
SMOKE_TARGET := $(BUILD_DIR)/smoke-test
ifneq ($(strip $(SYSROOT)),)
CPPFLAGS += --sysroot="$(SYSROOT)"
LDFLAGS += --sysroot="$(SYSROOT)"
endif
CPPFLAGS += -I. -I../source -I../android/app/src/main/cpp/core -I../android/app/src/main/cpp/input $(SDL_CFLAGS) -DYOKOI_PORTMASTER=1 -DYOKOI_ROMPACK_REQUIRED_CONTENT_VERSION=3
CXXFLAGS ?= -O2 -std=c++20 -Wall -Wextra
LDLIBS += $(SDL_LIBS) -pthread
CORE := $(wildcard ../source/SM5XX/*.cpp ../source/SM5XX/*/*.cpp ../source/std/*.cpp)
ANDROID := ../android/app/src/main/cpp/core/yokoi_cpu_utils.cpp ../android/app/src/main/cpp/input/yokoi_input_mapping.cpp
FRONTEND := $(wildcard core/*.cpp input/*.cpp render/*.cpp)
FONT_DATA := render/yokoi_font_data.S
FONT_ASSETS := third_party/Audiowide-Regular.ttf third_party/Audiowide-OFL.txt
SMOKE_WRAPS := SDL_OpenAudioDevice SDL_NewAudioStream SDL_GetNumVideoDisplays SDL_CreateWindow SDL_GetWindowDisplayIndex SDL_CreateRenderer SDL_RenderSetLogicalSize SDL_RenderPresent SDL_GetKeyboardState SDL_iconv_string time
.PHONY: all clean check check-cross-build check-launcher smoke-test
all: $(TARGET)
HEADERS := $(wildcard core/*.h input/*.h render/*.h ../source/SM5XX/*.h ../source/SM5XX/*/*.h ../source/std/*.h ../source/virtual_i_o/virtual_input.h ../android/app/src/main/cpp/core/yokoi_cpu_utils.h ../android/app/src/main/cpp/input/yokoi_input_mapping.h third_party/*.h)
$(TARGET): main.cpp $(FRONTEND) $(CORE) $(ANDROID) $(HEADERS) $(FONT_DATA) $(FONT_ASSETS)
	mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) main.cpp $(FRONTEND) $(CORE) $(ANDROID) $(FONT_DATA) $(LDFLAGS) $(LDLIBS) -o "$@"
clean:
	rm -f "$(TARGET)" "$(SMOKE_TARGET)" "$(BUILD_DIR)/yokoi.aarch64" yokoi smoke-test
smoke-test: $(SMOKE_TARGET)
$(SMOKE_TARGET): tests/smoke.cpp $(FRONTEND) $(CORE) $(ANDROID) $(HEADERS) $(FONT_DATA) $(FONT_ASSETS)
	mkdir -p "$(@D)"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) tests/smoke.cpp $(FRONTEND) $(CORE) $(ANDROID) $(FONT_DATA) $(LDFLAGS) $(foreach symbol,$(SMOKE_WRAPS),-Wl,--wrap=$(symbol)) $(LDLIBS) -o "$@"

check-cross-build:
	bash tests/cross-build.sh
	bash tests/toolchain.sh

check-launcher:
	bash tests/launcher.sh

check: all $(SMOKE_TARGET) check-cross-build check-launcher
	YOKOI_BINARY="$(abspath $(TARGET))" YOKOI_SMOKE_BINARY="$(abspath $(SMOKE_TARGET))" python3 tests/run_smoke.py
