CXX := g++
WIN_CXX := x86_64-w64-mingw32-g++

# Linux / Native Flags
SDL_CFLAGS := $(shell pkg-config --cflags sdl3 sdl3-ttf)
SDL_LIBS   := $(shell pkg-config --libs sdl3 sdl3-ttf)

CXXFLAGS := -Wall -Wextra -Werror -std=c++20 -O3 $(SDL_CFLAGS) # -fsanitize=address -static-libasan -g
LDFLAGS  := $(SDL_LIBS)

# Windows Cross-Compilation Flags
WIN_SDL      := deps/SDL3-3.4.10/x86_64-w64-mingw32
WIN_TTF      := deps/SDL3_ttf-3.2.2/x86_64-w64-mingw32
WIN_CXXFLAGS := -Wall -Wextra -Werror -O3 -std=c++20 -I$(WIN_SDL)/include -I$(WIN_TTF)/include
WIN_LDFLAGS  := -L$(WIN_SDL)/lib -L$(WIN_TTF)/lib -lSDL3_ttf -lSDL3 -static-libgcc -static-libstdc++ -lws2_32

# Directories
SRC_DIR   := src
BUILD_DIR := build
WIN_DIR   := $(BUILD_DIR)/windows
ZIP_NAME  := FH6Telemetry.zip

PROGS := telemetry udp_test udp_capture update_cars
WIN_PROGS := telemetry.exe capture_data.exe update_cars.exe

# Sources
telemetry_SRCS   := $(SRC_DIR)/main.cpp \
 					$(SRC_DIR)/udp/socket_setup.cpp \
					$(SRC_DIR)/util/date.cpp $(SRC_DIR)/util/texture_handler.cpp $(SRC_DIR)/util/csv_to_maps.cpp  \
					$(SRC_DIR)/car_info.cpp $(SRC_DIR)/engine_rpm.cpp $(SRC_DIR)/gforce.cpp $(SRC_DIR)/map.cpp $(SRC_DIR)/race_info.cpp
udp_test_SRCS    := $(SRC_DIR)/udp/test_udp.cpp $(SRC_DIR)/udp/socket_setup.cpp
udp_capture_SRCS := $(SRC_DIR)/udp/capture_udp.cpp $(SRC_DIR)/udp/socket_setup.cpp
update_cars_SRCS := $(SRC_DIR)/util/update_car_info.cpp

# Object Files
telemetry_OBJS   := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(telemetry_SRCS))
udp_test_OBJS    := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(udp_test_SRCS))
udp_capture_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(udp_capture_SRCS))
update_cars_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(update_cars_SRCS))

# Windows Object Files
telemetry_WIN_OBJS   := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%_win.o, $(telemetry_SRCS))
udp_capture_WIN_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%_win.o, $(udp_capture_SRCS))
update_cars_WIN_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%_win.o, $(update_cars_SRCS))

# --- Rules ---

all: $(PROGS) windows

# Native Linux Executables
telemetry: $(telemetry_OBJS)
	@echo "Linking" $@
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

udp_test: $(udp_test_OBJS)
	@echo "Linking" $@
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

udp_capture: $(udp_capture_OBJS)
	@echo "Linking" $@
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

update_cars: $(update_cars_OBJS)
	@echo "Linking" $@
	@$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

# Compile Native Objects (Automatically generates build subdirectories)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling" $@
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Windows Target
windows: $(WIN_PROGS)
	@mkdir $(WIN_DIR)
	@cp -r csv/ $(WIN_DIR)
	@cp -r assets/ $(WIN_DIR)
	@cp $(WIN_SDL)/bin/SDL3.dll $(WIN_DIR)
	@cp $(WIN_TTF)/bin/SDL3_ttf.dll $(WIN_DIR)
	@cp $(BUILD_DIR)/telemetry.exe $(WIN_DIR)/telemetry.exe
	@cp $(BUILD_DIR)/capture_data.exe $(WIN_DIR)/capture_data.exe
	@cp $(BUILD_DIR)/update_cars.exe $(WIN_DIR)/update_cars.exe
	@echo "Creating run script..."
	@echo "@echo off" > $(WIN_DIR)/run.bat
	@echo "update_cars.exe" >> $(WIN_DIR)/run.bat
	@echo "telemetry.exe" >> $(WIN_DIR)/run.bat

telemetry.exe: $(telemetry_WIN_OBJS)
	@echo "Linking" $@
	@$(WIN_CXX) -o $(BUILD_DIR)/$@ $^ $(WIN_LDFLAGS)

capture_data.exe: $(udp_capture_WIN_OBJS)
	@echo "Linking" $@
	@$(WIN_CXX) -o $(BUILD_DIR)/$@ $^ $(WIN_LDFLAGS)

update_cars.exe: $(update_cars_WIN_OBJS)
	@echo "Linking" $@
	@$(WIN_CXX) -o $(BUILD_DIR)/$@ $^ $(WIN_LDFLAGS)

# Compile Windows Objects (Automatically generates build subdirectories)
$(BUILD_DIR)/%_win.o: $(SRC_DIR)/%.cpp
	@echo "Compiling" $@
	@mkdir -p $(dir $@)
	@$(WIN_CXX) $(WIN_CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

.PHONY: all telemetry udp_test udp_capture update_cars windows clean
