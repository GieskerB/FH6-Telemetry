CXX := g++
WIN_CXX := x86_64-w64-mingw32-g++

# Linux / Native Flags
SDL_CFLAGS     := $(shell pkg-config --cflags sdl3)
SDL_LIBS       := $(shell pkg-config --libs sdl3)
SDL_TTF_CFLAGS := $(shell pkg-config --cflags sdl3-ttf)
SDL_TTF_LIBS   := $(shell pkg-config --libs sdl3-ttf)

CXXFLAGS       := -Wall -Wextra -Werror -std=c++20 -O3 $(SDL_CFLAGS) $(SDL_TTF_CFLAGS) # -fsanitize=address -static-libasan -g 
LDFLAGS        := $(SDL_LIBS) $(SDL_TTF_LIBS)

# Windows Cross-Compilation Flags
WIN_SDL        := deps/SDL3-3.4.10/x86_64-w64-mingw32
WIN_TTF        := deps/SDL3_ttf-3.2.2/x86_64-w64-mingw32
WIN_CXXFLAGS   := -Wall -Wextra -Werror -O3 -std=c++20 -I$(WIN_SDL)/include -I$(WIN_TTF)/include
WIN_LDFLAGS    := -L$(WIN_SDL)/lib -L$(WIN_TTF)/lib -lSDL3_ttf -lSDL3 -static-libgcc -static-libstdc++ -lws2_32

# Directories
SRCDIR         := src
BUILDDIR       := build
SHIP_DIR       := $(BUILDDIR)/fh6_telemetry_windows
ZIP_NAME       := fh6_telemetry_windows.zip

PROGS          := fh6_telemetry udp_test udp_capture update_car_info

# Sources (Fixed: added $(SRCDIR)/ back to main.cpp)
fh6_telemetry_SRCS   := $(SRCDIR)/main.cpp $(SRCDIR)/udp/socket_setup.cpp $(SRCDIR)/engine_rpm.cpp $(SRCDIR)/gforce.cpp $(SRCDIR)/map.cpp $(SRCDIR)/car_info.cpp $(SRCDIR)/race_info.cpp $(SRCDIR)/util/date.cpp $(SRCDIR)/util/texture_handler.cpp $(SRCDIR)/util/csv_to_maps.cpp 
udp_test_SRCS        := $(SRCDIR)/udp/test_udp.cpp $(SRCDIR)/udp/socket_setup.cpp
udp_capture_SRCS     := $(SRCDIR)/udp/capture_udp.cpp $(SRCDIR)/udp/socket_setup.cpp
update_car_info_SRCS := $(SRCDIR)/util/update_car_info.cpp

# Object Files
fh6_telemetry_OBJS   := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(fh6_telemetry_SRCS))
udp_test_OBJS        := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_test_SRCS))
udp_capture_OBJS     := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_capture_SRCS))
update_car_info_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(update_car_info_SRCS))

fh6_telemetry_WIN_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%_win.o, $(fh6_telemetry_SRCS))
udp_capture_WIN_OBJS   := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%_win.o, $(udp_capture_SRCS))

# --- Rules ---

all: $(PROGS)

# Native Linux Executables
fh6_telemetry: $(fh6_telemetry_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

udp_test: $(udp_test_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

udp_capture: $(udp_capture_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

update_car_info: $(update_car_info_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

# Compile Native Objects (Automatically generates build subdirectories)
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Windows Target
windows: $(BUILDDIR)/fh6_telemetry.exe $(BUILDDIR)/capture_data.exe 
	cp -r assets $(BUILDDIR)
	cp $(WIN_SDL)/bin/SDL3.dll $(BUILDDIR)
	cp $(WIN_TTF)/bin/SDL3_ttf.dll $(BUILDDIR)

$(BUILDDIR)/fh6_telemetry.exe: $(fh6_telemetry_WIN_OBJS)
	$(WIN_CXX) -o $@ $^ $(WIN_LDFLAGS)

$(BUILDDIR)/capture_data.exe: $(udp_capture_WIN_OBJS)
	$(WIN_CXX) -o $@ $^ -static-libgcc -static-libstdc++ -lws2_32

# Compile Windows Objects (Automatically generates build subdirectories)
$(BUILDDIR)/%_win.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(WIN_CXX) $(WIN_CXXFLAGS) -c $< -o $@

# Packaging
ship-windows: windows
	mkdir -p $(SHIP_DIR)
	cp $(BUILDDIR)/fh6_telemetry.exe $(SHIP_DIR)
	cp $(BUILDDIR)/capture_data.exe $(SHIP_DIR)
	cp $(BUILDDIR)/SDL3.dll $(SHIP_DIR)
	cp $(BUILDDIR)/SDL3_ttf.dll $(SHIP_DIR)
	cp -r assets/ $(SHIP_DIR)

zip-windows: ship-windows
	cd $(BUILDDIR) && zip -r $(ZIP_NAME) fh6_telemetry_windows

clean:
	rm -rf $(BUILDDIR)

.PHONY: all windows ship-windows zip-windows clean