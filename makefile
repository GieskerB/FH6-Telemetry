SDL_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL_LIBS   := $(shell pkg-config --libs sdl3)

SDL_TTF_CFLAGS := $(shell pkg-config --cflags sdl3-ttf)
SDL_TTF_LIBS   := $(shell pkg-config --libs sdl3-ttf)

CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -O3 -std=c++20 -lSDL3 $(SDL_CFLAGS) $(SDL__TTF_CFLAGS)
LDFLAGS = $(SDL_LIBS) $(SDL_TTF_LIBS)
SRCDIR := src
BUILDDIR := build


PROGS := fh6_telemetry udp_test udp_capture

fh6_telemetry_SRCS := $(SRCDIR)/main.cpp $(SRCDIR)/socket_setup.cpp $(SRCDIR)/engine_rpm.cpp $(SRCDIR)/gforce.cpp $(SRCDIR)/map.cpp
udp_test_SRCS := $(SRCDIR)/test_udp.cpp $(SRCDIR)/socket_setup.cpp
udp_capture_SRCS := $(SRCDIR)/capture_udp.cpp $(SRCDIR)/socket_setup.cpp

fh6_telemetry_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(fh6_telemetry_SRCS))
udp_test_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_test_SRCS))
udp_capture_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_capture_SRCS))

all: $(BUILDDIR) $(PROGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

fh6_telemetry: $(fh6_telemetry_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

udp_test: $(udp_test_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

udp_capture: $(udp_capture_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

# Windows target for cross compilation:

WIN_CXX := x86_64-w64-mingw32-g++
WIN_SDL := deps/SDL3-3.4.10/x86_64-w64-mingw32
WIN_TTF := deps/SDL3_ttf-3.2.2/x86_64-w64-mingw32

fh6_telemetry_WIN_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%_win.o, $(fh6_telemetry_SRCS))

SHIP_DIR    := $(BUILDDIR)/fh6_telemetry_windows
ZIP_NAME    := fh6_telemetry_windows.zip

windows: $(BUILDDIR) $(BUILDDIR)/fh6_telemetry.exe
	cp -r assets $(BUILDDIR)
	cp $(WIN_SDL)/bin/SDL3.dll $(BUILDDIR)
	cp $(WIN_TTF)/bin/SDL3_ttf.dll $(BUILDDIR)

ship-windows: windows
	mkdir -p $(SHIP_DIR)

	mv $(BUILDDIR)/fh6_telemetry.exe $(SHIP_DIR)
	mv $(BUILDDIR)/SDL3.dll $(SHIP_DIR)
	mv $(BUILDDIR)/SDL3_ttf.dll $(SHIP_DIR)

	cp -r assets/ $(SHIP_DIR)

	cd $(BUILDDIR) && zip -r $(ZIP_NAME) fh6_telemetry_windows
	rm -rf $(SHIP_DIR)

$(BUILDDIR)/fh6_telemetry.exe: $(fh6_telemetry_WIN_OBJS)
	$(WIN_CXX) -o $@ $^ \
		-L$(WIN_SDL)/lib \
		-L$(WIN_TTF)/lib \
		-lSDL3_ttf -lSDL3 \
		-static-libgcc -static-libstdc++ \
		-lws2_32

$(BUILDDIR)/%_win.o: $(SRCDIR)/%.cpp
	$(WIN_CXX) -Wall -Wextra -Werror -O3 -std=c++17 \
		-I$(WIN_SDL)/include \
		-I$(WIN_TTF)/include \
		-c $< -o $@

.PHONY: all windows ship-windows clean
