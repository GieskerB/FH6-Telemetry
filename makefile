CXX := g++
CXXFLAGS := -Wall -Wextra -Werror -o3 -std=c++17
SRCDIR := src
BUILDDIR := build

PROGS := fh6_telemetry, udp_test

PROGS := fh6_telemetry udp_test
fh6_telemetry_SRCS := $(SRCDIR)/main.cpp
udp_test_SRCS := $(SRCDIR)/test_udp.cpp

fh6_telemetry_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(fh6_telemetry_SRCS))
udp_test_OBJS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(udp_test_SRCS))

all: $(BUILDDIR) $(PROGS)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

fh6_telemetry: $(fh6_telemetry_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^

udp_test: $(udp_test_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
