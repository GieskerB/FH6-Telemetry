SDL_CFLAGS := $(shell pkg-config --cflags sdl3)
SDL_LIBS   := $(shell pkg-config --libs sdl3)

SDL_TTF_CFLAGS := $(shell pkg-config --cflags sdl3-ttf)
SDL_TTF_LIBS   := $(shell pkg-config --libs sdl3-ttf)

CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -o3 -std=c++17 -lSDL3 $(SDL_CFLAGS) $(SDL__TTF_CFLAGS)
LDFLAGS = $(SDL_LIBS) $(SDL_TTF_LIBS)
SRCDIR := src
BUILDDIR := build


PROGS := fh6_telemetry udp_test
fh6_telemetry_SRCS := $(SRCDIR)/main.cpp $(SRCDIR)/socket_setup.cpp $(SRCDIR)/engine_rpm.cpp
udp_test_SRCS := $(SRCDIR)/test_udp.cpp $(SRCDIR)/socket_setup.cpp

fh6_telemetry_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(fh6_telemetry_SRCS))
udp_test_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_test_SRCS))

all: $(BUILDDIR) $(PROGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

fh6_telemetry: $(fh6_telemetry_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

udp_test: $(udp_test_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
